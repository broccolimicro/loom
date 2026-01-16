---
title: Operators - Weaver Language Reference
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Operators

Operators are listed in descending precedence (highest to lowest).

## Operator Precedence Table

| Precedence | Operator(s) | Kind | Description |
|------------|-------------|------|-------------|
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
| 11 | `\|\|` | Binary | Boolean OR on boolean expressions. Short-circuit semantics at the language level. Commutative. |
| 12 | `^` | Binary | Wire-level XOR. Element-wise and commutative. |
| 13 | `&` | Binary | Wire-level AND. Element-wise and commutative. |
| 14 | `\|` | Binary | Wire-level OR. Element-wise and commutative. |
| 15 | `?:` | Ternary | Conditional selection. Evaluates the condition before `?`; if true evaluates the middle expression, otherwise the final expression. |

## Validity and Operators

Keep in mind that validity affects every operator. The null state is represented by the wire-type `gnd`. For example:

```weaver
gnd + 5 = gnd
5 + 5 = 10
gnd && true = gnd
true && false = false
gnd & true = gnd
vdd & true = vdd
gnd | true = vdd
```

## Assignment Operators

Assignments may be composed in parallel or choice based on the `,` and `:` operators:

```weaver
var fixed<16,-4> a[3] = [1, 2, 3.5], b[4][2] = [[1, 2], [3, 4], [5, 6], [7, 8]]
var myType x = {1, true, 2.2}
a[2] = 5, b[1][1] = 3 : x.mybool = false
```

- `,` - Parallel assignment (all assignments happen simultaneously)
- `:` - Choice assignment (only one assignment path is taken)

## Wire Operators

For `wire` types, the following operators are available:

- `&` - Wire-level AND
- `|` - Wire-level OR
- `^` - Wire-level XOR
- `~` - Wire-level NOT

## Boolean Operators

For `bool` types and boolean expressions:

- `&&` - Boolean AND (short-circuit)
- `||` - Boolean OR (short-circuit)
- `^^` - Boolean exclusive OR
- `!` - Boolean NOT

## Arithmetic Operators

For numeric types (`int`, `uint`, `fixed`, `ufixed`):

- `+` - Addition
- `-` - Subtraction
- `*` - Multiplication
- `/` - Division
- `%` - Modulo
- `<<` - Left bit shift
- `>>` - Right bit shift

## Comparison Operators

Produce boolean results:

- `==` - Equality
- `~=` - Inequality
- `<` - Less than
- `>` - Greater than
- `<=` - Less than or equal
- `>=` - Greater than or equal
