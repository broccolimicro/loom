---
title: Validity and Truthiness
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Validity and Truthiness

Most Hardware Description Languages use Register-Transfer Logic (Verilog,
SystemVerilog, Chisel, VHDL, etc). This approach assumes that systems are
clocked, generating many timing constraints that must be strictly enforced in
layout. As a result, clock-domain crossings and IO boundaries become painful to
navigate.

Meanwhile modern chips often have different clock frequencies per core, and the
asynchronous network that ties together these domains feels like black magic.
Interfacing with analog systems requires spice-level black-box models, and
non-standard logic families feel almost unobtainable.

**Weaver** takes a different approach. Weaver starts with the *most minimal
timing assumption* needed to be Turing Complete. Timing assumptions can then be
layered on as needed. This allows Weaver code to compile to different backends
with different timing models.

In clocked systems, the clock guarantees that data on a bus is valid. Explicit
flow-control structures like val-rdy must be added to handle no-ops or
backpressure in the pipeline.

However, Weaver does not have a clock. So, how do you express "this value isn't
ready yet" or "this computation hasn't finished" without assuming a specific
timing model?

## The Solution: Validity

Every variable implicitly tracks whether it contains a meaningful value.
- **Valid**: The variable has a value you can use
- **Null**: The variable does not have a value yet (or any longer)

For the software engineer, this is most similar to pointers. A pointer may be
`nullptr`, or it may point to an address in memory that stores a meaningful
value.

For the hardware engineer, this is like having the valid signal in a val-rdy or
AXI protocol alongside the data bus. The value on the bus is meaningful when
that valid signal is high.

In Weaver, this applies to **every** data type. For example, a `bool` variable
may take one of three states:
- `true` (valid and true)
- `false` (valid and false)  
- `null` (not valid)

Even a `wire` may be thought of as being valid or null:
- `vdd` (valid)
- `gnd` (not valid)

## Why Separate Validity and Truthiness?

Why not just use `null` or `undefined` like most other software languages do?
Why explicitly separate both validity and truthiness?

Most other software languages are fundamentally sequential. This means that
most actions are implicitly ordered. Parallelism comes in extremely coarse
grained abstractions like threads. When multiple threads need to communicate,
those software languages reach for heavy abstractions like locks, shared
memory, and a complex coherent shared cache heirarchy. As a result, most
software languages **do not use validity as a way to order access to shared
resources**.

Weaver is a hardware description language. This means that most actions are
fundamentally parallel, and parallelism is extremely fine-grained. Validity is
how Weaver manages this fine-grained parallel access to shared buses, and dince
that parallelism is extremely fine-grained, so must be validity.

Truthiness serves a very different purpose.

- **Validity: Timing and Presence** "Is this value available?" This is about
  timing and data flow. When you `await` a channel, you're waiting for
  validity—you want to know when data arrives, not what the data is.
- **Truthiness: Logic and Conditions** "Is this condition true?" This is about
  logic and control flow. When you use `if`, you're checking truthiness—you
  want to know if a condition is satisfied.

Consider this scenario:

```weaver
var bool error_flag
// ... later ...
if error_flag {
    // Handle error
}
```

If `error_flag` is `false`, that's a valid value meaning the task has completed
with no error. But if `error_flag` is `null`, that means the task has not yet
completed, it may still produce an error. Separating validity and truthiness
lets you distinguish between those two cases without knowing ahead of time how
long that task will take.

## How Validity Propagates

Validity affects every operation. The rule is simple: if any input is null, the output is null (represented as `gnd`):

```weaver
gnd + 5 = gnd        // null + valid = null
5 + 5 = 10           // valid + valid = valid
gnd && true = gnd    // null && valid = null
true && false = false // valid && valid = valid result
```

This propagation ensures that invalid data doesn't contaminate your computation. If you try to add a value that isn't ready yet, the result is also not ready.

## Validity and Operators

Different operators handle validity differently:

### Arithmetic Operators

