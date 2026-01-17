---
title: Composition Model
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

Most software languages are fundamentally sequential. One statement executes,
then the next, in a predictable order. Even with threads or async/await,
there's an underlying sequential execution model.

Hardware is different. Multiple things happen simultaneously. A CPU might fetch
an instruction, decode another, execute a third, and write back a fourth, all at
the same time. This parallelism is not optional, it's how hardware works.

Traditional hardware description languages handle this by making parallelism
implicit, and sequencing is implemented by hand using state machines.
Everything happens in parallel unless you say otherwise.

This approach is very hard to reason about because people naturally reason
sequentially. The end result is that designers must create two descriptions: a
behavioral model that the programmer can reason about, and a structural
implementation that the compiler can reason about. Then, the programmer must
manually tie those two descriptions together with testing and verification.
This takes so much time and effort that most people doing chip design are
employed simply to test.

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

**Weaver is fundamentally sequential and physical.**

![test]({{site.baseurl}}/images/loom.svg)

Weaver  

Weaver provides explicit composition operators that let you control how
statements relate to each other.

## The Four Composition Operators

Weaver has four main composition operators (plus assignment-specific variants):

1. **Sequential** (`;` or newline): One thing happens, then the next
2. **Parallel** (`and`): Multiple things happen simultaneously
3. **Conditional** (`or`): Only one of several things is possible (deterministic)
4. **Choice** (`xor`): Only one of several things is chosen (non-deterministic)

Each serves a different purpose and maps to different hardware patterns.

## Sequential Composition: The Default

Sequential composition is what you're used to from programming:

```weaver
a = 5
b = 2
c = a + b
```

This says: set `a` to 5, *then* set `b` to 2, *then* compute `c`. The order matters.

In hardware, sequential composition typically maps to:
- Pipeline stages (one stage completes before the next begins)
- State machine transitions (one state leads to the next)
- Clocked logic (one clock cycle, then the next)

But remember: even in sequential composition, Weaver doesn't assume a specific timing model. The compiler decides how to implement the sequencing based on the target backend.

## Parallel Composition: Hardware's Natural Mode

Parallel composition is where Weaver differs most from traditional languages:

```weaver
a = 5 and b = 2
```

This says: set `a` to 5 *and* set `b` to 2, in any order or simultaneously. The order doesn't matter.

In hardware, parallel composition maps to:
- Independent operations that can happen simultaneously
- Multiple pipeline stages processing different data
- Concurrent processes communicating through channels

### Why Not Just Make Everything Parallel?

You might wonder: if hardware is parallel, why not make everything parallel by default?

The answer is that *some* operations need sequencing. If `b` depends on `a`, they can't happen in parallel:

```weaver
a = 5
b = a + 1  // b depends on a, so this must be sequential
```

Weaver uses sequential composition by default because it's the safest assumption. You can always use `and` when you want parallelism, but you can't easily add sequencing if everything is parallel by default.

### Parallel Composition and Validity

Parallel composition relies heavily on validity for coordination:

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
1. Both processes can start (they're composed in parallel)
2. They coordinate through validity (one waits for the other)
3. They don't interfere (validity prevents race conditions)

This is fundamentally different from software parallelism, which uses locks, mutexes, or message passing. Validity provides a hardware-native coordination mechanism.

## Conditional Composition: Mutual Exclusion

Conditional composition (`or`) says: exactly one of these paths will execute, and they must be mutually exclusive:

```weaver
await channelA {
    // Handle A
} or await channelB {
    // Handle B
}
```

This requires that `channelA` and `channelB` are never both ready at the same time. If they are, it's an error.

### Why Mutual Exclusion?

Mutual exclusion ensures deterministic behavior. If both channels can be ready simultaneously, which one do you choose? The `or` operator says: "this shouldn't happen, and if it does, it's an error."

This is useful for:
- Protocol handlers (only one message type at a time)
- State machines (only one transition at a time)
- Resource arbitration (only one requester at a time)

### Enforcing Mutual Exclusion

The compiler can check that mutual exclusion is maintained. If your design allows both paths to be ready simultaneously, the compiler can warn you or error. This helps catch design bugs early.

## Choice Composition: Non-Deterministic Selection

Choice composition (`xor`) says: one of these paths will execute, and if multiple are ready, pick one arbitrarily:

```weaver
await channelA {
    // Handle A
} xor await channelB {
    // Handle B
}
```

This allows non-deterministic behavior. If both channels are ready, the implementation picks one.

### When to Use Choice

Choice is useful for:
- Arbiters (multiple requests, pick one)
- Load balancers (multiple workers, pick one)
- Non-deterministic protocols (multiple valid responses, pick one)

### Why Allow Non-Determinism?

Non-determinism is sometimes necessary or desirable:
- **Performance**: An arbiter might pick the first available resource
- **Fairness**: A round-robin arbiter might cycle through options
- **Protocol requirements**: Some protocols require arbitrary selection

The `xor` operator makes this non-determinism explicit, so readers know the behavior might vary.

## Assignment Composition

Assignments have special composition operators:

- `,` (comma): Parallel assignment
- `:` (colon): Choice assignment

These are syntactic sugar for common patterns:

```weaver
a = 5, b = 2  // Parallel: both happen
a = 5 : b = 2  // Choice: one or the other
```

## Composition Precedence

Composition operators have precedence, just like arithmetic operators:

1. `,` (parallel assignment) - highest
2. `:` (choice assignment)
3. `;` or newline (sequential)
4. `and` (parallel)
5. `or` (conditional)
6. `xor` (choice) - lowest

This precedence determines how complex compositions are parsed:

```weaver
a = 5 and b = 2 or c = 3
```

This is parsed as: `(a = 5) and ((b = 2) or (c = 3))`.

## Real-World Patterns

### Pipeline Pattern

```weaver
stage1() and stage2() and stage3()
```

Each stage processes data independently. They're composed in parallel because they can all be active simultaneously.

### State Machine Pattern

```weaver
if state == IDLE {
    // Transition to ACTIVE
} or if state == ACTIVE {
    // Transition to DONE
}
```

Only one state transition happens at a time. The `or` ensures mutual exclusion.

### Arbiter Pattern

```weaver
await requestA {
    grantA+
} xor await requestB {
    grantB+
}
```

Multiple requests, pick one. The `xor` allows non-deterministic selection.

## Design Trade-offs

The composition model has trade-offs:

### Advantages

- **Explicit parallelism**: You control what happens in parallel
- **Hardware-native**: Maps directly to hardware patterns
- **Composable**: Complex behaviors built from simple compositions
- **Verifiable**: The compiler can check composition properties

### Disadvantages

- **Learning curve**: Different from software languages
- **Verbosity**: Sometimes you need explicit composition operators
- **Complexity**: Complex compositions can be hard to understand

Overall, the advantages outweigh the disadvantages. The explicit composition model makes hardware designs clearer and more verifiable.

## Summary

Weaver's composition model is designed for hardware, where parallelism is fundamental. The four composition operators—sequential, parallel, conditional, and choice—let you express different hardware patterns explicitly. Understanding when to use each operator is key to writing effective Weaver code.

The composition model, combined with validity, enables clean coordination of parallel processes without traditional synchronization primitives. This makes Weaver code both more expressive and more verifiable than traditional hardware description languages.
