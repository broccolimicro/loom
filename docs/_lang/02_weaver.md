---
title: Weaver
author: Edward Bingham
date: 2024-09-24  #TODO: 2026-01-16, but hacked to order correctly
category: Language
layout: post
---

This document describes the desired minimal functionality for the Weaver language for
v1.0.0 of Loom.

Weaver is a hardware description language whose mission is to make it easy for
software engineers to design high-quality complex computer architectures. It
balances three core building-blocks: Behavior, Structure, and Types.

## Behavior

You may describe your circuit as a behavior, with much of the usual syntax.
There are two types of behaviors: processes and functions.

### Processes

Processes represent one or more pipeline stages in your design. As a result,
all processes have a perpetual outer loop and should never terminate. All of
the statements that come before the outer loop are handled during reset.

Arguments are typically channels along which data is communicated. However,
non-channel arguments may be used to represent shared variables in a more
complex handshake. 

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

Above shows an example of a fetch unit in a CPU. While this may be compiled
into multiple pipeline stages, the number and order of communications on the
input and output channels in the list of arguments and returns will be
preserved with no-ops and backpressure built-in.

### Functions

Functions terminate and have no side-effects other than the returned values.
All functions are inlined, though the compiler may decide to time-multiplex the
resulting circuitry across different function calls.

```weaver
import "math"

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

This example shows a Newton's method approximation of square root given a
fixed-point value `x`. The value remaining on `y` at the end of the function is
implicitly returned. If no assignments are made on `y`, then the value of `b`
in the following assignment will be left un-modified.

```weaver
var fixed<16,-4> a, b
a = 4
b = sqrt(a)
```

### Built-in Types

Weaver has a small set of built-in types:
- `wire` represents a single wire with two potential values `vdd` and `gnd`
- `bool` represents a boolean value
- `fixed<width, offset>` represents a two's complement binary coded fixed-point value with offset and width in bits
- `int<width>` is aliased to `fixed<width, 0>` for convenience
- `ufixed<width, offset>` represents an unsigned binary coded fixed-point value with offset and width in bits
- `uint<width>` is aliased to `ufixed<width, 0>` for convenience
- `chan<type>` represents a channel along which values may be communicated

**Wires** may be assigned to constant values or other wires
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

**Bool**, **Fixed**, and **Int** encode basic kinds of data and may be assigned similarly.

```weaver
var bool a
var fixed<16,-4> b
var int<16> c
a = true
a = false
b = 3.5
b = 2
c = 4
c = 25
```

All constants and compile-time expressions are arbitrary width and precision,
but implicitly cast by any assignment or operator with bounded width or
precision.

```weaver
c = 35 * 0.1 // 3.5 is then truncated to 3 because c is an integer
```

**Channels** allow you to communicate data between processes.

```weaver
var chan A
```

When sending a value, execution will block until the receiver on the other end
calls receive.

```weaver
A.send(5)
```

Receive removes the value from the channel and allows the sender to proceed.

```weaver
x = A.recv()
```

Alternatively, the receiver may read the value on the channel without executing
a full receive. This leaves the value on the channel and the sender remains
blocked.

```weaver
x = A
```

### Validity and Truthiness

Weaver is designed to make no implicit assumptions about timing other than the
Isochronic Fork Assumption required by Quasi-Delay Insensitive design. However,
Weaver may compile to different backends that introduce assumptions. For
example, Weaver may compile to Verilog, making the clocked timing assumption in
the process. This allows Weaver to generalize to many different types of
systems and behaviors by explicitly the necessary assumptions. To support this,
we must introduce the concept of Validity.  

All variables in Weaver implicitly keep track of whether they store a valid
value. Below, `a` is valid, `b` is null.

```weaver
var fixed a, b
a = 5
b-
```

This is separate from Truthiness, and boolean values have three different
values. Below, `a` and `b` are both valid and `c` is null.

```weaver
var bool a, b, c
a = true
b = false
c-
```

This has many downstream effects throughout expressions, conditions and loops,
and composition. In the event the implicit rules are getting in the way, there
are two built-in functions to force one evaluation over the other.

| Built-in Function | Category | Description |
|-------------------|----------|-------------|
| `valid(x)` | Validity | Type cast `x` to a wire that is `vdd` when `x` is valid and `gnd` when `x` is null. |
| `true(x)` | Truthiness | Type cast `x` to a bool that is `true` when `x` is valid and truthy, and null otherwise. |

### Operators and Precedence

Operators are listed in descending precedence.

|  | Operator(s) | Kind | Description |
|----------------------|-------------|------|-------------|
| 1 | `[ ... ]` | Group | Array literal or grouped list expression. Highest binding strength. |
| 2 | `::` | Modifier | Namespace or scope resolution operator. |
| 3 | `a'1`<br>`f(a, b, ...)`<br>`a.b`<br>`a[b:c]`<br>`a[b]` | Postfix / Modifier | Isochronic-region identifier,<br>function call,<br>member access,<br>slicing,<br>indexing. |
| 4 | `!`, `~`, `+`, `-` | Unary | Prefix operators: boolean NOT, wire-level NOT, identity, negation. |
| 5 | `*`, `/`, `%` | Binary | Multiplication, division, and modulo. `*` is commutative. |
| 6 | `+`, `-` | Binary | Addition and subtraction. `+` is commutative; `-` is not. |
| 7 | `<<`, `>>` | Binary | Logical bit shifts. |
| 8 | `==`, `~=`, `<`, `>`, `<=`, `>=` | Binary | Comparison and equality operators. Produce boolean results. |
| 9 | `^^` | Binary | Boolean exclusive OR. True if operands differ. Commutative. |
| 10 | `&&` | Binary | Boolean AND on boolean expressions. Short-circuit semantics at the language level. Commutative. |
| 11 | `||` | Binary | Boolean OR on boolean expressions. Short-circuit semantics at the language level. Commutative. |
| 12 | `^` | Binary | Wire-level XOR. Element-wise and commutative. |
| 13 | `&` | Binary | Wire-level AND. Element-wise and commutative. |
| 14 | `|` | Binary | Wire-level OR. Element-wise and commutative. |
| 15 | `?:` | Ternary | Conditional selection. Evaluates the condition before `?`; if true evaluates the middle expression, otherwise the final expression. |

