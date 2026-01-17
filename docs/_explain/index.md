---
title: Design Philosophy
category: Explanations
author: Edward Bingham
date: 2026-01-14
layout: post
---

**Weaver’s mission is to help software engineers design complex hardware systems
without the need for deep hardware expertise.**

## Hardware Is Hard

Software is sequential. Each statement executes one after another. Even with
threads or async/await, there is an underlying sequential execution model.
People naturally reason this way.

Hardware is different. Many operations occur at the same time: a processor may
fetch one instruction, decode another, execute a third, and write back a
fourth, all simultaneously. Parallelism is not optional; it is the nature of
hardware.

Traditional hardware description languages embrace implicit parallelism:
everything happens at once unless you explicitly sequence it using state
machines or control logic.

This creates a cognitive mismatch. People reason sequentially, but are forced to
design in a fully parallel model. As a result, designers must maintain two
descriptions of the same system:
- a behavioral model that people can understand, and
- a structural implementation that the compiler can synthesize.

Bridging these two descriptions requires extensive testing and verification,
consuming much of the effort in modern chip design.

Alternatively, High-Level Synthesis tries to compile software programs to
hardware, but it is very easy to write a program that is not synthesizeable.

Software is virtual: functions are loaded and unloaded, memory is allocated and
deallocated, network connections are created and destroyed, nothing occupies
physical space. Hardware is physical: functions, memory, and connections are
all implemented by real, physical, concrete circuit elements.

Virtualization hides sequencing and dependency through memory, and how
everything is organized in memory over time is *dependant on the inputs of the
program*. By definition, the compiler does not have enough information to
untangle virtualization at compile time.

## Introducing Weaver

**Weaver is fundamentally sequential and physical.**

It provides a sequential execution model that people can reason about while
mapping directly to physical hardware.

![test]({{site.baseurl}}/images/loom.svg)

Weaver looks and feels like a modern software language, abstracting the
hardware-level semantics into a more familiar execution model. This drives
several key design choices:

- **Behavior First** - Describe what the system should do, not how signals move
  between registers each cycle. The compiler handles the low-level details.

- **High Level Abstractions** - Stop worrying about encodings, protocols, or
	timing. The model and implementation are unified, and verification is built
  in.

- **Composable Design** - Components communicate through explicit channels with
  built-in flow control and validity. Parallel composition explicit and
  predictable, avoiding the hidden dependencies common in shared-memory models
  and the subtle timing bugs common in register-transfer logic.

## Hardware Semantics

Weaver describes hardware, not software.

- **Physical, not Virtual** - Functions, memory, and connections exist as real,
  concrete hardware elements. Resources are static, functions are inlined, and
  processes run as long as the chip is powered.

- **Fine-Grained Parallelism** - Hardware executes many operations
  simultaneously.

This has many downstream consequences. 

- [**Validity, not Locks**](./02-validity-truthiness) - Weaver uses a lighter
  mechanism to manage distributed access shared resources.

- [**Native Parallel Composition**](./03-composition) - Weaver provides native
  operators and semantics to interleave sequential, parallel, and conditional
  execution safely.

- [**Processes, not Functions**](./04-processes) - In Weaver, computation is
  expressed as long-lived processes rather than ephemeral software functions.
  Processes maintain state across time, communicate explicitly, and execute
  continuously while the chip is powered.

- [**Message Passing, not Shared Memory**](./04-message-passing) - Hardware
  naturally uses point-to-point communication. Shared memory is possible, but
  must be explicitly designed.

- [**Logic Families, not Instruction Set Architectures**](./06-timing) - Weaver
  organizes computation around hardware-friendly logic primitives and timing
  models, rather than abstract instruction sets. This makes timing, concurrency,
  and resource usage explicit.

## Balance of Power and Simplicity

Because Weaver describes hardware, it is built up from semantics that are very
different from software. As a result, there are many powerful low-level
semantics in the language. However, those semantics are often both complex and
complicated in extremely subtle ways.

Weaver tries to balance expressive power with simplicity through abstraction.
You may pick up Weaver and make it look like any other programming language.
Or, with a healthy dose of psychodelics and masochism, you too can join us down
the rabit hole of event semantics, timing models, handshake protocols,
non-determinism, and the beautifully woven hyperdimensional fractal that is a
program.

Until then, the language is powerful enough to describe complex architectures,
but simple enough that software engineers can learn it without deep hardware
expertise.

## What This Means for You

When designing with Weaver, keep these principles in mind:

1. **Think in hardware** - Your code describes physical circuits, not running programs.
2. **Design top-down, Build bottom-up** - Understand the bigger picture,
  but build and verify the smallest and simplest process first.
3. **Minimize data movement** - Prefer point-to-point communication over
  broadcast, feed-forward algorithms over iterative, distributed architectures
  over centralized, and small data packets over big.
4. **Minimize energy** - Avoid unnecessary work and take advantage of
  sparse or irregular data.

Understanding these principles helps you write Weaver code that's not just
correct, but also clear, maintainable, and well-architected.

