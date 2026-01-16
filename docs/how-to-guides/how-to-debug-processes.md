---
title: How to Debug Processes
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Debug Processes

This guide shows you how to identify and fix common issues in Weaver processes.

## Goal

By the end of this guide, you'll be able to:
- Identify why a process isn't working
- Use validity to trace execution flow
- Fix common process bugs
- Verify your fixes work

## Prerequisites

- You've completed the [First Process tutorial](../tutorials/first-process)
- You understand [validity and truthiness](../reference/validity-truthiness)
- You know how to use [channels](../tutorials/channels)

## Step 1: Identify the Problem

First, determine what's wrong. Common issues include:

- Process hangs (doesn't progress)
- Process never receives data
- Process never sends data
- Wrong values being computed
- Process deadlocks

### Check for Hanging Processes

A process hangs when it's waiting for something that never arrives:

```weaver
func broken(chan<int> in) chan<int> out {
    while {
        await in {  // Hangs if 'in' never receives data
            out.send(in.recv())
        }
    }
}
```

**Symptom**: Process appears to do nothing.

**Solution**: Verify that the channel `in` actually receives data from somewhere.

## Step 2: Trace Validity Flow

Use validity to understand when values are available. Add explicit validity checks:

```weaver
func debug_process(chan<int> in) chan<int> out {
    var int value
    while {
        await in {
            value = in.recv()
            // At this point, 'value' is valid
            // Add your processing here
            out.send(value)
        }
    }
}
```

### Check Validity at Each Step

Break down complex operations and verify validity:

```weaver
func complex_process(chan<int> a, chan<int> b) chan<int> out {
    var int x, y, result
    while {
        await a & b {
            x = a.recv()
            y = b.recv()
            // Both x and y are valid here
            result = x + y
            // result is valid here
            out.send(result)
        }
    }
}
```

## Step 3: Verify Channel Communication

Ensure channels are properly connected:

```weaver
func producer() chan<int> out {
    var int count = 0
    while {
        out.send(count)
        count = count + 1
    }
}

func consumer(chan<int> in) {
    var int value
    while {
        await in {
            value = in.recv()
            // Process value
        }
    }
}

// Make sure they're connected:
func main() {
    var chan<int> data
    producer() -> data
    consumer(data)
}
```

### Common Channel Issues

1. **Unconnected channels**: Producer sends but no consumer receives
   - **Fix**: Connect the channels properly

2. **Missing await**: Trying to receive without waiting
   ```weaver
   // Wrong:
   value = in.recv()  // May hang if nothing available
   
   // Right:
   await in {
       value = in.recv()
   }
   ```

3. **Blocked sender**: Sender waiting for receiver that never receives
   - **Fix**: Ensure receiver calls `recv()` or peeks

## Step 4: Check Composition Operators

Verify that composition operators are used correctly:

### Sequential vs Parallel

```weaver
// Sequential - b depends on a
a = 5
b = a + 1  // Correct: b waits for a

// Parallel - independent operations
a = 5 and b = 2  // Correct: both can happen simultaneously
```

### Conditional Composition

```weaver
// or - must be mutually exclusive
await channelA {
    // ...
} or await channelB {
    // ...
}
// Problem: If both channels ready, this errors
// Fix: Ensure channels are never ready simultaneously

// xor - allows arbitrary choice
await channelA {
    // ...
} xor await channelB {
    // ...
}
// Problem: Non-deterministic if both ready
// Fix: Use 'or' if you need determinism
```

## Step 5: Verify Process Loops

Ensure your process loop can make progress:

```weaver
func problematic() {
    var int x = 0
    while {
        if x < 10 {
            x = x + 1
        }
        // Problem: If x >= 10, loop does nothing
        // Fix: Add else case or break condition
    }
}
```

### Ensure Progress

Every loop iteration should either:
- Make progress toward a goal
- Wait for external input
- Have a way to exit (if not a perpetual process)

## Step 6: Test Incrementally

Build and test your process incrementally:

1. **Start simple**: Get basic structure working
   ```weaver
   func simple() chan<int> out {
       while {
           out.send(0)
       }
   }
   ```

2. **Add complexity gradually**: Add one feature at a time
   ```weaver
   func simple() chan<int> out {
       var int count = 0
       while {
           out.send(count)
           count = count + 1
       }
   }
   ```

3. **Test each addition**: Verify it works before adding more

## Step 7: Use Simulation Tools

Use the Weaver toolchain to simulate and debug:

```bash
# Build your design
lm build your-process.weaver

# Run simulation (if available)
lm sim your-process.weaver

# Check for warnings
lm build your-process.weaver --warn-all
```

## Common Bugs and Fixes

### Bug: Process Never Starts

**Symptom**: Process appears inactive.

**Possible causes**:
- Missing channel connection
- Process waiting for input that never arrives
- Reset logic incorrect

**Fix**: Check channel connections and initial state.

### Bug: Deadlock

**Symptom**: System hangs, nothing progresses.

**Possible causes**:
- Circular dependency (A waits for B, B waits for A)
- All processes waiting for each other

**Fix**: Break the circular dependency or add a process that doesn't wait.

### Bug: Wrong Values

**Symptom**: Process computes incorrect results.

**Possible causes**:
- Using invalid values
- Wrong operator precedence
- Validity not propagating correctly

**Fix**: Add validity checks and verify operator usage.

## Verification

After fixing your process:

1. **Test with simple inputs**: Verify basic functionality
2. **Test edge cases**: Empty channels, boundary values
3. **Test under load**: Multiple concurrent operations
4. **Check resource usage**: Ensure reasonable area/power

## Next Steps

- Learn about [testing processes](./how-to-test-processes)
- Understand [validity behavior](../explanations/validity-system)
- Read about [process architecture](../explanations/process-architecture)
