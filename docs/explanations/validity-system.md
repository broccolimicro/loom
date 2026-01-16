---
title: Validity and Truthiness System
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Validity and Truthiness System

Weaver's validity system is one of its most distinctive features. Understanding why it exists and how it works is crucial for writing effective Weaver code.

## The Problem: Timing Assumptions

Hardware description languages typically make strong timing assumptions. Verilog assumes synchronous, clocked logic. SystemVerilog adds more timing models but still requires you to commit to one. This makes it hard to write code that can target different hardware styles.

Weaver takes a different approach: it makes *no implicit timing assumptions* beyond the Isochronic Fork Assumption required by Quasi-Delay Insensitive (QDI) design. This allows Weaver code to compile to different backends with different timing models.

But this creates a problem: how do you express "this value isn't ready yet" or "this computation hasn't finished" without assuming a specific timing model?

## The Solution: Validity

Validity is Weaver's answer. Every variable implicitly tracks whether it contains a meaningful value:

- **Valid**: The variable has a value you can use
- **Null/Invalid**: The variable doesn't have a value yet (or any longer)

This is separate from the value itself. A boolean can be:
- `true` (valid and true)
- `false` (valid and false)  
- `null` (invalid)

This three-state model is fundamental to how Weaver works.

## Why Separate Validity and Truthiness?

You might wonder: why not just use `null` or `undefined` like some languages do? Why have both validity and truthiness?

The answer is that they serve different purposes:

### Validity: Timing and Presence

Validity answers: "Is this value available?" This is about timing and data flow. When you `await` a channel, you're waiting for validity—you want to know when data arrives, not what the data is.

### Truthiness: Logic and Conditions

Truthiness answers: "Is this condition true?" This is about logic and control flow. When you use `if`, you're checking truthiness—you want to know if a condition is satisfied.

### Why They're Different

Consider this scenario:

```weaver
var bool error_flag
// ... later ...
if error_flag {
    // Handle error
}
```

If `error_flag` is `false`, that's a valid value meaning "no error." You don't want to handle an error. But if `error_flag` is `null`, that might mean "we haven't checked for errors yet"—you also don't want to handle an error, but for a different reason.

Separating validity and truthiness lets you distinguish:
- "The value is false" (valid, false, don't act)
- "The value isn't available" (null, don't act, but for timing reasons)

This distinction becomes crucial in parallel compositions where timing matters.

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
