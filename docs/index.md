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

### [Reference](./reference/index)
This is the complete technical reference for the Weaver hardware description
language. Use this when you need to look up syntax, operators, types, or
language features.

1. [**Types**](./reference/01-types) - Describe how data is grouped (type definitions)
2. [**Behavior**](./reference/02-functions) - Describe what the circuit does (processes and functions)
3. [**Structure**](./reference/03-structures) - Describe how the circuit is organized (structural descriptions)

### [Design Philosophy](./explain/index)
This section provides conceptual background, design rationale, and deeper
understanding of Weaver's architecture and design decisions. These explanations
help you understand *why* things are the way they are.

1. [**Design Philosophy**](./explain/index) - The core principles that guide Weaver's design
2. [**Validity and Truthiness**](./explain/02-validity-truthiness) - Why validity exists and how it enables flexible timing
3. [**Composition Model**](./explain/03-composition-model) - The rationale behind parallel, sequential, and choice composition operators
4. [**Processes and Message Passing**](./explain/04-process-architecture) - Why processes have perpetual loops and how they map to hardware

## Version 0.15

This documentation covers the [lastest version of Loom](https://github.com/broccolimicro/loom), **v0.15**.
