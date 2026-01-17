---
title: Timing Assumptions
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

Weaver is designed to support multiple timing models, from Quasi-Delay Insensitive (QDI) to synchronous clocked logic. Understanding how Weaver handles timing helps you write code that can target different hardware styles.

## The Problem: Timing Models

Different hardware design styles make different timing assumptions:

- **Synchronous**: Everything happens on clock edges
- **Asynchronous**: No global clock, local timing only
- **QDI (Quasi-Delay Insensitive)**: Minimal timing assumptions
- **Bundled Data**: Data and control signals have timing relationships

Traditional hardware description languages typically commit to one timing model. Verilog assumes synchronous logic. SystemVerilog adds more models but still requires you to choose one upfront.

This creates a problem: code written for one timing model doesn't work for another. You can't easily retarget designs or experiment with different timing approaches.

## Weaver's Solution: Explicit Timing Assumptions

Weaver makes *no implicit timing assumptions* beyond the Isochronic Fork Assumption required by QDI design. Instead, it provides mechanisms to encode timing assumptions explicitly:

- **Validity**: Tracks when values are available
- **Channels**: Provide synchronization points
- **Composition operators**: Control when things happen relative to each other

The compiler then translates these explicit assumptions to the target timing model.

## The Isochronic Fork Assumption

Weaver assumes the Isochronic Fork Assumption (IFA), which is fundamental to QDI design:

> Signals that fork from the same point arrive at their destinations at approximately the same time.

This is a minimal assumption that's reasonable for most hardware. It means that if you have:

```weaver
var wire a
var wire b = a
var wire c = a
```

Then `b` and `c` will see the same value of `a` at approximately the same time. This is true in most hardware because wire delays are typically small compared to computation delays.

The IFA is the only timing assumption Weaver makes implicitly. Everything else is explicit.

## Validity and Timing

Validity is Weaver's primary mechanism for encoding timing assumptions. A value is valid when it's available for use, regardless of the underlying timing model:

```weaver
var int<32> a, b
a = compute_something()
b = a + 1  // Waits for a to be valid
```

In this code, `b` waits for `a` to be valid before computing. The compiler translates this to the appropriate timing mechanism for the target backend.

### QDI Backend

In QDI, validity maps to completion signals:

```weaver
a = compute_something()  // Generates completion signal when done
b = a + 1                 // Waits for a's completion signal
```

The compiler generates the necessary completion logic.

### Synchronous Backend

In synchronous designs, validity might map to clock cycles:

```weaver
a = compute_something()  // Valid on next clock edge
b = a + 1                 // Waits for next clock edge
```

The compiler inserts the necessary registers and clocking logic.

### Other Backends

The validity abstraction is flexible enough to support other timing models. The compiler handles the translation.

## Channels and Timing

Channels provide explicit synchronization points:

```weaver
sender.send(value)    // Synchronization point
receiver.recv()       // Synchronization point
```

These synchronization points are where timing matters. The compiler translates them to appropriate primitives for the target backend.

### QDI Backend

In QDI, channels map to handshake protocols:

```weaver
sender.send(value)    // Assert data + request, wait for acknowledge
receiver.recv()       // Wait for request, assert acknowledge
```

The compiler generates the handshake logic.

### Synchronous Backend

In synchronous designs, channels might map to ready/valid interfaces:

```weaver
sender.send(value)    // Assert valid + data, wait for ready
receiver.recv()       // Wait for valid, assert ready
```

The compiler generates the ready/valid logic.

## Composition and Timing

Composition operators control when things happen relative to each other:

### Sequential Composition

```weaver
a = 5
b = a + 1
```

Sequential composition says: `b` happens after `a`. The compiler ensures this ordering in the target timing model.

### Parallel Composition

```weaver
a = 5 and b = 2
```

Parallel composition says: `a` and `b` can happen simultaneously. The compiler ensures they don't interfere, using validity or other mechanisms.

### Conditional Composition

```weaver
await A {
    // ...
} or await B {
    // ...
}
```

Conditional composition says: exactly one path executes. The compiler ensures mutual exclusion in the target timing model.

## Compiling to Different Backends

The compiler translates Weaver code to different timing models:

### QDI Compilation

For QDI backends, the compiler:
- Generates completion signals for validity
- Implements handshake protocols for channels
- Ensures IFA compliance
- Generates delay-insensitive logic

### Synchronous Compilation

For synchronous backends, the compiler:
- Maps validity to clock cycles
- Implements ready/valid interfaces for channels
- Inserts registers for state
- Generates clocked logic

### Other Backends

The compiler can target other timing models by translating validity, channels, and composition to appropriate primitives.

## Writing Timing-Agnostic Code

To write code that works across timing models:

### Use Validity Explicitly

```weaver
var int<32> a, b
a = compute()
await a {  // Explicit: wait for a to be valid
    b = a + 1
}
```

### Use Channels for Synchronization

```weaver
func process(chan<int> in) chan<int> out {
    while {
        await in {  // Explicit synchronization point
            out.send(process(in.recv()))
        }
    }
}
```

### Avoid Implicit Timing Assumptions

Don't assume:
- Things happen on clock edges (unless targeting synchronous)
- Things happen instantly (unless targeting combinational)
- Things happen in a specific order (unless using sequential composition)

Instead, use validity and channels to make timing explicit.

## Design Trade-offs

Supporting multiple timing models has trade-offs:

### Advantages

- **Flexibility**: Code can target different hardware styles
- **Experimentation**: Easy to try different timing approaches
- **Portability**: Designs can be retargeted
- **Explicit assumptions**: Timing assumptions are clear in the code

### Disadvantages

- **Compiler complexity**: Compiler must handle multiple backends
- **Abstraction overhead**: Some efficiency may be lost to abstraction
- **Learning curve**: Developers must understand validity and timing

Overall, the advantages outweigh the disadvantages, especially for research, experimentation, and designs that might target multiple hardware styles.

## When to Commit to a Timing Model

While Weaver supports multiple timing models, sometimes you need to commit to one:

### Performance-Critical Designs

If performance is critical, you might want to commit to a specific timing model and optimize for it. The compiler can provide hints or directives for this.

### Legacy Integration

If integrating with existing hardware that uses a specific timing model, you might need to commit to that model.

### Tool Limitations

If your target tools only support one timing model, you're effectively committed to that model.

But even in these cases, starting with timing-agnostic code and then specializing can be beneficial.

## Summary

Weaver supports multiple timing models by making timing assumptions explicit through validity, channels, and composition operators. The compiler translates these explicit assumptions to the target timing model.

This approach provides flexibility and portability while maintaining the ability to target specific hardware styles. Understanding how timing works in Weaver helps you write code that's both correct and portable.
