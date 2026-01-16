---
title: Explanations
category: Explanations
author: Edward Bingham
date: 2026-01-14
layout: post
---

# Explanations

This section provides conceptual background, design rationale, and deeper understanding of Weaver's architecture and design decisions. Unlike tutorials (which teach you how to do things) or reference guides (which tell you what exists), explanations help you understand *why* things are the way they are.

## What You'll Find Here

- **Design Philosophy**: The core principles that guide Weaver's design
- **Validity and Truthiness**: Why these concepts are separate and how they enable flexible timing models
- **Composition Model**: The rationale behind parallel, sequential, and choice composition operators
- **Process Architecture**: Why processes have perpetual loops and how they map to hardware
- **Channel Communication**: The design decisions behind Weaver's communication model
- **Timing Assumptions**: How Weaver handles different timing models from QDI to synchronous

## When to Read Explanations

Explanations are most useful when you:
- Want to understand the reasoning behind a design decision
- Need context for why something works a certain way
- Are designing complex systems and need to understand trade-offs
- Want to understand the theoretical foundations

If you're just getting started, begin with the [Tutorials](../tutorials/). If you need to look up syntax or API details, see the [Reference](../reference/).

## Topics

- [Design Philosophy](./design-philosophy) - Core principles and goals of Weaver
- [Validity and Truthiness](./validity-system) - Why validity exists and how it enables flexible timing
- [Composition Model](./composition-model) - The rationale behind parallel, sequential, and choice operators
- [Process Architecture](./process-architecture) - Why processes work the way they do
- [Channel Communication](./channel-communication) - Design decisions behind Weaver's communication model
- [Timing Assumptions](./timing-assumptions) - From QDI to synchronous: how Weaver handles timing