Arithmetic operators propagate null: if any operand is null, the result is null. This makes sense—you can't add two numbers if one doesn't exist yet.

### Boolean Operators

Boolean operators (`&&`, `||`, `^^`) also propagate null, but they distinguish between:
- `null && true = null` (can't evaluate if left side is null)
- `true && false = false` (both valid, result is valid)

### Wire-Level Operators

Wire-level operators (`&`, `|`, `^`) work at the validity level. They combine validity signals:
- `gnd & vdd = gnd` (null & valid = null)
- `vdd & vdd = vdd` (valid & valid = valid)

This lets you build validity logic explicitly when needed.

## Validity and Control Flow

Validity is fundamental to Weaver's control flow:

### `await`: Waiting for Validity

`await` blocks until a value becomes valid. This is how you synchronize with data that arrives asynchronously:

```weaver
await channel {
    value = channel.recv()
}
```

You're not waiting for a specific value—you're waiting for *any* valid value.

### `if`: Checking Truthiness

`if` blocks until a condition becomes true. This checks both validity and truthiness:

```weaver
if x > 5 {
    // Only executes when x is valid AND x > 5
}
```

The `if` statement implicitly calls `true()` on the condition, which requires both validity and truthiness.

## Validity and Parallel Composition

Validity is crucial for parallel composition. Consider this example:

```weaver
var int<32> a, b, c
a-, b-, c-
(
    a = 5
    await b
    c = a + b
) and (
    await a
    b = 3
)
```

The validity system ensures that:
1. The first process sets `a = 5`, making `a` valid
2. The second process sees `a` is valid and proceeds to set `b = 3`
3. The first process sees `b` is valid and proceeds to compute `c = a + b`

Without validity, you'd need mutexes or other synchronization primitives. Validity provides a clean, hardware-native way to coordinate parallel processes.

## Compiling to Different Backends

The validity system enables Weaver to compile to different timing models:

### Quasi-Delay Insensitive (QDI)

In QDI, validity maps directly to completion signals. A value is valid when its completion signal is asserted. The compiler generates the necessary completion logic.

### Synchronous (Verilog)

In synchronous designs, validity might map to clock cycles. A value becomes valid on a specific clock edge. The compiler inserts the necessary registers and clocking logic.

### Other Models

The validity system is abstract enough to support other timing models. The compiler translates validity semantics to the target model's primitives.

## When to Use `valid()` and `true()`

Most of the time, you don't need to explicitly call `valid()` or `true()`. The language handles validity and truthiness automatically. But sometimes you need to force one interpretation:

### `valid(x)`: Force Validity Check

Use `valid()` when you need to check validity explicitly, perhaps in a complex condition:

```weaver
if valid(x) && valid(y) {
    // Both are valid, proceed
}
```

### `true(x)`: Force Truthiness Check

Use `true()` when you need to check truthiness explicitly, perhaps to distinguish false from null:

```weaver
if true(x) {
    // x is valid and truthy
} or if valid(x) && !true(x) {
    // x is valid but false
}
```

## Design Trade-offs

The validity system has trade-offs:

### Advantages

- **Flexible timing**: Code can target different timing models
- **Explicit synchronization**: Validity makes data flow clear
- **Hardware-native**: Maps naturally to completion signals and handshakes
- **Composable**: Works cleanly with parallel composition

### Disadvantages

- **Learning curve**: Software engineers need to learn a new concept
- **Verbosity**: Sometimes you need explicit validity checks
- **Compiler complexity**: The compiler must translate validity to target primitives

Overall, the advantages outweigh the disadvantages, especially for complex systems where timing flexibility and explicit synchronization are crucial.

## Summary

Validity is Weaver's solution to the timing assumption problem. By tracking whether values are available separately from their actual values, Weaver can:

- Support multiple timing models
- Enable clean parallel composition
- Make data flow explicit
- Compile to different hardware targets

Understanding validity is key to writing effective Weaver code. It's not just a feature—it's fundamental to how Weaver works.
