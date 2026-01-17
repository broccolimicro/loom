---
title: Design Philosophy
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

**Weaver’s goal is to help software engineers design complex hardware systems
without the need for deep hardware expertise.**

## Design Choices

To do this, Weaver looks and feels like a modern software language, abstracting
the hardware-level semantics into a more familiar execution model. This drives
several key design choices:

- **Behavior First** - You describe what the system should do, not how signals
  move between registers on every cycle. Instead of manually building state
  machines, you write the intended behavior and let the compiler handle the
  low-level details.
- **High Level Abstractions** - Stop worrying about encodings, protocols, or
  timing. The model and implementation are unified, and verification is
  built-in. Focus on writing your intended behavior.
- **Composable Design** - Communication between processes use channels with
	built-in flow control. Standard communication rules allow components to
  connect cleanly, without subtle timing bugs.

## Core Concepts

Weaver balances three fundamental aspects of hardware design:

- **Behavior** describes *what* the circuit does. Processes and functions let you
  describe computation and state transitions in a familiar, imperative style.
  This is where most of your logic lives.
- **Structure** describes *how* components are connected. Structures let you organize
  your design hierarchically and describe the physical layout of your circuit.
  This is where you compose processes into larger systems.
- **Types** describe *how data is organized*. Custom types let you group related data
  into buses, making your code more readable and your designs more maintainable.

These three aspects are intentionally separate. You can think about behavior
without worrying about structure, and vice versa. This separation of concerns
makes complex designs more manageable.

## Hardware Semantics, Not Software

Perhaps the most important principle to understand is that Weaver describes
hardware, not software. This has profound implications:

- **Extremely Fine-Grained Parallelism** - Software usually runs one step at a
	time, and parallelism requires heavy tools like threads and locks. Hardware
	runs many operations at once by default, and sequencing requires heavy tools,
	like a clock and event semantics. Weaver provides native operators and deeply
  integrated semantics to help interleave sequential, parallel, and conditional
  execution.
- **No Virtualization** - In software, functions are loaded and unloaded,
  memory is allocated and deallocated, network connections are created and
  destroyed, nothing occupies physical space. In hardware, virtualization is
  expensive and everything occupies physical space. Functions, memory, and
	connections are all implemented by real, physical, concrete circuit elements.
  As a result, functions are inlined, memory is static, and processes run as long
  as the chip is powered. Any form of virtualization must be explicitly built on
  top of static resources.
- **Message Passing, not Shared Memory** - Hardware naturally uses
  point-to-point communication. Small pieces of data flow through pipelines
  rather than large shared data structures. Shared memory is possible, but must
  be explicitly designed.
- **Validity instead of Locks** - Software uses locks to control access to
  shared resources. In hardware, even a single wire can be shared. Weaver uses
  a lighter mechanism called Validity to control when data is present and usable.

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

