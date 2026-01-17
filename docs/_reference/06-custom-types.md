---
title: Custom Types
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

Types specify how data should be grouped into a bus. They allow you to organize related fields together.

## Type Definition Syntax

```weaver
type typeName {
    type1 field1
    type2 field2, field3
    type3 field4
}
```

## Example: Opcode Type

```weaver
type opcode {
    int<4> fn, rs, rt, rd
}

var opcode op = {3, 0, 1, 2}
if op.fn == 0 {
    rf[op.rd] = rf[op.rs] + rf[op.rt]
}
```

## Type Instantiation

Types are instantiated using struct-like literal syntax:

```weaver
var opcode op = {3, 0, 1, 2}
var myType x = {1, true, 2.2}
```

## Field Access

Fields are accessed using the dot operator:

```weaver
op.fn    // Access the fn field
op.rs    // Access the rs field
x.mybool // Access the mybool field
```

## Use Cases

Custom types are useful for:
- Grouping related signals into buses
- Organizing instruction formats
- Creating structured data types for communication
- Improving code readability and maintainability
