---
title: Syntax - Weaver Language Reference
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Syntax

This page covers the core syntax elements of the Weaver language.

## Conditions and Loops

### `await` - Validity-Based Waiting

`await` blocks program execution until the condition expression is valid. For a condition expression `cond`, `valid(cond)` must evaluate to `vdd`.

```weaver
var fixed<16,-4> x
x = 3
await x {
    // Executes when x becomes valid
}
```

Can be used to wait for valid values on channels:

```weaver
var chan<bool> A
var chan<int<32>> B
await A & B {
    A.recv()
    B.recv()
}
```

Think of `await` as waiting for an event to occur - a transition on a wire from `gnd` to `vdd`.

### `if` - Truthiness-Based Conditional

`if` blocks program execution until the condition expression is true. For a condition expression `cond`, `true(cond)` must evaluate to `vdd`.

```weaver
var fixed<16,-4> x
x = 3
if x == 3 {
    // Executes when x == 3 is true
}
```

**Important**: The `if` statement in Weaver is **blocking** and there is **no `else`**. This is unlike if statements in any other language.

### `while` - Truthiness-Based Loop

`while` loops are inherently truthy. The loop continues until the condition evaluates to `gnd` (false or null).

```weaver
var uint<3> x = 0
while x < 7 {
    x = x + 1
}
```

The loop continues until `true(x < 7)` evaluates to `gnd`, which happens when `x` is `7` or greater.

## Composition Operators

There are four process composition operators. These operators may compose statements of any kind. Operators are listed in descending precedence.

| Precedence | Operator(s) | Description |
|------------|-------------|-------------|
| 1 | `,` | Parallel (assignment only) |
| 2 | `:` | Choice (assignment only) |
| 3 | `;`, newline | Sequential |
| 4 | `and` | Parallel |
| 5 | `or` | Conditional |
| 6 | `xor` | Choice |

### Sequential Composition

Statements in a function or process are inherently composed in sequence:

```weaver
var int<32> a, b
a = 5    // First
b = 2    // Then
```

### Parallel Composition (`and`)

The `and` operator allows parallel execution:

```weaver
var int<32> a, b
a = 5 and b = 2  // a and b assigned in any order or simultaneously
```

Parallel sequences may communicate with each other:

```weaver
var int<32> a, b, c
a-, b-, c-
(
    a = 5
    await b
    c = a + b
    a-
    await ~b
) and (
    await a
    b = 3
    await ~a
    b-
)
```

This uses validity-based communication instead of mutexes.

### Conditional Composition (`or`)

The `or` operator allows only one path to proceed. If more than one path is unblocked, it causes an error (enforces deterministic behavior):

```weaver
await A {
    A.recv()
} or await B {
    B.recv()
}
```

The channels `A` and `B` must be mutually exclusive - there must never be data on both simultaneously.

### Choice Composition (`xor`)

The `xor` operator allows only one path to proceed, but can exhibit non-deterministic behavior if multiple paths are unblocked (picks one arbitrarily):

```weaver
await A {
    A.recv()
} xor await B {
    B.recv()
}
```

### Complex Composition

`await`, `if`, `while`, and assignments may be composed in many ways:

```weaver
await A {
    // ...
} and if x < 3 {
    // ...
} or while x >= 3 {
    // ...
    x = x - 1
}
```

## Channels

### Channel Declaration

```weaver
var chan<int<32>> A
var chan<bool> B
```

### Sending on Channels

Sending blocks until the receiver calls receive:

```weaver
A.send(5)
```

### Receiving from Channels

Receive removes the value from the channel and allows the sender to proceed:

```weaver
x = A.recv()
```

### Peeking at Channels

Read the value without removing it (leaves value on channel, sender remains blocked):

```weaver
x = A  // Peek at channel value
```

## Variable Declarations

Declarations must be preceded by `var` if within a `func` or process body:

```weaver
func example() int<32> {
    var int<32> x = 5
    var bool flag
    // ...
}
```

Declarations in the argument list or return values need not be preceded by `var`:

```weaver
func example(int<32> x, bool flag) int<32> {
    // x and flag are already declared
}
```

## Imports

```weaver
import "math"
```

## Comments

```weaver
// Single-line comment
```
