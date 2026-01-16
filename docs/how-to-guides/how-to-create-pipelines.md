---
title: How to Create Pipelines
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Create Pipelines

This guide shows you how to build multi-stage processing pipelines in Weaver.

## Goal

By the end of this guide, you'll be able to:
- Connect processes into pipelines
- Handle data flow between stages
- Design efficient pipeline architectures
- Manage pipeline backpressure

## Prerequisites

- You understand [processes](../reference/processes)
- You know how to use [channels](../tutorials/channels)
- You've completed the [Complete Circuit tutorial](../tutorials/complete-circuit)

## Step 1: Design Your Pipeline Stages

Break your computation into stages. Each stage should:
- Take input from the previous stage
- Process the data
- Send output to the next stage

Example: Image processing pipeline
- Stage 1: Load image
- Stage 2: Apply filter
- Stage 3: Detect edges
- Stage 4: Output result

## Step 2: Create Individual Stages

Create each stage as a separate process:

```weaver
// Stage 1: Generate data
func stage1() chan<int<16>> out {
    var int<16> count = 0
    while {
        out.send(count)
        count = count + 1
    }
}

// Stage 2: Transform data
func stage2(chan<int<16>> in) chan<int<16>> out {
    var int<16> value
    while {
        await in {
            value = in.recv()
            out.send(value * 2)  // Double the value
        }
    }
}

// Stage 3: Further processing
func stage3(chan<int<16>> in) chan<int<16>> out {
    var int<16> value
    while {
        await in {
            value = in.recv()
            out.send(value + 1)  // Add one
        }
    }
}
```

## Step 3: Connect Stages with Channels

Connect stages by passing channels between them:

```weaver
func pipeline() {
    var chan<int<16>> s1_out, s2_out, s3_out
    
    // Connect: stage1 -> stage2 -> stage3
    stage1() -> s1_out
    stage2(s1_out) -> s2_out
    stage3(s2_out) -> s3_out
    
    // s3_out is the final output
}
```

## Step 4: Handle Pipeline Composition

You can compose pipelines in parallel or sequence:

### Sequential Pipeline

```weaver
func sequential_pipeline() {
    var chan<int<16>> s1_out, s2_out, s3_out
    stage1() -> s1_out
    stage2(s1_out) -> s2_out
    stage3(s2_out) -> s3_out
}
```

### Parallel Pipelines

Run multiple pipelines in parallel:

```weaver
func parallel_pipelines() {
    var chan<int<16>> p1_out, p2_out, p3_out
    
    // Pipeline 1
    stage1() -> s1_out
    stage2(s1_out) -> p1_out
    
    // Pipeline 2 (parallel)
    stage1() -> s2_out
    stage2(s2_out) -> p2_out
    
    // Pipeline 3 (parallel)
    stage1() -> s3_out
    stage2(s3_out) -> p3_out
}
```

## Step 5: Add Branching and Merging

Pipelines can branch (one input, multiple outputs) or merge (multiple inputs, one output):

### Branching Pipeline

```weaver
func branch_pipeline(chan<int<16>> in) (chan<int<16>> out1, chan<int<16>> out2) {
    var int<16> value
    while {
        await in {
            value = in.recv()
            // Send to both outputs
            out1.send(value)
            out2.send(value)
        }
    }
}
```

### Merging Pipeline

```weaver
func merge_pipeline(chan<int<16>> in1, chan<int<16>> in2) chan<int<16>> out {
    var int<16> value
    while {
        await in1 {
            value = in1.recv()
            out.send(value)
        } or await in2 {
            value = in2.recv()
            out.send(value)
        }
    }
}
```

## Step 6: Implement a Complete Pipeline

Here's a complete example: a CPU instruction pipeline:

