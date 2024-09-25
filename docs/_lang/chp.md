---
title: Communicating Hardware Processes
author: Edward Bingham
date: 2024-09-24
category: Language
layout: post
order: 2
---

**Communicating Hardware Processes (CHP)** is a program notation for QDI
circuits inspired by Tony Hoare's communicating sequential processes
(CSP) and Edsger W. Dijkstra's guarded commands. For example, this is how one
might write an adder with sources for the input channels `A` and `B` and a sink
for the output channel `S`. The environment is placed in a separate isochronic
region.

```
*[ A?a, B?b; S!(a+b) ] ||
(
  *[ A!rand() ] ||
  *[ B!rand() ] ||
  *[ S? ]
)'1
```

The syntax is described below in descending precedence. Spacing is
ignored during parsing.

## Data Types

There are currently only three data types supported by Loom. A **node**
is a single wire in the circuit. A **variable** is an integer type
encoded on a collection of nodes. A **channel** facilitates
communication between processes with a request, acknowledge or request,
enable protocol. Currently, data types are implied by the operators on them.

## Skip

`skip` skips to the next action.

## Assignment

`a+` sets the voltage of the **node** `a` to Vdd while `a-` sets the voltage of
`a` to GND. `b := e` evaluates the expression `e` then assigns the resulting
value to the **variable** `b`.

## Channel Operators

**Send** `X!e` evaluates the expression `e` then sends the resulting value
across the **channel** `X`. `X!` is a dataless send. 

**Receive** `X?a` waits until there is a valid value on the **channel** `X`
then assigns that value to the **variable** `a`. `X?` is a dataless receive.

**Probe** `#X` returns the value waiting on the **channel** `X` without
executing the receive.

## Parallel Composition

`P0 || P1 || ... || Pn` executes the processes `P0`, `P1`, ..., and `Pn` in
parallel. Keep in mind the difference between "parallel" and "concurrent".

* **parallel** two parallel processes can execute in any order or at the same time.
* **concurrent** two concurrent processes can execute in any order.

## Sequential Composition

`P0;P1;...;Pn` executes first executes the process `P0`, then `P1`, then ..., then `Pn`.

## Internal Parallel Composition

`P0,P1,...,Pn` is the same as parallel composition, but with higher precendence.

## Wait

A **guard** is a dataless [[boolean expression] or expression that is
implicitly cast using a validity check on the encoding. The guard evaluates to
`Vdd` when the data is valid and `GND` when it is neutral.

`[G]` waits for the guard `G` to evaluate to `Vdd` before continuing.

## Selection

The selection composition represents choice. The guard `G0`, `G1`, ..., `Gn`
represent the condition of the selection and `P0`, `P1`, ..., `Pn` are the
processes that are executed for each condition.

A deterministic selection statement is represented by the thick bar operator
`[]`. The guards are guaranteed by the user to be mutually exclusive so only
one can ever evaluate to true at any given time. The tooling will throw an
error if a deterministic selection violates this mutual exclusion constraint.
```
[ G0 -> P0
[] G1 -> P1
...
[] Gn -> Pn
]
```

A non-deterministic selection state is represented by the thin bar operator
`:`. An arbiter or in some extreme cases, a synchronizer, will be used to
guarantee the mutual exclusion of the selection.
```
[ G0 -> P0
: G1 -> P1
...
: Gn -> Pn
]
```

Ultimately the selection operator implements the following:
If `G0` do `P0`, if `G1` do `P1`, ... If `Gn` do `Pn`, else wait.

## Repetition

Repetition behaves almost the same as the non-repetitive selection
statement. Think of it like a while loop. While one of the guards
`(G0,G1,...,Gn)` is true, execute the associated process `(P0,P1,...,Pn)`.
Else, exit the loop.

Repetition can either be deterministic as represented by the thick bar.
```
*[  G0 -> P0
 [] G1 -> P1
 ...
 [] Gn -> Pn
 ]
```

Or it can be non-deterministic as represented by the thin bar.
```
*[ G0 -> P0
 : G1 -> P1
 ...
 : Gn -> Pn
 ]
```

`*[S]` is shorthand for `*[Vdd -> S]` and implements infinite repetition.

## Timing Assumption

`{G}` blocks as long as G evaluates to GND. Instabilities on `G` are not
propagated out into the rest of the circuit.

## Isochronic Regions

It has been shown that circuits that are entirely delay insensitive (make no
timing assumptions) aren't that useful. One of the weakest timing assumptions
you can make in order for the circuit class to be turing complete is called the
isochronic fork assumption. In order to implement this assumption, you can
identify isochronic regions. By default every reference to a node is assumed to 
be in the same isochronic region (region 0). However you may change the
isochronic region with the following syntax:

```
P'region
```

Where `P` is a process or node reference and 'region' is an integer representing
the name of the region. For example:

```
x'1+

(x+)'1

(x+,y+)'2

[  G0 -> P0
[] G1 -> P1
]'4

[x'1 & y'2]; z-
```

If there are two processes in two isochronic regions like `(a-;b+;P)'0 || ([b]; a+)'1`,
then during the process `P`, the value of the node `a` will be unknown because the
process on the left knows that `a` was `GND` but that it will change to `Vdd`. It
just doesn't know when. Meanwhile in the process on the right, the value of `a`
will start unknown and transition to `Vdd` after the assignment `a+`.

## Reset Behavior

Because reset behavior can be a complex thing that has a multitude of timing
assumptions and different possible implementations, Loom has a very basic
reset implementation. As long as there isn't any choice to be made, and we
don't enter a repetition, execute transitions and accumulate their affect on
the state into a reset state. Here is an example:

```
R.f-,R.t-,L.e+; [R.e&~L.f&~L.t];
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [~R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1
```

In this WCHB buffer, the reset state for each process is as follows:

 - Process 0: `~R.f&~R.t&L.e&R.e&~L.f&~L.t`
 - Process 1: `~L.f&~L.t&L.e`
 - Process 2: `R.e&~R.f&~R.t`


and the final hse after the reset behavior has been processed looks like this:

```
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(*[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
*[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1
```

## Limited Non-Proper Nesting

Asynchronous circuits are collections of intertwined, sequences of events. The
most basic way to visualize this is called a petri net. Handshaking expansions
represent that structure in a way that is human readable. However, there are
also valid handshaking expansions that are not representable in such a
linguistic format. These are called 'non-properly nested'. For full non-proper
nesting support, use the `astg` format.

In order to support things like initial token buffers where you reset the
circuit in the middle of the HSE, a limited reset-tagging system has been
added. 

```
R.f+,R.t-,L.e+,en+; [R.e&~L.f&~L.t];  
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; en-;
  (
     @ [~R.e]; R.f-,R.t- ||
     [~L.f & ~L.t]; L.e+ @
  ); en+
 ] ||

L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+]
```

In this system, the `@` symbol represents a reset token at the nearest
semicolon for current loop. So if there are multiple loops and you put a reset
token on the inner most loop, that loop will reset there on every iteration of
the outer loop.
