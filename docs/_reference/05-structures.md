---
title: Structures - Weaver Language Reference
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Structures

Structures describe circuit structure using production rules. Statements in a structure are inherently composed in parallel.

## Structure Syntax

```weaver
struct structureName(chan<type1> input1) chan<type2> output1 {
    // production rules
}
```

## Production Rules

Each statement follows the format of a production rule: **condition → action**

- **Condition**: Evaluated based on `valid()`. Use `true()` explicitly to evaluate truthiness. If not specified, assumed to be `vdd` (always true).
- **Action**: Either an assignment, a function call, or a process instantiation.

```weaver
a & b -> x = 5
z = x + y  // condition is vdd (always true)
```

## Example: Buffer

```weaver
struct buffer(chan<int<4>> L) chan<int<4>> R {
    R.e & L.r -> R.r = L.r
    R.r -> L.e-
    ~R.e & ~L.r -> R.r-
    ~R.r -> L.e+
}
```

This buffer structure:
- Forwards data from `L` to `R` when both are ready (`R.e & L.r`)
- Acknowledges receipt (`R.r -> L.e-`)
- Clears output when both are not ready
- Sets enable when output is not ready

## Example: FIFO

```weaver
struct fifo(chan<int<4>> L) chan<int<4>> R {
    var chan<int<4>> M[5]
    M[0] = L
    M[1] = R

    var buffer stages[4]
    stages[0](M[0], M[1])
    stages[1](M[1], M[2])
    stages[2](M[2], M[3])
    stages[3](M[3], M[4])
}
```

This FIFO structure:
- Creates an array of intermediate channels `M`
- Instantiates multiple buffer stages
- Connects them in a chain to create a FIFO queue

## Parallel Execution

All statements in a structure are evaluated in parallel. The production rules define the conditions under which each action occurs.

## Validity in Conditions

Conditions are evaluated using validity semantics. To check truthiness explicitly, use the `true()` function:

```weaver
true(x > 5) -> y = 10
valid(channel) -> process()
```
