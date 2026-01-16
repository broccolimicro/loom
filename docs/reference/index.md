---
title: Weaver Language Reference
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Weaver Language Reference

This is the complete technical reference for the Weaver hardware description language v1.0.0.

Weaver is a hardware description language whose mission is to make it easy for software engineers to design high-quality complex computer architectures. It balances three core building-blocks: **Behavior**, **Structure**, and **Types**.

## Quick Navigation

- [Types](./types) - Built-in types and type system
- [Operators](./operators) - Operators and precedence
- [Functions](./functions) - Function definitions and behavior
- [Processes](./processes) - Process definitions and pipeline stages
- [Structures](./structures) - Structural circuit descriptions
- [Custom Types](./custom-types) - User-defined types
- [Syntax](./syntax) - Language syntax and grammar
- [Validity and Truthiness](./validity-truthiness) - Validity semantics and truthiness

## Language Overview

Weaver allows you to describe circuits through three complementary approaches:

1. **Behavior** - Describe what the circuit does (processes and functions)
2. **Structure** - Describe how the circuit is organized (structural descriptions)
3. **Types** - Describe how data is grouped (type definitions)

All three can be used together in a single design.
