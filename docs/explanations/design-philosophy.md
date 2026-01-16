---
title: Design Philosophy
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Design Philosophy

Weaver is built on a foundation of principles that guide every design decision. Understanding these principles helps you write better Weaver code and understand why the language works the way it does.

## Mission

Weaver's mission is to make it easy for software engineers to design high-quality complex computer architectures. This mission drives several key design choices:

### Software Engineer Friendly

Weaver prioritizes familiarity for software engineers. The syntax looks similar to common programming languages, but the semantics are fundamentally different because they map to hardware, not software execution.

### High-Quality Architectures

Weaver is designed to help you build correct, efficient hardware. The language enforces patterns that lead to better designs:

- **Explicit communication**: Channels make data flow explicit and verifiable
- **Compositional design**: Processes compose cleanly, enabling modular architectures
- **Type safety**: The type system helps catch errors at compile time

### Complex Systems

Weaver doesn't just handle simple circuits—it's designed for complex architectures. The composition operators, channel system, and process model all support building large, interconnected systems.

## Three Core Building Blocks

Weaver balances three fundamental aspects of hardware design:

### Behavior

Behavior describes *what* the circuit does. Processes and functions let you describe computation and state transitions in a familiar, imperative style. This is where most of your logic lives.

### Structure

Structure describes *how* components are connected. Structures let you organize your design hierarchically and describe the physical layout of your circuit. This is where you compose processes into larger systems.

### Types

Types describe *how data is organized*. Custom types let you group related data into buses, making your code more readable and your designs more maintainable.

These three aspects are intentionally separate. You can think about behavior without worrying about structure, and vice versa. This separation of concerns makes complex designs more manageable.

## Hardware, Not Software

Perhaps the most important principle to understand is that Weaver describes hardware, not software. This has profound implications:

### No Sequential Execution Model

In software, statements execute one after another. In Weaver, statements can execute in parallel, and the language provides explicit operators to control this. The default sequential composition (`;` or newline) is a convenience, but parallel composition (`and`) is equally fundamental.

### Perpetual Processes

Processes never terminate because hardware doesn't stop. A CPU fetch unit runs forever, continuously fetching instructions. This is why processes have perpetual loops—they model hardware that's always active.

### Timing is Explicit

Unlike software languages that assume a single execution model, Weaver makes timing assumptions explicit. The validity system lets you write code that can compile to different timing models (QDI, synchronous, etc.) by encoding the necessary assumptions explicitly.

## Composition Over Inheritance

Weaver favors composition over other forms of code reuse. Processes compose through channels, structures compose through instantiation, and types compose through aggregation. This makes designs modular and testable.

## Explicit Over Implicit

Weaver makes important concepts explicit rather than hiding them:

- **Validity**: Instead of assuming all values are always valid, Weaver tracks validity explicitly
- **Timing**: Instead of assuming a single timing model, Weaver lets you encode timing assumptions explicitly
- **Communication**: Instead of shared memory, Weaver uses explicit channels
- **Composition**: Instead of implicit execution order, Weaver provides explicit composition operators

This explicitness makes the language more verbose in some ways, but it also makes designs more understandable, verifiable, and flexible.

## Balance of Power and Simplicity

Weaver tries to balance expressive power with simplicity. The language is powerful enough to describe complex architectures, but simple enough that software engineers can learn it without deep hardware expertise.

This balance is evident in choices like:
- Familiar syntax with hardware semantics
- Small set of built-in types with powerful composition
- Simple channel model that enables complex communication patterns
- Explicit operators that make parallel execution clear

## What This Means for You

When writing Weaver code, keep these principles in mind:

1. **Think in hardware**: Your code describes circuits, not programs
2. **Compose explicitly**: Use composition operators to make your design clear
3. **Make timing explicit**: Use validity to encode your timing assumptions
4. **Separate concerns**: Use behavior for logic, structure for organization, types for data
5. **Design for composition**: Write processes that can be easily connected and reused

Understanding these principles helps you write Weaver code that's not just correct, but also clear, maintainable, and well-architected.