```weaver
type Instruction {
    int<32> opcode
    int<5> rs, rt, rd
    int<16> immediate
}

// Stage 1: Instruction Fetch
func fetch(chan<Addr> pc) chan<Instruction> inst {
    var Addr address
    while {
        await pc {
            address = pc.recv()
            inst.send(memory_read(address))
        }
    }
}

// Stage 2: Instruction Decode
func decode(chan<Instruction> inst) (chan<Op> op, chan<Reg> regs) {
    var Instruction instruction
    while {
        await inst {
            instruction = inst.recv()
            op.send({opcode: instruction.opcode})
            regs.send({rs: instruction.rs, rt: instruction.rt, rd: instruction.rd})
        }
    }
}

// Stage 3: Execute
func execute(chan<Op> op, chan<Reg> regs, chan<Data> rf) chan<Result> result {
    var Op operation
    var Reg registers
    var Data data
    while {
        await op & regs {
            operation = op.recv()
            registers = regs.recv()
            data = rf_read(registers.rs, registers.rt)
            
            if operation.opcode == ADD {
                result.send({value: data.rs + data.rt, rd: registers.rd})
            } else if operation.opcode == SUB {
                result.send({value: data.rs - data.rt, rd: registers.rd})
            }
        }
    }
}

// Stage 4: Write Back
func writeback(chan<Result> result, chan<Data> rf) {
    var Result res
    while {
        await result {
            res = result.recv()
            rf_write(res.rd, res.value)
        }
    }
}

// Connect the pipeline
func cpu_pipeline() {
    var chan<Addr> pc
    var chan<Instruction> inst
    var chan<Op> op
    var chan<Reg> regs
    var chan<Result> res
    var chan<Data> rf
    
    fetch(pc) -> inst
    decode(inst) -> (op, regs)
    execute(op, regs, rf) -> res
    writeback(res, rf)
}
```

## Step 7: Handle Pipeline Hazards

Pipelines can have hazards (data dependencies, control dependencies). Handle them explicitly:

### Data Forwarding

Forward data directly to dependent stages:

```weaver
func stage_with_forwarding(chan<int> in, chan<int> forward) chan<int> out {
    var int value
    while {
        await in {
            value = in.recv()
            out.send(value)
            forward.send(value)  // Forward to dependent stage
        }
    }
}
```

### Pipeline Stalls

Stall the pipeline when dependencies aren't ready:

```weaver
func dependent_stage(chan<int> in, chan<int> dependency) chan<int> out {
    var int value, dep_value
    while {
        await in & dependency {
            value = in.recv()
            dep_value = dependency.recv()
            out.send(value + dep_value)
        }
    }
}
```

## Step 8: Optimize Pipeline Depth

Balance pipeline depth (latency) vs throughput:

- **More stages**: Higher throughput, more latency
- **Fewer stages**: Lower latency, less throughput

Adjust based on your requirements:

```weaver
// Deep pipeline (high throughput)
func deep_pipeline() {
    stage1() -> s1
    stage2(s1) -> s2
    stage3(s2) -> s3
    stage4(s3) -> s4
    stage5(s4) -> s5
}

// Shallow pipeline (low latency)
func shallow_pipeline() {
    stage1() -> s1
    stage2(s1) -> s2
    // Fewer stages = less latency
}
```

## Best Practices

### Keep Stages Independent

Each stage should be as independent as possible. This makes pipelines easier to understand and optimize.

### Use Descriptive Names

Name stages clearly: `fetch`, `decode`, `execute` rather than `stage1`, `stage2`, `stage3`.

### Handle Backpressure

Ensure stages can handle backpressure from downstream stages. See [How to Handle Backpressure](./how-to-handle-backpressure).

### Test Each Stage

Test each stage independently before connecting them into a pipeline.

## Verification

Test your pipeline:

1. **Individual stages**: Verify each stage works alone
2. **Connected pipeline**: Test the full pipeline
3. **Throughput**: Measure how much data flows through
4. **Latency**: Measure end-to-end delay
5. **Backpressure**: Test behavior when downstream is slow

## Next Steps

- Learn about [handling backpressure](./how-to-handle-backpressure)
- Understand [process architecture](../explanations/process-architecture)
- Read about [optimizing pipeline depth](./how-to-optimize-pipeline-depth)
