---
title: How to Optimize Pipeline Depth
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Optimize Pipeline Depth

This guide shows you how to balance pipeline depth (latency) and throughput in your Weaver designs.

## Goal

By the end of this guide, you'll be able to:
- Understand the trade-off between depth and throughput
- Identify when to add or remove pipeline stages
- Balance latency and performance
- Optimize your pipeline architecture

## Prerequisites

- You understand [pipelines](./how-to-create-pipelines)
- You know how [processes work](../reference/processes)
- You've read about [process architecture](../explanations/process-architecture)

## Step 1: Understand the Trade-off

Pipeline depth affects two key metrics:

- **Throughput**: How much work completes per unit time
- **Latency**: How long it takes for one item to complete

**Deeper pipelines**:
- Higher throughput (more parallelism)
- Higher latency (more stages to traverse)

**Shallower pipelines**:
- Lower latency (fewer stages)
- Lower throughput (less parallelism)

## Step 2: Identify Bottlenecks

Find stages that limit overall performance:

```weaver
func analyze_pipeline() {
    // Stage 1: Fast (1 cycle)
    stage1() -> s1
    
    // Stage 2: Slow (10 cycles) - BOTTLENECK
    stage2(s1) -> s2
    
    // Stage 3: Fast (1 cycle)
    stage3(s2) -> s3
}
```

The slowest stage limits throughput. Consider splitting it:

```weaver
// Split slow stage into multiple stages
stage2a(s1) -> s2a
stage2b(s2a) -> s2b
stage2c(s2b) -> s2
```

## Step 3: Add Pipeline Stages

Break complex operations into multiple stages:

### Before: Single Slow Stage

```weaver
func slow_stage(chan<int> in) chan<int> out {
    var int value, result
    while {
        await in {
            value = in.recv()
            // Complex computation (slow)
            result = complex_compute(value)
            out.send(result)
        }
    }
}
```

### After: Multiple Fast Stages

```weaver
func stage_a(chan<int> in) chan<int> out {
    var int value
    while {
        await in {
            value = in.recv()
            out.send(step1(value))
        }
    }
}

func stage_b(chan<int> in) chan<int> out {
    var int value
    while {
        await in {
            value = in.recv()
            out.send(step2(value))
        }
    }
}

func stage_c(chan<int> in) chan<int> out {
    var int value
    while {
        await in {
            value = in.recv()
            out.send(step3(value))
        }
    }
}

// Connect: stage_a -> stage_b -> stage_c
```

## Step 4: Balance Stage Work

Ensure stages have roughly equal work:

```weaver
// Unbalanced: stage1 is much slower
stage1() -> s1      // 10 cycles
stage2(s1) -> s2    // 1 cycle
stage3(s2) -> s3     // 1 cycle

// Balanced: stages have similar work
stage1a() -> s1a    // 4 cycles
stage1b(s1a) -> s1b // 3 cycles
stage1c(s1b) -> s1   // 3 cycles
stage2(s1) -> s2     // 4 cycles
stage3(s2) -> s3      // 4 cycles
```

## Step 5: Consider Data Dependencies

Some operations can't be pipelined due to dependencies:

```weaver
// Can't pipeline: b depends on a
a = compute()
b = a + 1  // Must wait for a

// Can pipeline: independent operations
stage1() -> s1
stage2(s1) -> s2
stage3(s2) -> s3
```

## Step 6: Handle Pipeline Hazards

Pipeline hazards can reduce effective throughput:

### Data Hazards

Forward data to dependent stages:

```weaver
func stage_with_forwarding(chan<int> in) (chan<int> out, chan<int> forward) {
    var int value
    while {
        await in {
            value = in.recv()
            out.send(process(value))
            forward.send(value)  // Forward to dependent stage
        }
    }
}
```

### Control Hazards

Handle branches efficiently:

```weaver
func branch_handler(chan<int> in) (chan<int> true_path, chan<int> false_path) {
    var int value
    while {
        await in {
            value = in.recv()
            if value > 0 {
                true_path.send(value)
            } else {
                false_path.send(value)
            }
        }
    }
}
```

## Step 7: Measure Performance

Measure actual performance to guide optimization:

```weaver
func performance_test() {
    var chan<Data> input, output
    var int<64> start_time, end_time, latency
    
    pipeline(input) -> output
    
    // Measure latency
    start_time = get_time()
    input.send(test_data)
    await output {
        output.recv()
        end_time = get_time()
        latency = end_time - start_time
    }
    
    // Measure throughput
    var int<32> count = 0
    while count < 1000 {
        input.send(test_data)
        await output {
            output.recv()
            count = count + 1
        }
    }
    // Calculate throughput = count / time
}
```

## Step 8: Complete Example: Optimized Pipeline

Here's an example showing pipeline optimization:

```weaver
// Initial: Single stage (low throughput)
func initial(chan<int> in) chan<int> out {
    var int value
    while {
        await in {
            value = in.recv()
            // All work in one stage (slow)
            out.send(step1(step2(step3(value))))
        }
    }
}

// Optimized: Multiple stages (high throughput)
func optimized() {
    var chan<int> s1, s2, s3
    
    // Split into stages
    func stage1(chan<int> in) chan<int> out {
        var int value
        while {
            await in {
                value = in.recv()
                out.send(step1(value))
            }
        }
    }
    
    func stage2(chan<int> in) chan<int> out {
        var int value
        while {
            await in {
                value = in.recv()
                out.send(step2(value))
            }
        }
    }
    
    func stage3(chan<int> in) chan<int> out {
        var int value
        while {
            await in {
                value = in.recv()
                out.send(step3(value))
            }
        }
    }
    
    // Connect pipeline
    stage1(input) -> s1
    stage2(s1) -> s2
    stage3(s2) -> output
}
```

## Best Practices

### Start Simple

Begin with a simple pipeline and optimize based on measurements.

### Measure Before Optimizing

Don't optimize blindly—measure to find actual bottlenecks.

### Balance Stages

Try to make stages have similar work to maximize throughput.

### Consider Latency Requirements

If latency is critical, use fewer stages even if throughput suffers.

### Test After Changes

Always verify that optimizations don't break functionality.

## Verification

After optimizing:

1. **Measure performance**: Compare before and after
2. **Verify correctness**: Ensure functionality is preserved
3. **Check resource usage**: Ensure optimizations don't increase area/power too much
4. **Test edge cases**: Verify optimizations work in all cases

## Common Optimization Patterns

### Pattern: Split Slow Stages

Break slow stages into multiple faster stages.

### Pattern: Merge Fast Stages

If stages are very fast, consider merging them to reduce latency.

### Pattern: Parallel Paths

Use parallel paths for independent operations:

```weaver
stage1() -> (s1a, s1b)
stage2a(s1a) -> s2a
stage2b(s1b) -> s2b
merge(s2a, s2b) -> output
```

## Next Steps

- Learn about [creating pipelines](./how-to-create-pipelines)
- Understand [process architecture](../explanations/process-architecture)
- Read about [reducing resource usage](./how-to-reduce-resource-usage)
