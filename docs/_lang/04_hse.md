---
title: Hand Shaking Expansions
author: Edward Bingham
date: 2024-09-24
category: Language
layout: post
---

**Hand Shaking Expansions (HSE)** are a subset of CHP in which channel
protocols are expanded into guards and assignments and only dataless
operators are permitted. This is an intermediate representation toward
the synthesis of QDI circuits. A process in HSE should at most represent a
single pipeline stage. For example, this is how one might write a 1-bit adder
with sources for the three input channels `A`, `B`, and `Ci` and sinks for the
two output channels `S` and `Co`. The environment is placed in a separate
isochronic region.

```
S.f-,S.t-,Co.f-,Co.t-,ABCi.e+; [S.e&Co.e&~A.f&~A.t&~B.f&~B.t&~Ci.f&~Ci.t];
*[
  (
    [  S.e & (A.t & B.f & Ci.f | A.f & B.t & Ci.f | A.f & B.f & Ci.t | A.t & B.t & Ci.t) -> S.t+ 
    []  S.e & (A.t & B.t & Ci.f | A.t & B.f & Ci.t | A.f & B.t & Ci.t | A.f & B.f & Ci.f) -> S.f+
    ] ||
    [   Co.e & (A.t & B.t & Ci.f | A.t & B.f & Ci.t | A.f & B.t & Ci.t | A.t & B.t & Ci.t) -> Co.t+
    []  Co.e & (A.t & B.f & Ci.f | A.f & B.t & Ci.f | A.f & B.f & Ci.t | A.f & B.f & Ci.f) -> Co.f+
    ]
  ); ABCi.e-; [~A.t & ~A.f & ~B.t & ~B.f & ~Ci.t & ~Ci.f];
  (
    [~S.e -> S.t-,S.f-] ||
    [~Co.e -> Co.t-,Co.f-]
  );
  ABCi.e+
] || 

(S.e+; [~S.f&~S.t]; *[[S.t | S.f]; S.e-; [~S.t & ~S.f]; S.e+] ||

Co.e+; [~Co.f&~Co.t]; *[[Co.t | Co.f]; Co.e-; [~Co.t & ~Co.f]; Co.e+] ||

A.f-,A.t-; [ABCi.e]; 
*[[1->A.t+:1->A.f+]; [~ABCi.e]; A.t-,A.f-; [ABCi.e]] ||

B.f-,B.t-; [ABCi.e];
*[[1->B.t+:1->B.f+]; [~ABCi.e]; B.t-,B.f-; [ABCi.e]] ||

Ci.f-,Ci.t-; [ABCi.e];
*[[1->Ci.t+:1->Ci.f+]; [~ABCi.e]; Ci.t-,Ci.f-; [ABCi.e]])'1
```

## Internal Representation of State

The state of a node is represented by four basic values `(-,0,1,X)`. `-` means
that the node is stable at either GND or VDD but the process doesn't know
which. `0` means the node is stable at GND. `1` means the node is stable at
VDD. And `X` means the node is unstable or metastable (some unknown value
between GND and VDD).

`a+` drives the node `a` to `1` and `a-` drives the node `a` to `0`. If two 
assignments interfere as in `a+,a-`, then the value of the node `a` will
be driven to `X`. If an assignment is unstable as in `[a];b+||a-`, then the
node `b` will be drive to `X`.  

If there is a selection like `[1->a+:1->a-];b-`, then at the semicolon before
`b-`, the value of the node `a` will be `-`. (Yes I know this example does not
represent a real circuit because you don't know when the selection has
completed).

If a node has a value of `X`, then it will propagate as expected. For example
in `b-; [a]; b+` if the node `a` is unstable, then after `b+`, the node `b` will
also be unstable.
