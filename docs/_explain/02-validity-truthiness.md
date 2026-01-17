---
title: Validity and Truthiness
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

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

## Compiling to Different Backends

The validity semantic naturally compiles to any timing model.

In **quasi-delay insensitive systems**, encodings naturally encode both the value
and it's validity. For example, a 2-wire one-hot encoding (called 1of2 or
dualrail) has one null and two valid states.
- `00 = null`
- `01 = false`
- `10 = true`
- `11 = illegal`

In other **asynchronous systems**, variables are encoded with a data bus and a
valid wire. The data on the data bus is always meaningful by the time the valid
wire transitions from `gnd` to `vdd` following the bundled-data timing
assumption.

For **clocked systems**, variables are encoded with a data bus and a valid
wire. When the valid wire is high on the clock tick, then there is meaningful
data on the data bus for that clock cycle as guaranteed by the clocked timing
assumption. If the compiler can guarantee that every clock tick will have a
different valid value, then it can simply delete the valid wire.

## How Validity Propagates

Validity affects every operation. Most operators are **conjunctive**, which means
that all inputs need to be valid to produce a valid result. For example, in the
expression `a + b`, we need to know the value of *both* `a` and `b` to be able
to compute their sum.

A few operators are **disjunctive**, which means that they can produce a valid
result before all of their arguments are valid. For example, in the expression
`a == 0 || b == 0`, if 'a' is '0', then we have enough information to evaluate
the `||` operator to `true` before `b` becomes valid.

|  | Operator(s) | Validity Propagation |
|----------------------|-------------|------|
| 5 | `*`, `/`, `%` | conjunctive |
| 6 | `+`, `-` | conjunctive |
| 7 | `<<`, `>>` | conjunctive |
| 8 | `==`, `~=`, `<`, `>`, `<=`, `>=` | conjunctive |
| 9 | `^^` | conjunctive |
| 10 | `&&` | conjunctive |
| 11 | `||` | **disjunctive** |
| 12 | `^` | **disjunctive** |
| 13 | `&` | conjunctive |
| 14 | `|` | **disjunctive** |

## Conditioning on Validity vs Truthiness

Suppose we have a process which receives a boolean value over a channel.
```weaver
var chan<bool> C
```

We might want to only do something if the value that arrives on `C` is true.
```weaver
if C {
	count = count + 1
} or if !C {
	skip
}
```

Or, we might want to wait for a value to arrive on `C` before proceeding.
```weaver
await C {
	count = count + 1
}
```

Taking a step back, our boolean values can be one of three values. This means
that if statements alone are no longer sufficient
- `true` (valid and true)
- `false` (valid and false)  
- `null` (not valid)

Under the hood, conditions are evaluated as a result of a transition on a
single wire. Weaver uses two built-in functions.

- `valid(cond)` evaluates to `vdd` when `cond` is valid and `gnd` otherwise.
- `true(cond)` evaluates to `vdd` when `cond` is `true` and `gnd` otherwise.

Then `await cond` is simply a condition on `valid(cond)`, blocking until the
`cond` becomes valid, and `if cond` is a condition on `true(cond)`, blocking
until `cond` becomes `true`.

Most of the time, you do not need to explicitly call `valid()` or `true()`. The
language handles validity and truthiness automatically. But sometimes you need
to force one interpretation.

Use `valid()` when you need to check validity explicitly in a complex boolean condition.

```weaver
if valid(x) && y == 3 {
	...
}
```

Or, use `true()`. This is the same as above.

```weaver
await x & true(y == 3) {
	...
}
```

## Validity and Parallel Composition

Two parallel sequences may communicate with eachother by making use of validity and await for signalling.

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

The above is equivalent to the following, but this communication is key to
creating more complex distributed behaviors.

```weaver
a = 5
b = 3
c = a + b
a-
b-
```

Without validity, you'd need mutexes or other synchronization primitives.
Validity provides a clean, hardware-native way to coordinate parallel
processes.

Understanding validity is key to writing effective Weaver code. It's not just a
feature—it's fundamental to how Weaver works.
