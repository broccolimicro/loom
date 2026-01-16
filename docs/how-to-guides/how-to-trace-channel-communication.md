---
title: How to Trace Channel Communication
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Trace Channel Communication

This guide shows you how to understand and debug data flow through channels in your Weaver designs.

## Goal

By the end of this guide, you'll be able to:
- Trace data flow through channels
- Identify where data gets stuck
- Debug channel communication issues
- Verify correct data propagation

## Prerequisites

- You understand [channels](../tutorials/channels)
- You know how [processes work](../reference/processes)
- You've read about [channel communication](../explanations/channel-communication)

## Step 1: Add Tracing to Channels

Add logging or tracing to see when data flows:

```weaver
func traced_channel(chan<int> in) chan<int> out {
    var int value
    while {
        await in {
            value = in.recv()
            // Trace: received value
            trace_receive(value)
            out.send(value)
            // Trace: sent value
            trace_send(value)
        }
    }
}
```

## Step 2: Trace Send Operations

Track when data is sent:

```weaver
func producer_with_trace() chan<int> out {
    var int<32> count = 0
    while {
        // Trace: about to send
        trace("Sending: ", count)
        out.send(count)
        // Trace: sent
        trace("Sent: ", count)
        count = count + 1
    }
}
```

## Step 3: Trace Receive Operations

Track when data is received:

```weaver
func consumer_with_trace(chan<int> in) {
    var int<32> value
    while {
        // Trace: waiting for data
        trace("Waiting for data")
        await in {
            value = in.recv()
            // Trace: received
            trace("Received: ", value)
            process(value)
        }
    }
}
```

## Step 4: Trace Channel State

Monitor channel state (ready, valid, etc.):

```weaver
func trace_channel_state(chan<int> ch) {
    while {
        if valid(ch) == vdd {
            trace("Channel has data")
            var int value = ch  // Peek without receiving
            trace("Channel value: ", value)
        } else {
            trace("Channel empty")
        }
    }
}
```

## Step 5: Add Tracing Probes

Insert tracing processes into your pipeline:

```weaver
func trace_probe(chan<int> in, chan<int> out) {
    var int<32> value, count = 0
    while {
        await in {
            value = in.recv()
            // Trace: probe sees value
            trace("Probe [", count, "]: ", value)
            count = count + 1
            out.send(value)
        }
    }
}

// Insert probe into pipeline
func traced_pipeline() {
    var chan<int<32>> s1, s2, s3
    
    stage1() -> s1
    trace_probe(s1, s2)  // Probe between stage1 and stage2
    stage2(s2) -> s3
}
```

## Step 6: Trace Multiple Channels

Track multiple channels simultaneously:

```weaver
func trace_multiple(
    chan<int> ch1, chan<int> ch2, chan<int> ch3
) {
    while {
        if valid(ch1) == vdd {
            trace("ch1: ", ch1)
        }
        if valid(ch2) == vdd {
            trace("ch2: ", ch2)
        }
        if valid(ch3) == vdd {
            trace("ch3: ", ch3)
        }
    }
}
```

## Step 7: Identify Blocked Channels

Find channels that are blocked:

```weaver
func detect_blocked(chan<int> in, chan<int> out) {
    var int<32> value
    var int<64> wait_count = 0
    
    while {
        // Try to receive
        if valid(in) == vdd {
            value = in.recv()
            wait_count = 0  // Reset counter
            out.send(value)
        } else {
            wait_count = wait_count + 1
            if wait_count > threshold {
                trace("Channel blocked! Wait count: ", wait_count)
            }
        }
    }
}
```

## Step 8: Complete Example: Tracing System

Here's a complete example with comprehensive tracing:

```weaver
func traced_system() {
    var chan<int<32>> producer_out, consumer_in
    var int<32> trace_count = 0
    
    // Producer with tracing
    func traced_producer() chan<int<32>> out {
        var int<32> count = 0
        while {
            trace("PRODUCER: Sending ", count)
            out.send(count)
            trace("PRODUCER: Sent ", count)
            count = count + 1
        }
    }
    
    // Consumer with tracing
    func traced_consumer(chan<int<32>> in) {
        var int<32> value
        while {
            trace("CONSUMER: Waiting")
            await in {
                value = in.recv()
                trace("CONSUMER: Received ", value)
                process(value)
            }
        }
    }
    
    // Channel monitor
    func monitor(chan<int<32>> ch) {
        while {
            if valid(ch) == vdd {
                trace("MONITOR: Channel has data: ", ch)
            } else {
                trace("MONITOR: Channel empty")
            }
        }
    }
    
    // Connect with monitoring
    traced_producer() -> producer_out
    monitor(producer_out)  // Monitor in parallel
    traced_consumer(producer_out)
}
```

## Best Practices

### Trace Selectively

Don't trace everything—focus on channels you're debugging.

### Use Clear Labels

Use descriptive labels in trace output to identify sources.

### Trace at Key Points

Trace at:
- Channel sends
- Channel receives
- Process boundaries
- Decision points

### Keep Traces Minimal

Too much tracing can slow down simulation. Trace only what you need.

## Verification

After adding tracing:

1. **Run your design**: Execute with tracing enabled
2. **Analyze traces**: Look for patterns and issues
3. **Identify problems**: Find where data gets stuck
4. **Fix issues**: Address problems found in traces

## Common Issues Found Through Tracing

### Issue: Data Never Sent

**Trace shows**: Producer never sends.

**Possible causes**:
- Producer blocked waiting for something
- Producer logic error

**Fix**: Check producer logic and dependencies.

### Issue: Data Never Received

**Trace shows**: Consumer never receives.

**Possible causes**:
- Channel not connected
- Consumer blocked
- Producer not sending

**Fix**: Verify channel connections and consumer logic.

### Issue: Data Stuck in Channel

**Trace shows**: Data sent but never received.

**Possible causes**:
- Consumer not calling recv()
- Channel connection issue

**Fix**: Verify consumer receives data and channel connections.

## Next Steps

- Learn about [debugging processes](./how-to-debug-processes)
- Understand [channel communication](../explanations/channel-communication)
- Read about [verifying validity](./how-to-verify-validity)
