---
title: How to Reduce Resource Usage
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Reduce Resource Usage

This guide shows you how to minimize area and power consumption in your Weaver designs.

## Goal

By the end of this guide, you'll be able to:
- Identify resource-heavy operations
- Optimize data types and widths
- Reduce memory usage
- Minimize power consumption

## Prerequisites

- You understand [types](../reference/types)
- You know how [processes work](../reference/processes)
- You've read about [optimization](../explanations/design-philosophy)

## Step 1: Choose Appropriate Data Widths

Use the smallest data width that meets your requirements:

```weaver
// Before: Over-sized
var int<64> counter = 0  // 64 bits, but only needs 8

// After: Right-sized
var int<8> counter = 0  // 8 bits, sufficient for 0-255
```

## Step 2: Optimize Array Sizes

Size arrays appropriately—don't allocate more than needed:

```weaver
// Before: Over-allocated
var int<32> buffer[1024]  // 32KB, but only need 64 entries

// After: Right-sized
var int<32> buffer[64]  // 2KB, sufficient
```

## Step 3: Share Resources

Share resources between processes when possible:

```weaver
// Before: Duplicate resources
func process1() {
    var int<32> lookup_table[256]
    // ...
}

func process2() {
    var int<32> lookup_table[256]  // Duplicate
    // ...
}

// After: Shared resource
func shared_lookup() chan<LookupRequest> {
    var int<32> lookup_table[256]  // Shared
    // ...
}
```

## Step 4: Use Appropriate Types

Choose types that match your data:

```weaver
// Before: Using larger type
var int<32> flag  // 32 bits for a boolean

// After: Using appropriate type
var bool flag  // 1 bit
```

## Step 5: Minimize State

Reduce the amount of state in processes:

```weaver
// Before: Storing unnecessary state
func process() {
    var int<32> a, b, c, d, e  // Many state variables
    // ...
}

// After: Minimal state
func process() {
    var int<32> a, b  // Only what's needed
    // ...
}
```

## Step 6: Optimize Memory Access

Reduce memory accesses and use local variables:

```weaver
// Before: Multiple memory accesses
func process(chan<int> in) {
    var int value
    while {
        await in {
            value = in.recv()
            memory[value] = memory[value] + 1
            memory[value + 1] = memory[value]
        }
    }
}

// After: Local variables
func process(chan<int> in) {
    var int value, temp
    while {
        await in {
            value = in.recv()
            temp = memory[value]
            temp = temp + 1
            memory[value] = temp
            memory[value + 1] = temp
        }
    }
}
```

## Step 7: Power Optimization

Design for lower power consumption:

```weaver
// Use clock gating for inactive processes
func power_efficient_process(chan<wire> enable) {
    while {
        await enable {
            // Only active when enabled
            process_data()
        }
    }
}
```

## Step 8: Complete Example: Resource Optimization

Here's an example showing resource optimization:

```weaver
// Before: Resource-heavy
func unoptimized(chan<int<64>> in) chan<int<64>> out {
    var int<64> buffer[1024]
    var int<64> value, result
    while {
        await in {
            value = in.recv()
            buffer[value % 1024] = value
            result = buffer[value % 1024] * 2
            out.send(result)
        }
    }
}

// After: Optimized
func optimized(chan<int<16>> in) chan<int<16>> out {
    var int<16> buffer[64]  // Smaller buffer
    var int<16> value, result
    while {
        await in {
            value = in.recv()
            buffer[value % 64] = value
            result = buffer[value % 64] * 2
            out.send(result)
        }
    }
}
```

## Best Practices

### Right-Size Everything

Use the smallest data types and structures that meet requirements.

### Profile First

Measure resource usage before optimizing to find actual bottlenecks.

### Balance Trade-offs

Consider trade-offs between resources, performance, and functionality.

### Test After Changes

Verify that optimizations don't break functionality.

## Verification

After optimizing:

1. **Measure resources**: Compare before and after
2. **Verify functionality**: Ensure optimizations don't break behavior
3. **Check performance**: Ensure optimizations don't hurt performance too much

## Next Steps

- Learn about [optimizing pipeline depth](./how-to-optimize-pipeline-depth)
- Understand [process architecture](../explanations/process-architecture)
