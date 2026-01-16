---
title: How to Handle Backpressure
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Handle Backpressure

This guide shows you how to manage flow control when downstream processes can't keep up with upstream data production.

## Goal

By the end of this guide, you'll be able to:
- Understand when backpressure occurs
- Implement buffering to handle backpressure
- Design processes that handle slow consumers
- Prevent data loss from overflow

## Prerequisites

- You understand [channels](../tutorials/channels)
- You know how [processes work](../reference/processes)
- You've read about [channel communication](../explanations/channel-communication)

## Step 1: Understand Backpressure

Backpressure occurs when a producer sends data faster than the consumer can process it. In Weaver, channels block by default, providing automatic backpressure:

```weaver
func fast_producer() chan<int> out {
    while {
        out.send(data)  // Blocks if consumer not ready
    }
}

func slow_consumer(chan<int> in) {
    while {
        await in {
            process_slowly(in.recv())  // Producer waits here
        }
    }
}
```

The producer automatically slows down to match the consumer's rate.

## Step 2: Add Buffering for Smoothing

If you need to smooth out rate differences, add a buffer stage:

```weaver
func buffer(chan<int> in) chan<int> out {
    var int<8> buffer[16]  // 16-element buffer
    var int<4> head = 0, tail = 0, count = 0
    
    while {
        // Try to receive into buffer
        if count < 16 {
            await in {
                buffer[head] = in.recv()
                head = (head + 1) % 16
                count = count + 1
            } and if count > 0 {
                // Try to send from buffer
                out.send(buffer[tail])
                tail = (tail + 1) % 16
                count = count - 1
            }
        } or if count == 16 {
            // Buffer full, only send
            if count > 0 {
                out.send(buffer[tail])
                tail = (tail + 1) % 16
                count = count - 1
            }
        } or if count == 0 {
            // Buffer empty, only receive
            await in {
                buffer[head] = in.recv()
                head = (head + 1) % 16
                count = count + 1
            }
        }
    }
}
```

## Step 3: Implement a FIFO Buffer

A FIFO (First-In-First-Out) buffer is a common solution for backpressure:

```weaver
func fifo_buffer(chan<int<32>> in, int<8> depth) chan<int<32>> out {
    var int<32> buffer[depth]
    var int<8> write_ptr = 0, read_ptr = 0
    var int<8> count = 0
    
    while {
        // Write side: add to buffer if not full
        if count < depth {
            await in {
                buffer[write_ptr] = in.recv()
                write_ptr = (write_ptr + 1) % depth
                count = count + 1
            } and if count > 0 {
                // Read side: remove from buffer if not empty
                out.send(buffer[read_ptr])
                read_ptr = (read_ptr + 1) % depth
                count = count - 1
            }
        } or if count == depth {
            // Buffer full: only read
            if count > 0 {
                out.send(buffer[read_ptr])
                read_ptr = (read_ptr + 1) % depth
                count = count - 1
            }
        } or if count == 0 {
            // Buffer empty: only write
            await in {
                buffer[write_ptr] = in.recv()
                write_ptr = (write_ptr + 1) % depth
                count = count + 1
            }
        }
    }
}
```

## Step 4: Use Ready/Valid Handshaking

Implement explicit ready/valid handshaking for more control:

```weaver
type Handshake {
    int<32> data
    wire valid
    wire ready
}

func producer_with_handshake() chan<Handshake> out {
    var Handshake h
    while {
        h.data = generate_data()
        h.valid = vdd
        out.send(h)
        await h.ready {
            h.valid = gnd
        }
    }
}

func consumer_with_handshake(chan<Handshake> in) {
    var Handshake h
    while {
        await in {
            h = in.recv()
            if h.valid == vdd {
                process(h.data)
                h.ready = vdd
                // Signal ready
            }
        }
    }
}
```

## Step 5: Implement Backpressure Signals

Add explicit backpressure signals to control flow:

