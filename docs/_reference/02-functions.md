---
title: Behavior
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

`func` in Weaver can represent one of two things:
- a transient function that returns a value or values.
- a persistent process that communicates across channels.

## Functions

Functions have the following properties:
- **Terminate**: Functions always complete and return
- **No side effects**: Functions only affect their return values
- **Inlined**: All functions are inlined by the compiler

```weaver
func functionName(type1 arg1, arg2; type2 arg3) type3 arg4, arg5; type4 arg6 {
    // function body
}
```

Return values are implicit. The value remaining on the return variable at the
end of the function is returned. If no assignments are made to the return
variable, its value remains unmodified. For example:

```weaver
func sqrt(fixed<16,-4> x) fixed<16,-4> y {
    var fixed<16,-4> y1

    if x < 1 {
        y = 1
        y1 = 0.5 + x/2
    } or if x >= 1 {
        y = x/2
        y1 = 1 + x/4
    }

    var uint<10> i = 1023
    while i != 0 && y != y1 {
        y = y1
        y1 = (y + x/y)/2
        i = i - 1
    }
}
```

In the example above, `y` is the return variable. Its final value is implicitly returned.

```weaver
var fixed<16,-4> a, b
a = 4
b = sqrt(a)  // b receives the return value of sqrt(a)
```

## Processes

Processes represent one or more pipeline stages in your design. They are the
primary mechanism for describing concurrent, stateful hardware behavior.

```weaver
func processName(chan<type1> input1, input2; chan<type2> input3)
	chan<type3> output1, output2; chan<type4> output3 {
    // reset statements
    while {
        // process body (perpetual loop)
    }
}
```

Processes have the following properties:
- **Non-terminating loops**: All processes have a non-terminating outer loop
- **Reset behavior**: All statements that come before the outer loop are handled during reset
- **Channel communication**: Arguments are typically channels along which data
	is communicated. Non-channel arguments may also be used to represent shared
  variables in a more complex handshake protocol.
- **Decomposition**: The compiler may split a process into multiple pipeline
  stages. The communication order on channels is preserved, ensuring correct
  behavior even when pipelined.

The following example shows a fetch unit in a CPU that:
1. Sends the current program counter (`pc`) on the `Addr` channel
2. Waits for either a jump (`Jmp`) or increment (`Inc`) signal
3. Updates `pc` accordingly and continues

```weaver
func fetch(chan Inc, Jmp) chan Addr {
    var uint<32> pc = 0
    while {
        Addr.send(pc)
        await Jmp {
            pc = Jmp.recv()
        } or await Inc {
            Inc.recv()
            pc = pc + 1
        }
    }
}
```

## Control-Flow Operators

### `await` - Validity-Based Waiting

`await` blocks program execution until the condition expression is valid. For a
condition expression `cond`, `valid(cond)` must evaluate to `vdd`.

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

`if` blocks program execution until the condition expression is true. For a
condition expression `cond`, `true(cond)` must evaluate to `vdd`.

```weaver
var fixed<16,-4> x
x = 3
if x == 3 {
    // Executes when x == 3 is true
}
```

**Important**: The `if` statement in Weaver is **blocking** and there is **no
`else`**. This is unlike if statements in any other language.

### `while` - Truthiness-Based Loop

`while` loops are inherently truthy. The loop continues until the condition
evaluates to `gnd` (false or null).

```weaver
var uint<3> x = 0
while x < 7 {
    x = x + 1
}
```

The loop continues until `true(x < 7)` evaluates to `gnd`, which happens when `x` is `7` or greater.

## Composition Operators

There are four process composition operators. These operators may compose
statements of any kind. Operators are listed in descending precedence.

| Precedence | Operator(s) | Description |
|------------|-------------|-------------|
| 1 | `;`, newline | Sequential |
| 2 | `and` | Parallel |
| 3 | `or` | Conditional |
| 4 | `xor` | Choice |

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

