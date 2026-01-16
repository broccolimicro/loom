---
title: How to Test Processes
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Test Processes

This guide shows you how to verify that your Weaver processes work correctly.

## Goal

By the end of this guide, you'll be able to:
- Write test processes to verify behavior
- Test with various inputs
- Verify outputs are correct
- Test edge cases and error conditions

## Prerequisites

- You understand [processes](../reference/processes)
- You know how to use [channels](../tutorials/channels)
- You've completed the [tutorials](../tutorials/index)

## Step 1: Create a Test Harness

Build a test process that feeds inputs and checks outputs:

```weaver
func test_harness() {
    var chan<int<32>> input, output
    var int<32> test_value, result
    
    // Create process under test
    process_under_test(input) -> output
    
    // Send test input
    input.send(5)
    
    // Check output
    await output {
        result = output.recv()
        if result == 10 {
            // Test passed
        } else {
            // Test failed
        }
    }
}
```

## Step 2: Test with Multiple Inputs

Test your process with various input values:

```weaver
func test_multiple_inputs() {
    var chan<int<32>> input, output
    var int<32> test_cases[5] = [1, 2, 5, 10, 100]
    var int<32> i = 0, result
    
    process_under_test(input) -> output
    
    while i < 5 {
        input.send(test_cases[i])
        await output {
            result = output.recv()
            verify_result(test_cases[i], result)
        }
        i = i + 1
    }
}
```

## Step 3: Test Edge Cases

Test boundary conditions and edge cases:

```weaver
func test_edge_cases() {
    var chan<int<32>> input, output
    var int<32> edge_cases[4] = [0, 1, -1, 1000000]
    var int<32> i = 0, result
    
    process_under_test(input) -> output
    
    while i < 4 {
        input.send(edge_cases[i])
        await output {
            result = output.recv()
            verify_edge_case(edge_cases[i], result)
        }
        i = i + 1
    }
}
```

## Step 4: Test Channel Communication

Verify that channel communication works correctly:

```weaver
func test_channels() {
    var chan<int<32>> in1, in2, out
    var int<32> val1, val2, result
    
    multi_input_process(in1, in2) -> out
    
    // Send to both inputs
    in1.send(5)
    in2.send(3)
    
    // Verify output
    await out {
        result = out.recv()
        if result == 8 {  // 5 + 3
            // Test passed
        }
    }
}
```

## Step 5: Test Parallel Composition

Verify parallel composition works correctly:

```weaver
func test_parallel() {
    var int<32> a, b, c
    a-, b-, c-
    
    (
        a = 5
        await b
        c = a + b
    ) and (
        await a
        b = 3
    )
    
    // Verify results
    await c {
        if c == 8 {
            // Test passed
        }
    }
}
```

## Step 6: Test State Machines

Test state machine transitions:

```weaver
func test_state_machine() {
    var chan<Event> events
    var chan<State> state_output
    var Event event
    var State state
    
    state_machine(events) -> state_output
    
    // Test transition: IDLE -> ACTIVE
    event = {type: START}
    events.send(event)
    await state_output {
        state = state_output.recv()
        if state.value == ACTIVE {
            // Transition correct
        }
    }
    
    // Test transition: ACTIVE -> DONE
    event = {type: COMPLETE}
    events.send(event)
    await state_output {
        state = state_output.recv()
        if state.value == DONE {
            // Transition correct
        }
    }
}
```

## Step 7: Test Error Handling

Verify error handling works correctly:

```weaver
func test_errors() {
    var chan<int<32>> input
    var chan<Result> output
    var int<32> invalid_input = -1
    var Result result
    
    process_with_errors(input) -> output
    
    // Test invalid input
    input.send(invalid_input)
    await output {
        result = output.recv()
        if result.error.occurred {
            // Error detected correctly
        }
    }
}
```

## Step 8: Complete Example: Test Suite

Here's a complete test suite example:

```weaver
func test_suite() {
    // Test 1: Basic functionality
    test_basic_functionality()
    
    // Test 2: Multiple inputs
    test_multiple_inputs()
    
    // Test 3: Edge cases
    test_edge_cases()
    
    // Test 4: Channel communication
    test_channels()
    
    // Test 5: Parallel composition
    test_parallel()
    
    // Test 6: Error handling
    test_errors()
}

func test_basic_functionality() {
    var chan<int<32>> input, output
    var int<32> test_value = 5, expected = 10, result
    
    double_process(input) -> output
    
    input.send(test_value)
    await output {
        result = output.recv()
        if result == expected {
            // Pass
        } else {
            // Fail: expected 10, got result
        }
    }
}

func double_process(chan<int<32>> in) chan<int<32>> out {
    var int<32> value
    while {
        await in {
            value = in.recv()
            out.send(value * 2)
        }
    }
}
```

## Best Practices

### Test Incrementally

Test each feature as you add it, not just at the end.

### Test Edge Cases

Don't just test the happy path—test boundaries, null values, and error conditions.

### Verify Outputs

Always check that outputs match expected values.

### Test in Isolation

Test processes individually before testing them in larger systems.

### Document Tests

Clearly document what each test verifies.

## Verification

After writing tests:

1. **Run tests**: Execute your test suite
2. **Check results**: Verify all tests pass
3. **Fix failures**: Address any test failures
4. **Re-run**: Verify fixes work

## Common Testing Patterns

### Pattern: Golden Reference

Compare output against known-good reference:

```weaver
func test_with_reference() {
    var chan<int> input, output
    var int<32> test_value, result, expected
    
    process(input) -> output
    expected = reference_implementation(test_value)
    
    input.send(test_value)
    await output {
        result = output.recv()
        if result == expected {
            // Pass
        }
    }
}
```

### Pattern: Property-Based Testing

Test that properties hold for all inputs:

```weaver
func test_property() {
    var chan<int> input, output
    var int<32> value, result
    
    process(input) -> output
    
    // Test property: result should always be positive
    while value < 1000 {
        input.send(value)
        await output {
            result = output.recv()
            if result < 0 {
                // Property violated
            }
        }
        value = value + 1
    }
}
```

## Next Steps

- Learn about [debugging processes](./how-to-debug-processes)
- Understand [process architecture](../explanations/process-architecture)
- Read about [simulating circuits](./how-to-simulate-circuits)