Keep in mind that validity affects every operator. The null state is represented by the wire-type `gnd`. For example

```weaver
gnd + 5 = gnd
5 + 5 = 10
gnd && true = gnd
true && false = false
gnd & true = gnd
vdd & true = vdd
gnd | true = vdd
```

Assignments may be composed in parallel or choice based on the `,` and `:`
operators. Declarations must be preceded by `var` if within a `func`.
Declarations in the arg-list or return values need not be preceded by `var`.

```weaver
var fixed<16,-4> a[3] = [1, 2, 3.5], b[4][2] = [[1, 2], [3, 4], [5, 6], [7, 8]]
var myType x = {1, true, 2.2}
a[2] = 5, b[1][1] = 3 : x.mybool = false
```

### Conditions and Loops

There are two types of conditionals based on the difference between validity
and truthiness. The first operates on validity. Await blocks program execution
until the condition expression is valid. For a condition expression `cond`,
`valid(cond)` must evaluate to `vdd`.

```weaver
var fixed<16,-4> x
x = 3
await x {
	...
}
```

This may also be used to wait for a valid value on a channel.

```weaver
var chan<bool> A
var chan<int<32> > B
await A & B {
	A.recv()
	B.recv()
}
```

The best way to think about await is that we are waiting for an event to occur,
a transition on a wire from `gnd` to `vdd`.

The second operates on truthiness. If blocks program execution until the
condition expression is true. For a condition expression `cond`, `true(cond)`
must evaluate to `vdd`.

```weaver
var fixed<16,-4> x
x = 3
if x == 3 {
	...
}
```

Keep in mind that the if-statement in Weaver is blocking and there is no
`else`. This is unlike if statements in any other language.

While loops are inherently truthy, and the following example loops until
`true(x < 7)` evaluates to `gnd`, which happens when `x` is `7`.

```weaver
var uint<3> x = 0
while x < 7 {
	x = x + 1
}
```

### Composition

There are four process composition operators. These operators may compose
statements of any kind. Operators are listed in descending precedence.

|  | Operator(s) | Description |
|----------------------|-------------|-------------|
| 1 | `,` | Parallel (assignment only) |
| 2 | `:` | Choice (assignment only) |
| 3 | `;`, newline | Sequential |
| 4 | `and` | Parallel |
| 5 | `or` | Conditional |
| 6 | `xor` | Choice |

As you have already seen in the previous examples, statements in a function or
process are inherently composed in sequence. In the following example, `a` is
assigned to `5`, **then** `b` is assigned to `2`.

```weaver
var int<32> a, b
a = 5
b = 2
```

Weaver also has parallel composition represented by the `and` operator. In the
following example, `a` is assigned to `5` and `b` is assigned to `2` in any
order or even simultaneously.

```weaver
var int<32> a, b
a = 5 and b = 2
```

Again, this is unlike any language, and this operator is heavily dependent upon
the concept of validity instead of using mutexes. For example two parallel
sequences may communicate with eachother.

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

The above is equivalent to the following, but this communication is key
to creating more complex distributed behaviors.

```weaver
a = 5
b = 3
c = a + b
a-
b-
```

Finally, there is the condition and choice operators `or` and `xor`. Both allow
only one of the paths to proceed. However, if more than one of the two paths is
unblocked, then `or` would cause an error, and `xor` would pick one of the
unblocked paths arbitrarily.

In the following example, the channels `A` and `B` must be mutually exclusive.
There must never be data on both `A` and `B` simultaneously. This enforces
deterministic behavior.

```weaver
await A {
	A.recv()
} or await B {
	B.recv()
}
```

However, with an `xor`, this program can exhibit non-deterministic behavior,
picking one of the two sides arbitrarily.

```weaver
await A {
	A.recv()
} xor await B {
	B.recv()
}
```

Await, if, while, and assignments may be composed in a myrad of ways.

```weaver
await A {
	...
} and if x < 3 {
	...
} or while x >= 3 {
	...
	x = x - 1
}
``` 

## Structure

Structures describe circuit structure. Statements in a structure are inherently composed in parallel.

```weaver
struct buffer(chan<int<4> > L) chan<int<4> > R {
	R.e & L.r -> R.r = L.r
	R.r -> L.e-
	~R.e & ~L.r -> R.r-
	~R.r -> L.e+
}

struct fifo(chan<int<4> > L) chan<int<4> > R {
	var chan<int<4> > M[5]
	M[0] = L
	M[1] = R

	var buffer stages[4]
	stages[0](M[0], M[1])
	stages[1](M[1], M[2])
	stages[2](M[2], M[3])
	stages[3](M[3], M[4])
}
```

### Production Rule

Each statement follows the format of a production rule: condition implies
action. Where action is either an assignment, a function call, or a process
instantiation. The condition is evaluated based on `valid()`, and `true()` must
be used explicitly to evaluate truthiness. If the condition is not specified,
it is assumed to be `vdd`.

```
a & b -> x = 5
z = x + y
```

## Types

Types specify how data should be grouped into a bus.

```weaver
type opcode {
	int<4> fn, rs, rt, rd
}

var opcode op = {3, 0, 1, 2}
if op.fn == 0 {
	rf[op.rd] = rf[op.rs] + rf[op.rt]
}
```
