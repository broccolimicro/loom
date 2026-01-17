---
title: Building a Complete Circuit
category: Tutorials
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Building a Complete Circuit

In this final tutorial, you'll build a complete, functional circuit that combines all the concepts you've learned: processes, channels, custom types, and composition.

## What You'll Learn

- Defining custom types for structured data
- Building a multi-component system
- Using parallel composition
- Creating a realistic hardware component

## Step 1: Define Custom Types

We'll build a simple ALU (Arithmetic Logic Unit) component. First, let's define the types we'll need:

```weaver
// alu.weaver - A simple ALU implementation

// Operation type
type opcode {
    uint<2> op  // 0=add, 1=subtract, 2=multiply, 3=pass
}

// ALU input packet
type alu_input {
    opcode op
    int<16> a, b
}

// ALU output
type alu_output {
    int<16> result
    bool valid
}
```

## Step 2: Create the ALU Process

Now let's implement the ALU:

```weaver
func alu(chan<alu_input> input) chan<alu_output> output {
    var alu_input in
    var alu_output out
    
    while {
        await input {
            in = input.recv()
            
            // Perform operation based on opcode
            if in.op.op == 0 {
                // Add
                out.result = in.a + in.b
                out.valid = true
            } or if in.op.op == 1 {
                // Subtract
                out.result = in.a - in.b
                out.valid = true
            } or if in.op.op == 2 {
                // Multiply (simplified)
                out.result = in.a * in.b
                out.valid = true
            } or if in.op.op == 3 {
                // Pass through
                out.result = in.a
                out.valid = true
            }
            
            output.send(out)
            out.valid = false
        }
    }
}
```

## Step 3: Create a Test Harness

Let's create a test process that feeds the ALU:

```weaver
// Test harness that generates ALU operations
func test_harness() chan<alu_input> alu_in {
    var alu_input test_input
    var uint<2> op_count = 0
    var int<16> a_val = 10
    var int<16> b_val = 5
    
    while {
        // Create test input
        test_input.op.op = op_count
        test_input.a = a_val
        test_input.b = b_val
        
        alu_in.send(test_input)
        
        // Cycle through operations
        op_count = op_count + 1
        if op_count == 4 {
            op_count = 0
            a_val = a_val + 1
            b_val = b_val + 1
        }
    }
}

// Result checker that receives ALU outputs
func result_checker(chan<alu_output> alu_out) {
    var alu_output result
    
    while {
        await alu_out {
            result = alu_out.recv()
            if result.valid {
                // Result is valid, could log or verify here
            }
        }
    }
}
```

## Step 4: Compose the Complete System

Now let's wire everything together:

```weaver
// Complete ALU system
func alu_system() {
    var chan<alu_input> test_to_alu
    var chan<alu_output> alu_to_checker
    
    // Run all processes in parallel
    test_harness() -> test_to_alu and
    alu(test_to_alu) -> alu_to_checker and
    result_checker(alu_to_checker)
}
```

The `and` operator runs all three processes in parallel, with data flowing through the channels.

## Step 5: Build the System

Compile your complete circuit:

```bash
lm build alu.weaver
```

If there are errors, check:
- All types are properly defined
- Channel types match between processes
- All variables are declared

## Step 6: Visualize the Circuit

See the complete system structure:

```bash
lm show alu.weaver -o alu-system.dot
```

Then render it:

```bash
dot -Tpng alu-system.dot -o alu-system.png
```

## Step 7: Simulate (Optional)

Run a simulation to see the behavior:

```bash
lm sim alu.weaver
```

## Step 8: Understanding the Design

Let's break down what we built:

1. **Custom Types** (`opcode`, `alu_input`, `alu_output`) organize related data
2. **ALU Process** performs arithmetic operations based on opcode
3. **Test Harness** generates test inputs
4. **Result Checker** receives and validates outputs
5. **System Composition** connects everything with channels

## Advanced: Adding More Features

### Add a Register File

```weaver
type register_file {
    int<16> regs[8]
}

func register_file_unit(chan<alu_output> alu_out, chan<alu_input> feedback) {
    var register_file rf
    var alu_output result
    var alu_input next_op
    
    // Initialize registers
    var uint<3> i = 0
    while i < 8 {
        rf.regs[i] = 0
        i = i + 1
    }
    
    while {
        await alu_out {
            result = alu_out.recv()
            if result.valid {
                // Store result in register
                // Generate next operation
                feedback.send(next_op)
            }
        }
    }
}
```

### Add Parallel Operations

You can run multiple ALUs in parallel:

```weaver
func parallel_alu_system() {
    var chan<alu_input> input1, input2
    var chan<alu_output> output1, output2
    
    test_harness() -> input1 and
    test_harness() -> input2 and
    alu(input1) -> output1 and
    alu(input2) -> output2 and
    result_checker(output1) and
    result_checker(output2)
}
```

## Key Concepts Learned

- **Custom types** organize related data into buses
- **Complex systems** are built by composing processes
- **Parallel composition** (`and`) runs processes simultaneously
- **Type safety** ensures channels connect correctly
- **Complete circuits** combine behavior, structure, and types

## Design Patterns

### Pattern: Request-Response

```weaver
func server(chan<request> req, chan<response> resp) {
    var request r
    var response s
    
    while {
        await req {
            r = req.recv()
            // Process request
            s.result = process(r)
            resp.send(s)
        }
    }
}
```

### Pattern: Pipeline with Feedback

```weaver
func pipeline_with_feedback(chan<input> in, chan<output> out) {
    var chan<feedback> fb
    
    stage1(in) -> fb and
    stage2(fb) -> out and
    feedback_processor(out) -> fb
}
```

## What You've Accomplished

Congratulations! You've now:
- ✅ Written Weaver processes
- ✅ Used channels for communication
- ✅ Defined custom types
- ✅ Composed complete systems
- ✅ Built a functional hardware component

## Next Steps

Now that you've completed the tutorials:

1. **Explore the Reference** - Deep dive into language features
2. **Read How-to Guides** - Learn specific techniques
3. **Study Explanations** - Understand the "why" behind Weaver
4. **Build Your Own** - Start designing your hardware!

## Additional Resources

- [Reference Documentation](../reference/) - Complete language reference
- [How-to Guides](../how-to-guides/) - Task-specific guides
- [Explanations](../explanations/) - Conceptual background

Happy designing!
