---
title: Types
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

## Comments

```weaver
// Single-line comment
/* Multi
line
comment */
```

## Imports

```weaver
import "math"
```

## Type Declaration

Types specify how wires should be grouped into a bus. They allow you to organize related fields together.

```weaver
type typeName {
    type1 field1
    type2 field2, field3
    type3 field4
}
```

The following is an example of a type definition to represent a CPU opcode.

```weaver
type opcode {
    int<4> fn, rs, rt, rd
}

var opcode op = {3, 0, 1, 2}
if op.fn == 0 {
    rf[op.rd] = rf[op.rs] + rf[op.rt]
}
```

Fields are accessed using the dot operator:

```weaver
op.fn    // Access the fn field
op.rs    // Access the rs field
x.mybool // Access the mybool field
```

Weaver has a small set of built-in types for representing hardware signals and data.

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

Sending blocks until the receiver calls receive:

```weaver
A.send(5)
```

Receive removes the value from the channel and allows the sender to proceed:

```weaver
x = A.recv()
```

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

Declarations in the argument list or return values for `func`, `struct`, or `type` need not be preceded by `var`.

```weaver
func example(int<32> x, bool flag) int<32> {
    // x and flag are already declared
}
```

Arrays can be declared with dimensions:

```weaver
var fixed<16,-4> a[3] = [1, 2, 3.5]
var fixed<16,-4> b[4][2] = [[1, 2], [3, 4], [5, 6], [7, 8]]
```

## Assignments

Assignments may be composed in parallel or choice based on the `,` and `:` operators:
- `,` - Parallel assignment (all assignments happen simultaneously)
- `:` - Choice assignment (only one assignment path is taken)

```weaver
var fixed<16,-4> a[3] = [1, 2, 3.5], b[4][2] = [[1, 2], [3, 4], [5, 6], [7, 8]]
var myType x = {1, true, 2.2}
a[2] = 5, b[1][1] = 3 : x.mybool = false
```

## Operator Precedence

Operators are listed in descending precedence (highest to lowest).

| Precedence | Operator(s) | Description |
|------------|-------------|-------------|
| 1 | `[ ... ]` | Array literal or grouped list expression. Highest binding strength. |
| 2 | `::` | Namespace or scope resolution operator. |
| 3 | `a'1`<br>`f(a, b, ...)`<br>`a.b`<br>`a[b:c]`<br>`a[b]` | Isochronic-region identifier,<br>function call,<br>member access,<br>slicing,<br>indexing. |
| 4 | `!`, `~`, `+`, `-` | Prefix operators: boolean NOT, wire-level NOT, identity, negation. |
| 5 | `*`, `/`, `%` | Multiplication, division, and modulo. `*` is commutative. |
| 6 | `+`, `-` | Addition and subtraction. `+` is commutative; `-` is not. |
| 7 | `<<`, `>>` | Logical bit shifts. |
| 8 | `==`, `!=`, `<`, `>`, `<=`, `>=` | Comparison and equality operators. Produce boolean results. |
| 9 | `^^` | Boolean exclusive OR. True if operands differ. Commutative. |
| 10 | `&&` | Boolean AND on boolean expressions. Short-circuit semantics at the language level. Commutative. |
| 11 | `\|\|` | Boolean OR on boolean expressions. Short-circuit semantics at the language level. Commutative. |
| 12 | `^` | Wire-level XOR. Element-wise and commutative. |
| 13 | `&` | Wire-level AND. Element-wise and commutative. |
| 14 | `\|` | Wire-level OR. Element-wise and commutative. |
| 15 | `?:` | Conditional selection. Evaluates the condition before `?`; if true evaluates the middle expression, otherwise the final expression. |

## Validity

All variables in Weaver implicitly keep track of whether they store a valid value. A variable can be made invalid (null) using the `-` operator:

```weaver
var fixed<16,-4> a, b
a = 5    // a is valid
b-       // b is null/invalid
```

When the implicit validity/truthiness rules are getting in the way, use these built-in functions:

| Function | Category | Description |
|----------|----------|-------------|
| `valid(x)` | Validity | Type cast `x` to a wire that is `vdd` when `x` is valid and `gnd` when `x` is null. |
| `true(x)` | Truthiness | Type cast `x` to a bool that is `true` when `x` is valid and truthy, and null otherwise. |

Validity affects all operations. See [Validity and Truthiness]({{site.baseurl}}/explain/01-validity-truthiness) for details.

## Constants

All constants and compile-time expressions are arbitrary width and precision, but are implicitly cast by any assignment or operator with bounded width or precision.

```weaver
var int<16> c
c = 35 * 0.1  // 3.5 is then truncated to 3 because c is an integer
```

