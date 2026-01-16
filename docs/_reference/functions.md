---
title: Functions - Weaver Language Reference
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Functions

Functions in Weaver are pure, side-effect-free computations that terminate and return values.

## Function Syntax

```weaver
func functionName(type1 arg1, type2 arg2) returnType {
    // function body
}
```

## Characteristics

- **Terminate**: Functions always complete and return
- **No side effects**: Functions only affect their return values
- **Inlined**: All functions are inlined by the compiler
- **Time-multiplexing**: The compiler may decide to time-multiplex the resulting circuitry across different function calls

## Return Values

Return values are implicit. The value remaining on the return variable at the end of the function is returned. If no assignments are made to the return variable, its value remains unmodified.

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

## Function Calls

```weaver
var fixed<16,-4> a, b
a = 4
b = sqrt(a)  // b receives the return value of sqrt(a)
```

## Variable Declarations

Variable declarations within a function must be preceded by `var`:

```weaver
func example() int<32> {
    var int<32> x = 5
    var bool flag = true
    // ...
}
```

Declarations in the argument list or return values need not be preceded by `var`.
