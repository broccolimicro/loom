---
title: Weaver
author: Edward Bingham
date: 2026-01-15
layout: post
permalink: /
---

# Weaver Documentation

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

### [Tutorials](./tutorials/)
Step-by-step learning guides. Start here if you're new to Weaver.

### [How-to Guides](./how-to-guides/)
Task-oriented guides for common operations. Use these when you know what you want to do but need to know how.

### [Reference](./reference/)
Complete technical reference for the Weaver language. Use this when you need to look up syntax, operators, types, or language features.

### [Explanations](./explanations/)
Conceptual background and deeper understanding. Read these to understand the "why" behind Weaver's design.

## Language Version

This documentation covers **Weaver v1.0.0** for Loom.
