---
title: Validity and Truthiness
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

Weaver introduces two distinct concepts: **validity** and **truthiness**. Understanding the difference is crucial for writing correct Weaver code.

## Validity

All variables in Weaver implicitly keep track of whether they store a valid value. A variable can be:
- **Valid**: Contains a meaningful value
- **Null/Invalid**: Does not contain a meaningful value (represented as `gnd`)

```weaver
var fixed<16,-4> a, b
a = 5    // a is valid
b-       // b is null/invalid
```

## Truthiness

Truthiness is separate from validity. For boolean values, there are three states:
- `true` - valid and true
- `false` - valid and false  
- `null` - invalid

```weaver
var bool a, b, c
a = true   // valid and true
b = false  // valid and false
c-         // null/invalid
```

## Why Validity?

Weaver is designed to make no implicit assumptions about timing other than the Isochronic Fork Assumption required by Quasi-Delay Insensitive design. However, Weaver may compile to different backends (like Verilog) that introduce timing assumptions. Validity allows Weaver to generalize to many different types of systems and behaviors by explicitly encoding the necessary assumptions.

## Effects on Operations

Validity affects every operator. The null state is represented by the wire-type `gnd`:

```weaver
gnd + 5 = gnd        // null + value = null
5 + 5 = 10           // valid + valid = valid result
gnd && true = gnd    // null && true = null
true && false = false // valid && valid = valid result
gnd & true = gnd     // wire-level: null & true = null
vdd & true = vdd     // wire-level: valid & true = valid
gnd | true = vdd     // wire-level: null | true = valid
```

## Built-in Functions

When the implicit validity/truthiness rules are getting in the way, use these built-in functions:

| Function | Category | Description |
|----------|----------|-------------|
| `valid(x)` | Validity | Type cast `x` to a wire that is `vdd` when `x` is valid and `gnd` when `x` is null. |
| `true(x)` | Truthiness | Type cast `x` to a bool that is `true` when `x` is valid and truthy, and null otherwise. |

## Usage in Conditions

- **`await`** blocks operate on validity - they wait for a variable to become valid
- **`if`** blocks operate on truthiness - they wait for a condition to become true

```weaver
var fixed<16,-4> x
x = 3
await x {        // Waits for x to be valid
    // ...
}

if x == 3 {      // Waits for x == 3 to be true
    // ...
}
```

## Making Variables Invalid

Use the `-` operator to make a variable invalid:

```weaver
var int<32> x = 5
x-  // x is now null/invalid
```

## Making Variables Valid

Use the `+` operator or assignment to make a variable valid:

```weaver
var wire a
a+        // a is now vdd (valid)
a = vdd   // a is now vdd (valid)

var int<32> x
x = 10    // x is now valid with value 10
```
