---
title: Production Rule Set
author: Edward Bingham
date: 2024-09-24
category: Language
layout: post
---

A production rule specifies either the pull-up or pull-down network of a gate
in a QDI circuit. Many implicit inverters can be automatically added, feedback
for stateholding or non-combinational gates is automatically generated, and
sizing is automatically computed. For example a simple production rule set for
a dataless buffer with a source for the input channel `L` and a sink for the
output channel `R` would look like this.

```
require driven, stable, noninterfering
_Reset & R.e & L.r -> R.r+
R.r -> L.e-
~_Reset | ~R.e & ~L.r -> R.r-
~R.r -> L.e+

L.e -> L.r+
~L.e -> L.r-

R.r -> R.e-
~R.r -> R.e+
```

## Production Rule

The syntax `G -> S` has a **guard** `G` and **dataless assignment** `S`. In
states not covered by the guards, it is assumed that the assigned nodes remain
at their previous states. This can be achieved using a keeper using either weak
or combinational feedback. The most basic example is the C-element in which the
guards do not cover the states where the two inputs are not the same value.

## Require and Assume

At the top of the production rule file, you may specify a comma
separated list of global behavioral constraints and assumptions. The
following are supported:

```
assume nobackflow, static
require driven, stable, noninterfering, adiabatic
```

* **assume nobackflow** - NMOS transistors no longer drives a weak 1 and PMOS
transistors no longer drive a weak 0.
* **assume static** - Undriven nodes keep their value instead of drifting to
`X`. The strength still drifts to undriven.
* **require driven** - An error is thrown when a floating node is detected.
* **require stable** - An error is thrown when a glitch is detected.
* **require noninterfering** - An error is thrown when a short is detected.
* **require adiabatic** - An error is thrown when a transition on the gate of a
transistor connects source to drain when they are not already the same
voltage.

## Pass Transistor Logic

The source node of guard may be specified using
`@source & G -> S`. This allows you to efficiently write pass transistor logic
or shared gate networks.

## Anonymous Nodes

Anonymous nodes must follow the naming pattern `_0`
where `0` can be any integer number.

## Flags

Behaviors for specific production rules can be tuned using various
flags as follows `G -> S [flag...]`. The following flags are supported.

* **keep** - This net holds its value when it's not driven. We're assuming a
keeper on this node.
* **weak** - The transistors in this production rule are weak and can be
overridden by any other non-weak production rule. Use this for keepers.
* **force** - The transistors in this production rule are very strong and
cannot be overridden at all. This also overrides normal production rules. Use
this for power rails.
* **pass** - The production rule allows bi-directional current flow from source
to drain or from drain to source.
* **after=0** - Specify a maximum delay for this production rule where `0` can
be any integer representing some number of pico-seconds. 

## Timing Assumptions

Timing assumptions may be optionally specified using the following syntax:
`G0 -> S {G1}`. This will block the production rule from firing as long as G1
evaluates to GND. Instabilities on `G1` do not propagate out into the rest of
the circuit.

## Examples

With weak feedback, shared gate networks,
and anonymous nodes, and sizing, the above turns into this:

```
require driven, stable, noninterfering
@_3&R.r<1>|_Reset<3>&L.r<3>&R.e<3>->v1-
~v1<1>->R.r+
R.r<1>->L.e-
@_4&~R.r<1>|~_Reset<1>|~L.r<2>&~R.e<2>->v1+
v1<1>->R.r-
~R.r<1>->L.e+

L.e'1<1>->v0-
~v0<1>->L.r'1+
~L.e'1<1>->v0+
v0<1>->L.r'1-

R.r'1<1>->R.e'1-
~R.r'1<1>->R.e'1+

Vdd<0.1>->_3- [weak]
~GND<0.1>->_4+ [weak]
```
