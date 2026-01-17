---
title: Types
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

Weaver has a small set of built-in types for representing hardware signals and data.

## Built-in Types

### `wire`

Represents a single wire with two potential values: `vdd` (high/logic 1) and `gnd` (low/logic 0).

```weaver
var wire a, b, c, d, e
a+        // vdd
a-        // gnd
b = gnd   // gnd
b = vdd   // vdd
c = a & b // gnd & vdd = gnd
d = a | b // gnd | vdd = vdd
e = ~a    // ~gnd = vdd
```

### `bool`

Represents a boolean value. Unlike `wire`, boolean values have three states: `true`, `false`, and `null` (invalid).

```weaver
var bool a
a = true
a = false
a-  // null/invalid
```

### `fixed<width, offset>`

Represents a two's complement binary coded fixed-point value with specified width (in bits) and offset.

```weaver
var fixed<16,-4> b
b = 3.5
b = 2
```

The offset determines the position of the binary point. For example, `fixed<16,-4>` has 4 fractional bits.

### `int<width>`

Aliased to `fixed<width, 0>` for convenience. Represents a signed integer.

```weaver
var int<16> c
c = 4
c = 25
```

### `ufixed<width, offset>`

Represents an unsigned binary coded fixed-point value with specified width and offset.

### `uint<width>`

Aliased to `ufixed<width, 0>` for convenience. Represents an unsigned integer.

```weaver
var uint<32> x
x = 0
x = 4294967295
```

### `chan<type>`

Represents a channel along which values of the specified type may be communicated between processes.

```weaver
var chan<int<32>> A
var chan<bool> B
```

## Type Conversions

All constants and compile-time expressions are arbitrary width and precision, but are implicitly cast by any assignment or operator with bounded width or precision.

```weaver
var int<16> c
c = 35 * 0.1  // 3.5 is then truncated to 3 because c is an integer
```

## Arrays

Arrays can be declared with dimensions:

```weaver
var fixed<16,-4> a[3] = [1, 2, 3.5]
var fixed<16,-4> b[4][2] = [[1, 2], [3, 4], [5, 6], [7, 8]]
```

## Validity

All variables in Weaver implicitly keep track of whether they store a valid value. A variable can be made invalid (null) using the `-` operator:

```weaver
var fixed<16,-4> a, b
a = 5    // a is valid
b-       // b is null/invalid
```

The validity state affects all operations. See [Validity and Truthiness](./validity-truthiness) for details.
