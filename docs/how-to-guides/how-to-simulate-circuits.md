---
title: How to Simulate Circuits
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Simulate Circuits

This guide shows you how to run and debug your Weaver designs through simulation.

## Goal

By the end of this guide, you'll be able to:
- Run simulations of your Weaver designs
- Debug through simulation
- Verify correct behavior
- Analyze simulation results

## Prerequisites

- You have Loom (`lm`) installed
- You understand [processes](../reference/processes)
- You've completed the [tutorials](../tutorials/index)

## Step 1: Build Your Design

First, build your Weaver design:

```bash
lm build your-design.weaver
```

This compiles your design and prepares it for simulation.

## Step 2: Run Basic Simulation

Run a simulation of your design:

```bash
lm sim your-design.weaver
```

This executes your design and shows basic output.

## Step 3: Add Test Stimuli

Create test inputs for your simulation:

```weaver
func test_stimuli() chan<int<32>> test_data {
    var int<32> test_values[10] = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    var int<4> i = 0
    
    while i < 10 {
        test_data.send(test_values[i])
        i = i + 1
    }
}
```

## Step 4: Monitor Outputs

Add monitoring to observe outputs:

```weaver
func monitor_output(chan<int<32>> output) {
    var int<32> value, count = 0
    
    while {
        await output {
            value = output.recv()
            print("Output [", count, "]: ", value)
            count = count + 1
        }
    }
}
```

## Step 5: Run Simulation with Monitoring

Connect test stimuli and monitoring:

```weaver
func simulation_setup() {
    var chan<int<32>> test_data, design_output
    
    test_stimuli() -> test_data
    your_design(test_data) -> design_output
    monitor_output(design_output)
}
```

## Step 6: Debug Through Simulation

Use simulation to debug issues:

```weaver
func debug_simulation() {
    var chan<int<32>> input, output
    var int<32> value
    
    // Add debug prints
    func debug_process(chan<int<32>> in) chan<int<32>> out {
        var int<32> val
        while {
            await in {
                val = in.recv()
                print("DEBUG: Received ", val)
                out.send(process(val))
                print("DEBUG: Sent ", process(val))
            }
        }
    }
    
    debug_process(input) -> output
}
```

## Step 7: Analyze Results

Compare simulation results with expected values:

```weaver
func verify_simulation() {
    var chan<int<32>> input, output
    var int<32> expected[10] = [2, 4, 6, 8, 10, 12, 14, 16, 18, 20]
    var int<32> actual, i = 0
    
    your_design(input) -> output
    
    while i < 10 {
        input.send(i + 1)
        await output {
            actual = output.recv()
            if actual == expected[i] {
                print("PASS: Expected ", expected[i], ", got ", actual)
            } else {
                print("FAIL: Expected ", expected[i], ", got ", actual)
            }
            i = i + 1
        }
    }
}
```

## Step 8: Complete Example: Simulation

Here's a complete simulation example:

```weaver
func simulation_example() {
    var chan<int<32>> test_input, design_output
    
    // Test generator
    func test_generator() chan<int<32>> out {
        var int<32> i = 0
        while i < 100 {
            out.send(i)
            i = i + 1
        }
    }
    
    // Design under test
    func design_under_test(chan<int<32>> in) chan<int<32>> out {
        var int<32> value
        while {
            await in {
                value = in.recv()
                out.send(value * 2)
            }
        }
    }
    
    // Monitor
    func monitor(chan<int<32>> output) {
        var int<32> value, count = 0
        while {
            await output {
                value = output.recv()
                print("Result [", count, "]: ", value)
                count = count + 1
            }
        }
    }
    
    // Connect
    test_generator() -> test_input
    design_under_test(test_input) -> design_output
    monitor(design_output)
}
```

## Best Practices

### Start Simple

Begin with simple simulations and add complexity gradually.

### Verify Incrementally

Test each component before testing the whole system.

### Use Clear Test Cases

Create test cases that clearly show expected behavior.

### Document Results

Keep records of simulation results for comparison.

## Verification

After simulation:

1. **Check outputs**: Verify outputs match expectations
2. **Review timing**: Ensure timing is correct
3. **Check for errors**: Look for simulation errors or warnings
4. **Compare runs**: Compare different simulation runs

## Next Steps

- Learn about [testing processes](./how-to-test-processes)
- Understand [debugging processes](./how-to-debug-processes)
