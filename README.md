# Haystack

Haystack is a collection of tools for the design and verification of
asynchronous circuits. Not all of the tools are complete. The documentation for
each tool may be found in that tool's git repository. Below lists the state of
each tool:

 - [  2%] [**chpsim**](https://github.com/broccolimicro/chpsim/) is a simulator for Communicating Hardware Processes.
 - [ 50%] [**hseenc**](https://github.com/broccolimicro/hseenc/) finds state space conflicts and helps to fix them with state variable assignments.
 - [100%] [**hseplot**](https://github.com/broccolimicro/hseplot/) renders state graphs, petri nets, and signal transition graphs.
 - [100%] [**hsesim**](https://github.com/broccolimicro/hsesim/) is a simulator for Handshaking Expansions which can efficiently elaborate the whole state space of a circuit for input to hseenc.
 - [100%] [**prsim**](https://github.com/broccolimicro/prsim/) is a simulator for Production Rules.
 - [100%] [**bubble**](https://github.com/broccolimicro/bubble/) for bubble reshuffling Production Rules.
 - [ 80%] [**gated**](https://github.com/broccolimicro/gated/) a simple logic synthesis tool to convert arithmetic expressions to boolean logic.
 - [ 20%] [**prsize**](https://github.com/broccolimicro/prsize/) is an automatic sizing program for Production Rules.

## Build

Haystack is built in two phases: libraries then binaries.

```
git submodule update --init --recursive
cd lib
make
cd ../bin
make
```

## Language Definitions
[Quasi-Delay Insensitive Circuits](https://en.wikipedia.org/wiki/Quasi-delay-insensitive_circuit)

### Communicating hardware processes (CHP)

**Communicating hardware processes (CHP)** is a program notation for QDI circuits inspired by Tony Hoare's communicating sequential processes (CSP) and Edsger W. Dijkstra's guarded commands. The syntax is described below in descending precedence.

* **Skip** `skip` does nothing. It simply acts as a placeholder for pass-through conditions.
* **Dataless assignment** `a+` sets the voltage of the node `a` to Vdd while `a-` sets the voltage of `a` to GND.
* **Assignment** `a := e` evaluates the expression `e` then assigns the resulting value to the **variable** `a`.
* **Send** `X!e` evaluates the expression `e` then sends the resulting value across the **channel** `X`. `X!` is a dataless send.
* **Receive** `X?a` waits until there is a valid value on the **channel** `X` then assigns that value to the **variable** `a`. `X?` is a dataless receive.
* **Probe** `#X` returns the value waiting on the **channel** `X` without executing the receive.
* **Simultaneous composition** `S * T` executes the **process fragments** `S` and `T` at the same time.
* **Internal parallel composition** `S, T` executes the **process fragments** `S` and `T` in any order.
* **Sequential composition** `S; T` executes the **process fragments** `S` followed by `T`.
* **Parallel composition** `S || T` executes the **process fragments** `S` and `T` in any order. This is functionally equivalent to internal parallel composition but with lower precedence.
* **Deterministic selection** `[G0 -> S0[]G1 -> S1[]...[]Gn -> Sn]` implements choice in which `G0,G1,...,Gn` are **guards** which are dataless [[boolean expression]]s or data expressions that are implicitly cast using a validity check and `S0,S1,...,Sn` are **process fragments**. Deterministic selection waits until one of the guards evaluates to Vdd, then proceeds to execute the guard's associated **process fragment**. If two guards evaluate to Vdd during the same window of time, an error occurs. `[G]` is shorthand for `[G -> skip]` and simply implements a wait.
* **Non-deterministic selection** `[G0 -> S0:G1 -> S1:...:Gn -> Sn]` is the same as deterministic selection except that more than one guard is allowed to evaluate to Vdd. Only the **process fragment** associated with the first guard to evaluate to Vdd is executed.
* **Repetition** `*[G0 -> S0[]G1 -> S1[]...[]Gn -> Sn]` or `*[G0 -> S0:G1 -> S1:...:Gn -> Sn]` is similar to the associated selection statements except that the action is repeated while any guard evaluates to Vdd. `*[S]` is shorthand for `*[Vdd -> S]` and implements infinite repetition.

### Hand-shaking expansions (HSE)

**Hand-shaking expansions (HSE)** are a subset of CHP in which channel
protocols are expanded into guards and assignments and only dataless operators
are permitted. This is an intermediate representation toward the synthesis of
QDI circuits.

### Production rule set (PRS)
A production rule specifies either the pull-up or pull-down network of a gate
in a QDI circuit and follows the syntax `G -> S` in which `G` is a **guard** as
described above and `S` is one or more **dataless assignments** in parallel as
described above. In states not covered by the guards, it is assumed that the
assigned nodes remain at their previous states. This can be achieved using a
staticizor of either weak or combinational feedback. The most basic example is
the C-element in which the guards do not cover the states where the two inputs
are not the same value.

## Example

**stream_complete.hse**
```
(_Lr+,L.r-; [L.e];
*[_Lr-; L.r+; [~L.e]; _Lr+; L.r-; [L.e]])'1 ||

_Rr+,R.r-,L.e+,v0-,v1+,v2+; [R.e&~L.r];
*[[L.e & R.e & L.r]; _Rr-; R.r+; v2-; [~R.e]; _Rr+; R.r-; v0+; v1-; v2+;
  [L.e & R.e & L.r]; _Rr-; R.r+; L.e-; [~R.e]; _Rr+; R.r-; v1+; v0-;
  [~L.r]; L.e+
 ]||

(R.e+; [~R.r];
*[[R.r]; R.e-; [~R.r]; R.e+])'1
```

![stream](https://github.com/broccolimicro/haystack/assets/8902287/55b7a2dd-d651-4169-a9f9-57c9241a6687)

## License

Licensed by Cornell University under GNU GPL v3.

Written by Ned Bingham.
Copyright © 2020 Cornell University.

Haystack is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Haystack is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License may be found in COPYRIGHT.
Otherwise, see <https://www.gnu.org/licenses/>.

