---
title: How to Reset Processes
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Reset Processes

This guide shows you how to properly initialize and reset Weaver processes.

## Goal

By the end of this guide, you'll be able to:
- Initialize process state during reset
- Reset processes to known states
- Handle reset in complex processes
- Verify reset behavior

## Prerequisites

- You understand [processes](../reference/processes)
- You know how processes handle [reset](../reference/processes.md#reset-initialization)

## Step 1: Understand Reset Behavior

In Weaver, all statements before the `while` loop execute during reset:

```weaver
func process() {
    var int<32> count = 0  // Executes during reset
    var int<32> max = 100  // Executes during reset
    
    while {
        // Loop body executes after reset
    }
}
```

## Step 2: Initialize State Variables

Initialize all state variables before the loop:

```weaver
func counter() chan<int<32>> out {
    var int<32> count = 0      // Reset: count starts at 0
    var int<32> increment = 1   // Reset: increment is 1
    
    while {
        out.send(count)
        count = count + increment
    }
}
```

## Step 3: Initialize Complex Types

Reset works with complex types too:

```weaver
type State {
    int<2> current
    int<32> counter
    bool active
}

func state_machine() {
    var State state = {current: 0, counter: 0, active: false}  // Reset values
    
    while {
        // Use state
    }
}
```

## Step 4: Reset Arrays

Initialize arrays during reset:

```weaver
func buffer() {
    var int<32> data[16] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    var int<4> head = 0
    var int<4> tail = 0
    
    while {
        // Use buffer
    }
}
```

Or use a loop to initialize (if supported):

```weaver
func buffer() {
    var int<32> data[16]
    var int<4> i = 0
    
    // Initialize array during reset
    while i < 16 {
        data[i] = 0
        i = i + 1
    }
    
    var int<4> head = 0
    var int<4> tail = 0
    
    while {
        // Use buffer
    }
}
```

## Step 5: Reset Multiple Processes

Each process resets independently:

```weaver
func process1() chan<int> out {
    var int count = 0  // Resets to 0
    while {
        out.send(count)
        count = count + 1
    }
}

func process2(chan<int> in) {
    var int received = 0  // Resets to 0
    while {
        await in {
            received = in.recv()
        }
    }
}

func system() {
    var chan<int> data
    process1() -> data
    process2(data)
    // Both processes reset independently
}
```

## Step 6: Handle Reset Signals

If you need to reset a process during operation, use a reset channel:

```weaver
func resettable_process(chan<wire> reset) chan<int> out {
    var int<32> count = 0  // Initial reset
    
    while {
        await reset {
            reset.recv()
            count = 0  // Reset to initial value
        } and {
            out.send(count)
            count = count + 1
        }
    }
}
```

## Step 7: Reset with External State

Reset can depend on external configuration:

```weaver
func configurable_process(int<32> initial_value) chan<int> out {
    var int<32> count = initial_value  // Reset to provided value
    
    while {
        out.send(count)
        count = count + 1
    }
}
```

## Step 8: Complete Example: Reset System

Here's a complete example showing proper reset handling:

```weaver
type CounterState {
    int<32> value
    int<32> max
    bool enabled
}

func resettable_counter(chan<wire> reset, chan<wire> enable) chan<int<32>> out {
    var CounterState state = {value: 0, max: 100, enabled: true}
    
    while {
        await reset {
            reset.recv()
            // Reset to initial state
            state.value = 0
            state.enabled = true
        } and await enable {
            enable.recv()
            state.enabled = !state.enabled
        } and if state.enabled {
            if state.value < state.max {
                out.send(state.value)
                state.value = state.value + 1
            } else {
                state.value = 0  // Wrap around
            }
        }
    }
}
```

## Best Practices

### Initialize All State

Ensure all state variables are initialized during reset. Uninitialized state leads to undefined behavior.

### Use Meaningful Initial Values

Choose initial values that make sense for your process. Often this is zero or a known safe state.

### Document Reset Behavior

Clearly document what state your process resets to and why.

### Test Reset

Verify that your process resets correctly to the expected initial state.

## Verification

Test your reset behavior:

1. **Initial state**: Verify process starts in correct state
2. **Reset signal**: Test reset during operation (if applicable)
3. **Multiple resets**: Verify multiple resets work correctly
4. **State consistency**: Ensure all state resets together

## Common Issues

### Issue: State Not Resetting

**Symptom**: Process doesn't return to initial state.

**Possible causes**:
- State variable not initialized before loop
- Reset logic incorrect

**Fix**: Ensure all state is initialized before the `while` loop.

### Issue: Partial Reset

**Symptom**: Some state resets, some doesn't.

**Possible causes**:
- Not all state variables initialized
- Reset logic incomplete

**Fix**: Initialize all state variables during reset.

## Next Steps

- Learn about [debugging processes](./how-to-debug-processes)
- Understand [process architecture](../explanations/process-architecture)
- Read about [handling errors](./how-to-handle-errors)