```weaver
func producer_with_backpressure(chan<wire> backpressure) chan<int> out {
    var int data
    while {
        await ~backpressure {  // Wait for no backpressure
            data = generate_data()
            out.send(data)
        }
    }
}

func consumer_with_backpressure(chan<int> in) chan<wire> backpressure {
    var int data
    while {
        if can_accept() {
            backpressure.send(gnd)  // No backpressure
            await in {
                data = in.recv()
                process(data)
            }
        } else {
            backpressure.send(vdd)  // Signal backpressure
        }
    }
}
```

## Step 6: Handle Multiple Producers

When multiple producers feed one consumer, coordinate backpressure:

```weaver
func multi_producer_consumer(
    chan<int> in1, chan<int> in2, chan<int> in3
) chan<wire> backpressure {
    var int data
    var wire bp1, bp2, bp3
    
    while {
        // Check if we can accept data
        if can_accept() {
            // Accept from any ready producer
            await in1 {
                data = in1.recv()
                process(data)
            } or await in2 {
                data = in2.recv()
                process(data)
            } or await in3 {
                data = in3.recv()
                process(data)
            }
            // Signal no backpressure
            backpressure.send(gnd)
        } else {
            // Signal backpressure to all
            backpressure.send(vdd)
        }
    }
}
```

## Step 7: Implement Rate Limiting

Limit the rate at which data is produced to match consumer capacity:

```weaver
func rate_limited_producer(chan<wire> rate_limit) chan<int> out {
    var int data
    while {
        await rate_limit {  // Wait for rate limit signal
            rate_limit.recv()
            data = generate_data()
            out.send(data)
        }
    }
}

func rate_limiter(chan<int> in) chan<int> out {
    var int data
    var wire limit_signal
    while {
        // Generate rate limit signal (e.g., every N cycles)
        limit_signal = vdd
        await in {
            data = in.recv()
            out.send(data)
        }
        limit_signal = gnd
    }
}
```

## Step 8: Complete Example: Buffered Pipeline

Here's a complete example with buffering between pipeline stages:

```weaver
func buffered_pipeline() {
    var chan<int<32>> raw_data, buffered_data, processed_data
    
    // Stage 1: Fast producer
    func producer() chan<int<32>> out {
        var int<32> count = 0
        while {
            out.send(count)
            count = count + 1
        }
    }
    
    // Buffer: Smooths rate differences
    func buffer_stage(chan<int<32>> in) chan<int<32>> out {
        var int<32> buf[8]
        var int<3> head = 0, tail = 0, count = 0
        
        while {
            if count < 8 {
                await in {
                    buf[head] = in.recv()
                    head = (head + 1) % 8
                    count = count + 1
                } and if count > 0 {
                    out.send(buf[tail])
                    tail = (tail + 1) % 8
                    count = count - 1
                }
            } or if count == 8 {
                // Full: only send
                out.send(buf[tail])
                tail = (tail + 1) % 8
                count = count - 1
            } or if count == 0 {
                // Empty: only receive
                await in {
                    buf[head] = in.recv()
                    head = (head + 1) % 8
                    count = count + 1
                }
            }
        }
    }
    
    // Stage 2: Slow consumer
    func consumer(chan<int<32>> in) {
        var int<32> value
        while {
            await in {
                value = in.recv()
                process_slowly(value)  // Takes time
            }
        }
    }
    
    // Connect with buffer
    producer() -> raw_data
    buffer_stage(raw_data) -> buffered_data
    consumer(buffered_data)
}
```

## Best Practices

### Size Buffers Appropriately

Buffer size should match expected rate differences:
- Too small: Frequent backpressure
- Too large: Wastes resources

### Monitor Buffer Utilization

Track how full buffers get to tune their size.

### Handle Buffer Overflow

Decide what to do when buffers are full:
- Block producer (default in Weaver)
- Drop data (if acceptable)
- Signal error

### Test Under Load

Test your backpressure handling with various load conditions.

## Verification

Test your backpressure handling:

1. **Fast producer, slow consumer**: Verify producer blocks
2. **Slow producer, fast consumer**: Verify consumer waits
3. **Variable rates**: Test with changing rates
4. **Buffer limits**: Test buffer full/empty conditions

## Next Steps

- Learn about [creating pipelines](./how-to-create-pipelines)
- Understand [channel communication](../explanations/channel-communication)
- Read about [optimizing performance](./how-to-optimize-pipeline-depth)
