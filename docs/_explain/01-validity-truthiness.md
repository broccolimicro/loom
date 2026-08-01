---
title: Validity and Truthiness
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

Most hardware description languages are based on register-transfer logic
(Verilog, SystemVerilog, Chisel, VHDL, etc.). These languages assume a global
clock that defines when data is valid, and the entire design is built around
ensuring data is ready before the next clock tick.

This works, but it comes with costs. Clock-domain crossings are difficult.
Interfaces between subsystems are fragile. IO boundaries require careful
timing assumptions. Once you step outside a single synchronous domain, things
quickly become complicated.

Modern chips rarely live in one clock domain. Different cores often run at
different frequencies. Communication between them happens over asynchronous
networks that are hard to reason about. Interfacing with analog systems or
non-standard logic families often means falling back to black-box models.

**Weaver takes a different approach.**

Instead of starting with a clock, Weaver starts with the minimum timing
assumption needed to express computation at all. Stronger timing assumptions
can then be added later. This allows the same program to target multiple
backends, each with a different timing model.

In a clocked system, the clock answers a basic question for you: “Is this data
valid right now?” If the answer is “no”, designers add explicit flow-control
signals like valid-ready to handle stalls and backpressure.

But Weaver does not assume a clock. So how do you say “this value isn’t ready
yet” or “this computation hasn’t finished” without baking in a specific timing
model?

## The Solution: Validity

In Weaver, *every variable tracks whether it currently has a meaningful value.*
- **Valid**: the variable holds a usable value
- **Null**: the variable does not have a value (yet, or anymore)

For a software engineer, validity is similar to pointers. A pointer may be
`nullptr`, or it may point to a real value in memory.

For a hardware engineer, validity is like the `valid` signal in a val-rdy or
AXI-style interface. The data bus is only meaningful when valid is asserted.

The key difference is that in Weaver, **this applies to every value**, not just
channels or interfaces.

For example, a `bool` has three possible states:
- `true` → valid and true
- `false` → valid and false
- `null` → not valid

Even a `wire` may be thought of in these terms:
- `vdd` → valid
- `gnd` → not valid

## Why Separate Validity and Truthiness?

Many software languages already have null or undefined. Why isn’t that
enough? Why explicitly separate validity from truth?

The reason is parallelism.

Most software languages are fundamentally sequential. Statements execute in a
well-defined order. Parallelism exists, but only through heavy abstractions
like threads, locks, shared memory, and cache coherence. In that world,
validity is not used to coordinate access to shared resources.

Weaver describes hardware.

In hardware, parallelism is the default, and it is extremely fine-grained.
Signals are shared at the level of individual wires. Validity is how Weaver
coordinates access to those wires without locks, mutexes, or global ordering.

Truthiness serves a different role.

- **Validity answers:** “Is this value available yet?” This is about timing and
  data flow.
- **Truthiness answers:** “Is this condition true?” This is about logic and
  control flow.

These are not the same question. For example,

```weaver
var bool error_flag
// ... later ...
if error_flag {
    // Handle error
}
```

If `error_flag` is `false`, the operation completed successfully. That is a
meaningful result.

If `error_flag` is `null`, the operation has not completed yet. It may still
produce an error.

By separating validity from truthiness, Weaver lets you distinguish between
“false” and “not finished” without assuming anything about how long the
operation takes.

## Compiling to Different Backends

Validity maps cleanly onto many hardware styles.

In **quasi-delay insensitive systems**, the encoding itself represents both the
value and its validity. For example, a dual-rail (1-of-2) encoding:
- `00` → `null`
- `01` → `false`
- `10` → `true`
- `11` is illegal

In **bundled-data asynchronous systems**, values are represented by a data bus
plus a valid wire. The data is guaranteed to be meaningful when the valid wire
transitions.

In **clocked systems**, values are also represented by a data bus and a valid
wire. The clock guarantees that the data is meaningful when valid is high on
the clock edge. If the compiler can prove that a value is always valid every
cycle, the valid wire can be optimized away entirely.

## How Validity Propagates

Validity participates in every operation.

Most operators are **conjunctive**: all inputs need to be valid to produce a
valid output. For example, `a + b` cannot be computed until both `a` and `b`
are valid.

Some operators are **disjunctive**: they may produce a valid
result as soon as enough information is available. For example, in:

```weaver
a == 0 || b == 0
```

If 'a' is valid and equal to '0', the result is `true` even if `b` is not yet
valid.

| Operator(s) | Validity Propagation |
|-------------|------|
| `*`, `/`, `%` | conjunctive |
| `+`, `-` | conjunctive |
| `<<`, `>>` | conjunctive |
| `==`, `!=`, `<`, `>`, `<=`, `>=` | conjunctive |
| `^^` | conjunctive |
| `&&` | conjunctive |
| `||` | **disjunctive** |
| `^` | **disjunctive** |
| `&` | conjunctive |
| `|` | **disjunctive** |

## Conditioning on Validity vs Truthiness

Suppose a process received a boolean over a channel.
```weaver
var chan<bool> C
```

If you want to act when the value is true, use an `if` statement to check
**truthiness**.
```weaver
if C {
	count = count + 1
}
```

If you want to act when the value arrives, use an `await` statement to check
**validity**.
```weaver
await C {
	count = count + 1
}
```

Internally, conditions are evaluated as a result of a transition on a single
wire. Weaver uses two built-in functions.

- `valid(x)` evaluates to `vdd` when `x` is valid and `gnd` otherwise. `await
  x` waits for `valid(x)`
- `true(x)` evaluates to `vdd` when `x` is `true` and `gnd` otherwise. `if x`
  waits for `true(x)`

Most of the time, this happens automatically. You only need to be explicit when
mixing validity and logic in complex expressions.

```weaver
if valid(x) && y == 3 {
	...
}
```

or

```weaver
await x & true(y == 3) {
	...
}
```

## Validity and Parallel Composition

Validity also allows parallel processes to synchronize without locks.

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

This is equivalent to:

```weaver
a = 5
b = 3
c = a + b
a-
b-
```

The difference is that validity provides the signaling needed to coordinate
these actions safely in parallel.

Without validity, this would require mutexes or explicit synchronization logic.
With validity, coordination is implicit, local, and hardware-native.

Validity is not just a feature of Weaver. It is the foundation that allows
parallel hardware behavior to be expressed clearly, safely, and without
assuming a specific timing model.
