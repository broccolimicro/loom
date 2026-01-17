---
title: Reference
category: Reference
author: Edward Bingham
date: 2026-01-14
layout: post
---

This is the complete technical reference for the Weaver hardware description language v1.0.0.

Weaver is a hardware description language whose mission is to make it easy for software engineers to design high-quality complex computer architectures. It balances three core building-blocks: **Behavior**, **Structure**, and **Types**.

- [**Types**](./01-types) - Built-in types and type system
- [**Operators**](./02-operators) - Operators and precedence
- [**Functions**](./03-functions) - Function definitions and behavior
- [**Processes**](./04-processes) - Process definitions and pipeline stages
- [**Structures**](./05-structures) - Structural circuit descriptions
- [**Custom Types**](./06-custom-types) - User-defined types
- [**Syntax**](./07-syntax) - Language syntax and grammar
- [**Validity and Truthiness**](./08-validity-truthiness) - Validity semantics and truthiness

Weaver allows you to describe circuits through three complementary approaches:

1. **Behavior** - Describe what the circuit does (processes and functions)
2. **Structure** - Describe how the circuit is organized (structural descriptions)
3. **Types** - Describe how data is grouped (type definitions)

All three can be used together in a single design.
