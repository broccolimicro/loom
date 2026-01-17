---
title: Loom
author: Edward Bingham
date: 2026-01-15
layout: post
permalink: /
---

# Loom Documentation

Welcome to the Weaver hardware description language documentation.

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

Weaver is a hardware description language whose mission is to make it easy for
software engineers to design high-quality complex computer architectures. It
balances three core building-blocks: **Behavior**, **Structure**, and
**Types**.

## New Here?

1. Start with a [Tutorial](./tutorial/index) to get hands-on experience
2. Use [How-to Guides](./guide/index) for specific tasks
3. Refer to the [Reference](./reference/index) for syntax and details
4. Read [Explanations](./explain/index) to deepen your understanding

### [Tutorials](./tutorial/index)
Step-by-step learning guides. Start here if you're new to Weaver.

### [How-to Guides](./guide/index)
Task-oriented guides for common operations. Use these when you know what you want to do but need to know how.

### [Reference](./reference/index)
Complete technical reference for the Weaver language. Use this when you need to look up syntax, operators, types, or language features.

### [Explanations](./explain/index)
Conceptual background and deeper understanding. Read these to understand the "why" behind Weaver's design.

## Version 0.15

This documentation covers the [lastest version of Loom](https://github.com/broccolimicro/loom), **v0.15**.
