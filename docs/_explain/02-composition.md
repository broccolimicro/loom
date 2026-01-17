---
title: Composition Model
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

Hardware is intrinsically parallel. Unlike software, which executes one
operation at a time, hardware is always propagating every signal and executing
every operation simultaneously. So, we cannot just hide parallelism
behind implicit data-dependent concurrency or relegate it to a few special
application domains.

Weaver integrates sequencing, parallelism, and choice into a comprehensive
composition model, **making the relationships between operations in hardware
explicit**. This lets you reason about complex behaviors without hiding
parallelism.

## The Four Composition Operators

Weaver defines four core composition operators, each mapping naturally to
hardware patterns:

- **Sequential** (`;` or newline): execute one operation, then the next
- **Parallel** (`and`): execute multiple operations simultaneously
- **Conditional** (`or`): Only one of several operations is possible (deterministic)
- **Choice** (`xor`): Only one of several operations is chosen (arbitrarily, non-deterministic)

Assignments also have specialized operators:

- **Parallel assignment** (`,`): perform multiple assignments simultaneously
- **Choice assignment** (`:`): choose one of multiple assignments

Each operator expresses both **temporal relationships** and **causal
dependencies**, allowing the hardware to be synthesized predictably.

## Sequential vs. Parallel

Just because operations are written one after another doesn’t mean they
**have** to execute that way in hardware. Sequential composition gives a safe
default, operations executed in the order written should always produce a
correct result.

For example:

```weaver
a = 5
b = 2
c = a + b
```

Here, `b` does not depend on `a`, so in principle, `a = 5` and `b = 2` could
happen at the same time. But `c` depends on both, so it must happen after both
assignments. Sequential composition **defines a clear order**, but the compiler
can often identify opportunities to run independent operations in parallel.

Parallel composition is used when we want more complex behaviors, especially
when processes communicate or interact. Making parallelism explicit allows
designers to reason about **coordination and communication** in a hardware
system.

```weaver
a = 5 and b = 2
```

This tells the compiler that `a` and `b` are independent and may run
simultaneously. If these operations later communicate through channels or
shared resources, the parallel operator ensures that **valid interactions
happen correctly**, without assuming a specific timing.

In hardware, this distinction is important:

- Sequential composition is **safe by default**. You can always execute
  operations one after another.
- Explicit parallel composition lets you **exploit concurrency** and reason
  about how processes interact, which is essential for pipelines, dataflow
  algorithms, and massively parallel hardware.

Making parallelism explicit is the key to writing hardware that is both
**efficient** and **predictable**, while still being easy for humans to reason
about.

## Conditional and Choice Composition

In software, all selection is **conditional** because operations are
effectively sequential. Even when threads appear concurrent, time is discrete
and a total ordering exists. Locks, schedulers, or context switches determine
which operation happens first. Non-determinism is hidden behind these
mechanisms.

Hardware is different. Operations are inherently **parallel**: multiple things
can happen at the same time. This creates situations where more than one
operation could be ready simultaneously, and the hardware must explicitly
decide which one to handle first.

Weaver separates these concepts with two distinct composition operators:

**Conditional composition (`or`)** - only one path will execute, and the
programmer must guarantee that multiple paths are never ready at the same time.
This enforces **deterministic behavior**.

```weaver
await channelA {
	// handle A
} or await channelB {
	// handle B
}
```

Here, exactly one of the two operations can occur. If both are ready
simultaneously, it is a design error.

**Choice composition (`xor`)** - one of multiple operations will execute, but
if multiple are ready, the hardware chooses arbitrarily. This allows
**non-deterministic selection**.

```weaver
await requestA {
	grantA+
} xor await requestB {
	grantB+
}
```

Non-determinism is necessary for arbiters, load balancers, and asynchronous
protocols. In software, this kind of choice is often implicit, hidden behind
which thread acquires a lock first. Hardware requires **fine-grained, explicit
control**. The `xor` operator makes this behavior clear and analyzable.

By separating conditional and choice composition, Weaver lets designers reason
about **mutual exclusion and arbitration** directly in the hardware model,
rather than relying on hidden scheduling or implicit ordering. This explicit
handling is essential for correct, predictable, and performant parallel
hardware.

## Why Composition is Separate from Timing

In Weaver, composition defines **relationships between operations**, not
wall-clock durations. Sequential, parallel, conditional, and choice composition
describe **causal and logical constraints**, leaving the actual timing to the
compiler and backend. This separation achieves three goals:

- **Understandability** - Designers can reason about behavior in a familiar,
  sequential or conditional model.
- **Physical correctness** - The compiler respects causality, resource
  constraints, and mutual exclusion.
- **Backend flexibility** - The same composition model can target synchronous,
  asynchronous, or hybrid hardware without changing the conceptual reasoning.

