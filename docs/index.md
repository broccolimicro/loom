---
title: Weaver Hardware Description Language
author: Edward Bingham
date: 2026-01-15
layout: post
permalink: /
---

```weaver
func fetch(chan<void> Inc; chan<uint<32> > Jmp) chan<uint<32> > Addr {
	var uint<32> pc = 0
	while {
		Addr.send(pc)
		await Jmp {
			pc = Jmp.recv()
		} or await Inc {
			Inc.recv()
			pc = pc + 1
		}
	}
}
```

Weaver is a hardware description language whose mission is to help software
engineers design complex hardware systems without the need for deep hardware
expertise.

## New Here?

1. Start with a [Tutorial](./tutorial/index) to get hands-on experience
2. Use [How-to Guides](./guide/index) for specific tasks
3. Refer to the [Reference](./reference/index) for syntax and details
4. Read [Explanations](./explain/index) to deepen your understanding

### [Tutorials](./tutorial/index)
Welcome to the Weaver tutorials! These step-by-step guides will teach you how
to use Weaver to design hardware. Tutorials are designed to be completed in
order. Each tutorial builds on concepts from previous ones.

1. [**Getting Started with Weaver**](./tutorial/01-getting-started) - Learn the basics: write your first Weaver program, understand the toolchain, and see your code in action.
2. [**Building Your First Process**](./tutorial/02-first-process) - Create a simple process that demonstrates basic Weaver concepts: variables, loops, and channel communication.
3. [**Working with Channels**](./tutorial/03-channels) - Learn how processes communicate using channels, including sending, receiving, and waiting for data.
4. [**Building a Complete Circuit**](./tutorial/04-complete-circuit) - Put it all together: create a functional circuit using processes, channels, and custom types.

### [How-to Guides](./guide/index)
These guides show you how to accomplish specific tasks in Weaver. Each guide
provides step-by-step instructions to solve a particular problem. Each how-to
guide follows this structure:

1. **Goal**: What you'll accomplish
2. **Prerequisites**: What you need to know or have
3. **Steps**: Clear, numbered steps to achieve the goal
4. **Verification**: How to verify it worked
5. **Next Steps**: Related guides or concepts

Guides focus on practical steps. For deeper understanding of concepts, they
link to explanations or reference material.

**Debugging and Verification**
- [How to Debug Processes](./how-to-debug-processes) - Identify and fix issues in your processes
- [How to Verify Validity Behavior](./how-to-verify-validity) - Check that validity is working correctly
- [How to Trace Channel Communication](./how-to-trace-channel-communication) - Understand data flow through channels

**Design Patterns**
- [How to Implement State Machines](./how-to-implement-state-machines) - Create state machines using Weaver processes
- [How to Build Arbiters](./how-to-build-arbiters) - Implement arbitration logic for multiple requesters
- [How to Create Pipelines](./how-to-create-pipelines) - Build multi-stage processing pipelines
- [How to Handle Backpressure](./how-to-handle-backpressure) - Manage flow control in your designs

**Optimization**
- [How to Optimize Pipeline Depth](./how-to-optimize-pipeline-depth) - Balance latency and throughput
- [How to Reduce Resource Usage](./how-to-reduce-resource-usage) - Minimize area and power consumption

**Error Handling**
- [How to Handle Errors in Processes](./how-to-handle-errors) - Implement error detection and recovery
- [How to Reset Processes](./how-to-reset-processes) - Properly initialize and reset your processes

**Testing and Simulation**
- [How to Test Processes](./how-to-test-processes) - Verify your processes work correctly
- [How to Simulate Circuits](./how-to-simulate-circuits) - Run and debug your designs

**Integration**
- [How to Connect to External Interfaces](./how-to-connect-to-external-interfaces) - Interface with external hardware
- [How to Integrate with Verilog](./how-to-integrate-with-verilog) - Use Weaver with existing Verilog code

### [Reference](./reference/index)
This is the complete technical reference for the Weaver hardware description
language. Use this when you need to look up syntax, operators, types, or
language features.

1. [**Types**](./reference/01-types) - Built-in types and type system
2. [**Operators**](./reference/02-operators) - Operators and precedence
3. [**Functions**](./reference/03-functions) - Function definitions and behavior
4. [**Processes**](./reference/04-processes) - Process definitions and pipeline stages
5. [**Structures**](./reference/05-structures) - Structural circuit descriptions
6. [**Custom Types**](./reference/06-custom-types) - User-defined types
7. [**Syntax**](./reference/07-syntax) - Language syntax and grammar
8. [**Validity and Truthiness**](./reference/08-validity-truthiness) - Validity semantics and truthiness

### [Design Philosophy](./explain/index)
This section provides conceptual background, design rationale, and deeper
understanding of Weaver's architecture and design decisions. These explanations
help you understand *why* things are the way they are.

1. [**Design Philosophy**](./explain/index) - The core principles that guide Weaver's design
2. [**Validity and Truthiness**](./explain/02-validity-truthiness) - Why validity exists and how it enables flexible timing
3. [**Composition Model**](./explain/03-composition-model) - The rationale behind parallel, sequential, and choice composition operators
4. [**Process Architecture**](./explain/04-process-architecture) - Why processes have perpetual loops and how they map to hardware
5. [**Channel Communication**](./explain/05-channel-communication) - Design decisions behind Weaver's communication model
6. [**Timing Assumptions**](./explain/06-timing-assumptions) - How Weaver handles different timing models from QDI to synchronous

## Version 0.15

This documentation covers the [lastest version of Loom](https://github.com/broccolimicro/loom), **v0.15**.
