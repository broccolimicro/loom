---
title: Working with Channels
category: Tutorials
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Working with Channels

Channels are the primary way processes communicate in Weaver. This tutorial explores channel communication patterns, including sending, receiving, and conditional channel operations.

## What You'll Learn

- Different ways to interact with channels
- Conditional channel operations with `or` and `xor`
- Waiting for multiple channels
- Channel peeking vs receiving

## Step 1: Basic Channel Operations

Let's review the three ways to interact with channels:

### Sending

```weaver
channel.send(value)  // Blocks until receiver is ready
```

### Receiving

```weaver
value = channel.recv()  // Removes value, unblocks sender
```

### Peeking

```weaver
value = channel  // Reads without removing, sender stays blocked
```

## Step 2: Conditional Channel Operations

Weaver provides `or` and `xor` for handling multiple channels conditionally.

### Using `or` for Mutually Exclusive Channels

Create `multiplexer.weaver`:

```weaver
// multiplexer.weaver - Selects from two mutually exclusive inputs

func multiplexer(chan<int<16>> inputA, chan<int<16>> inputB) chan<int<16>> output {
    var int<16> value
    
    while {
        await inputA {
            value = inputA.recv()
            output.send(value)
        } or await inputB {
            value = inputB.recv()
            output.send(value)
        }
    }
}
```

The `or` operator ensures that only one path executes. If both `inputA` and `inputB` have data simultaneously, it's an error - they must be mutually exclusive.

### Using `xor` for Non-Deterministic Choice

Create `arbiter.weaver`:

```weaver
// arbiter.weaver - Arbitrarily selects from available inputs

func arbiter(chan<int<16>> inputA, chan<int<16>> inputB) chan<int<16>> output {
    var int<16> value
    
    while {
        await inputA {
            value = inputA.recv()
            output.send(value)
        } xor await inputB {
            value = inputB.recv()
            output.send(value)
        }
    }
}
```

The `xor` operator allows non-deterministic behavior. If both channels have data, one is chosen arbitrarily.

## Step 3: Waiting for Multiple Channels

Sometimes you need to wait for data on multiple channels before proceeding:

```weaver
// adder.weaver - Adds values from two channels

func adder(chan<int<16>> inputA, chan<int<16>> inputB) chan<int<16>> sum {
    var int<16> a, b
    
    while {
        await inputA & inputB {
            a = inputA.recv()
            b = inputB.recv()
            sum.send(a + b)
        }
    }
}
```

The `&` operator in the `await` condition means "wait until both are valid".

## Step 4: Building a Complete Example

Let's create a system that demonstrates these patterns. Create `channel-demo.weaver`:

```weaver
// channel-demo.weaver - Demonstrates channel communication patterns

// Producer that sends sequential values
func producer(chan<int<8>> out) {
    var int<8> count = 0
    
    while {
        out.send(count)
        count = count + 1
        if count == 10 {
            count = 0
        }
    }
}

// Consumer that receives and processes values
func consumer(chan<int<8>> in) {
    var int<8> value
    
    while {
        await in {
            value = in.recv()
            // Process value here
        }
    }
}

// Router that can handle two inputs
func router(chan<int<8>> inA, chan<int<8>> inB, chan<int<8>> out) {
    var int<8> value
    
    while {
        await inA {
            value = inA.recv()
            out.send(value)
        } or await inB {
            value = inB.recv()
            out.send(value)
        }
    }
}
```

## Step 5: Understanding Channel Blocking

It's important to understand when channels block:

- **Sender blocks** until receiver calls `recv()` or peeks
- **Receiver blocks** (in `await`) until sender calls `send()`
- **Peek doesn't unblock sender** - use `recv()` to unblock

This blocking behavior ensures proper synchronization between processes.

## Step 6: Build and Test

```bash
lm build channel-demo.weaver
```

## Step 7: Advanced Pattern - Pipeline

Create a pipeline of processes:

```weaver
// pipeline.weaver - A three-stage pipeline

func stage1() chan<int<16>> out {
    var int<16> count = 0
    while {
        out.send(count)
        count = count + 1
    }
}

func stage2(chan<int<16>> in) chan<int<16>> out {
    var int<16> value
    while {
        await in {
            value = in.recv()
            out.send(value * 2)  // Double the value
        }
    }
}

func stage3(chan<int<16>> in) chan<int<16>> out {
    var int<16> value
    while {
        await in {
            value = in.recv()
            out.send(value + 1)  // Add one
        }
    }
}

// Connect the pipeline
func main() {
    var chan<int<16>> s1_out, s2_out, s3_out
    
    stage1() -> s1_out
    stage2(s1_out) -> s2_out
    stage3(s2_out) -> s3_out
}
```

Data flows: stage1 → stage2 → stage3, with each stage processing the data.

## Key Concepts

- **`or`** enforces mutual exclusion (deterministic)
- **`xor`** allows arbitrary choice (non-deterministic)
- **`&` in await** waits for multiple channels
- **Channels block** to ensure proper synchronization
- **Pipelines** are created by connecting process outputs to inputs

## Common Patterns

### Pattern: Producer-Consumer

```weaver
func producer(chan<int<8>> out) {
    // Generate data
    out.send(data)
}

func consumer(chan<int<8>> in) {
    await in {
        var int<8> data = in.recv()
        // Process data
    }
}
```

### Pattern: Conditional Processing

```weaver
await condition {
    // Process when condition is true
} or await ~condition {
    // Process when condition is false
}
```

## What's Next?

Now you understand channel communication! In the next tutorial, you'll:
- Combine processes, channels, and custom types
- Build a complete, functional circuit
- Use structures for circuit organization

Continue to [Building a Complete Circuit](./complete-circuit) to put everything together.
