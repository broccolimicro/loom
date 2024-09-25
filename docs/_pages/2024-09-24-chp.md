---
title: Communicating Hardware Processes
author: Edward Bingham
date: 2024-09-24
category: Language
layout: post
---

# Communicating Hardware Processes (CHP)
 
HSE stands for Handshaking Expansions. It is a step in between Communicating 
Hardware Processes (CHP) and Production Rules (PRs). Its a control flow 
language where all actions are limited to 1 bit boolean. There are only a few 
basic syntax structures most of which are composition operators. Spacing is
ignored during parsing. The following list explains what each syntax does.
Composition operators are listed by precedence from weakest to strongest.

-------------------------------------------------------------------------------

```
skip
```

This is just a no-op.

-------------------------------------------------------------------------------

```
x-
x+
```

Every variable in HSE represents a node in the circuit. `x-` sets the voltage
on that node to GND and `x+` sets the voltage on that node to VDD.

-------------------------------------------------------------------------------

```
P0 || P1 || ... || Pn
```

Parallel composition: do `P0`, `P1`, ..., and `Pn` in any interleaving.

-------------------------------------------------------------------------------

```
P0;P1;...;Pn
```

Sequential composition: do `P0`, then `P1`, then ..., then `Pn`.

-------------------------------------------------------------------------------

```
P0,P1,...,Pn
```

Internal parallel composition is the same as parallel composition.

-------------------------------------------------------------------------------

```
[ G0 -> P0
[] G1 -> P1
...
[] Gn -> Pn
]

[ G0 -> P0
: G1 -> P1
...
: Gn -> Pn
]

[G]
```

The selection composition represents choice. `G0`, `G1`, ..., `Gn` are called guards. 
They are boolean expressions that represent the condition of the selection and 
`P0`, `P1`, ..., `Pn` are the processes that are executed for each condition.

A selection statement can either be deterministic as represented by the thick
bar operator `[]` or non-deterministic as represented by the thin bar operator 
`:`. If it is a deterministic selection, then the guards are guaranteed by the
user to be mutually exclusive so only one can ever evaluate to true at any
given time. Meanwhile if it is non-deterministic, then an arbiter or in
some extreme cases, a synchronizer, must be used to guarantee the mutual
exclusion of the selection. Ultimately the selection operator implements the
following:

If `G0` do `P0`, if `G1` do `P1`, ... If `Gn` do `Pn`, else wait.

If there is no process specified as in the third example, then the process
is just a `skip`. This is shorthand for a wait until operation, also
known simply as a 'guard'.

-------------------------------------------------------------------------------

```
*[ G0 -> P0
[] G1 -> P1
...
[] Gn -> Pn
]

*[ G0 -> P0
: G1 -> P1
...
: Gn -> Pn
]

*[P]
```

Repetitive selection behaves almost the same as the non-repetitive selection
statement. Think of it like a while loop.

While one of the guards `(G0,G1,...,Gn)` is true, execute the associated process
`(P0,P1,...,Pn)`. Else, exit the loop.

If the guard is not specified, then the guard is assumed to be '1'. This
is shorthand for a loop that will never exit.

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
then during the process `P`, the value of the node `a` will be `-` because the
process on the left knows that `a` was `0` but that it will change to `1`. It
just doesn't know when. Meanwhile in the process on the right, the value of `a`
will start at `-` and transition to `1` after the assignment `a+`.

## Reset Behavior

Because reset behavior can be a complex thing that has a multitude of timing
assumptions and different possible implementations, hsesim has a very basic
reset implementation. It goes as follows: as long as there isn't any choice to
be made, and we don't enter a loop, execute transitions and accumulate their
affect on the state into a reset state. Here is an example:

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
