# Loom

Loom is a collection of tools for the design and verification of
asynchronous circuits. Not all of the tools are complete.

## Table of Contents
1. [Example](#example)
3. [Build](#build)
2. [Development Status](#status)
4. [Language Reference](#reference)

<a name="example"></a>
## Example

<a name="build"></a>
## Build

```
sudo apt install ninja-build libqhull-dev libgraphviz-dev
git submodule update --init --recursive
make
```

To generate test binaries and then run the tests
```
make test
make check
```

<a name="status"></a>
## Development Status

### Synthesis
* **Templating (0%)** parameterize your module specifications.
* **Modules (0%)** be able to break up your circuit into modules and construct larger systems.
* **Process Decomposition (0%)** Break large processes up into pipeline stages.
* **Handshake Expansion (0%)** Expand channel actions into handshake protocols and multi-bit operations into transitions on wires.
* **Handshake Reshuffling (0%)** Reorder transitions to simplify the state space, simplify implementation, and improve performance.
* **State Elaboration (100%)** Explore every state and record the state space.
* **State Variable Insertion (80%)** Deconflict states by inserting transitions. This results in a complete state encoding.
	- The current implementation is not yet able to solve every possible encoding
		problem. There are some transition insertion locations it can't see yet.
		It's also fairly slow on larger circuits since the current implementation
		requires a full re-elaboration of the state space between each new state
		variable insertion. There are fixes for both of these problems planned.
* **Guard Weakening (100%)** Generate production rules that implement that state space.
* **Bubble Reshuffling (90%)** Move inverters off of isochronic forks to protect the isochronic fork assumption. This algorithm is no longer needed as this process is taken care of by state variable insertion now. However, this is left in to help with manual compilation.
	- This doesn't always correctly identify isochronic forks in the production
	  rule set. This is because a literal can show up in a guard, but that
		doesn't mean the guard acknowledges any transitions on that literal.
* **Device Level Sizing (90%)** Size the transistors in a production rule set.
	- Haven't tied in the PN ratio yet.
* **Gate Level Sizing (0%)** Size the gates using logical effort.
* **Netlist Synthesis (99%)** Generate a spice netlist from a production rule set.
	- Need to generate device perimeter and area values among other parameters.
* **Cell Generation (0%)** Break large subcircuits into cells for cell-layout.
* **Cell Layout (80%)** Generate the layouts for those cells.
* **Placement (0%)** Place the cells to start the layout of larger subcircuits.
* **Routing (0%)** Route paths finish the layout of larger subcircuits.

### Simulation

* **CHP Simulator (50%)** Simulate channel actions and multi-bit operations in a control flow language.
	- Still has false positive instability errors. Need to implement quantifier
	  elimination with cylindrical algebraic decomposition to be able to test
		whether expressions are tautilogically true/false to correctly handle guards.
* **HSE Simulator (100%)** Simulate transitions on wires in a control flow language.
* **PR Simulator (100%)** Digital simulation of the gates and wires as represented by production rules.
* **Spice Simulator Tie-in (0%)** Tie a spice simulator to the binary so that you can simulate at any level.
* **Co-simulation of all levels (0%)** Cosimulate the behavioral spec against the wire-level spec, the digital circuit behavior, and the analog circuit behavior.

### Visualization

* **CHP and HSE (100%)** Render the petri-nets representing CHP or HSE processes.
* **State Space (0%)** Render the state space of HSE or PRS.
* **Transistor Networks (0%)** Render transistor diagrams of the production rule set.
* **Waveforms (100%)** Export to VCD for viewing in GTKWave.
* **Event Rule (0%)** Debug your system using an event rule representation instead of waveforms.

<a name="reference"></a>
## Language Reference
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
* **Timing Assumption** `{G}` blocks as long as G evaluates to GND. Instabilities on `G` are not propagated out into the rest of the circuit.

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
are not the same value. Timing assumptions may be optionally specified using
the following syntax: `G0 -> S {G1}`. This will block the production rule from
firing as long as G1 evaluates to GND. Instabilities on `G1` do not propagate
out into the rest of the circuit.

<a name="usage"></a>
## Usage

**Usage:** `lm [options] <command> [arguments]`

**Execute a sub-command:**
 - `sim`           simulate the described circuit
 - `plot`          render the described circuit in various ways

Use `lm help <command>` for more information about a command.

**General Options:**
 - `-h,--help`      display this information
 -    `--version`   display version information
 - `-v,--verbose`   display verbose messages
 - `-d,--debug`     display internal debugging messages
 - `-p,--progress`  display progress information

The following tutorials will use this circuit as the example.

**wchb1b.hse**
```
R.f-,R.t-,L.e+; [R.e&~L.f&~L.t];
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [~R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1
```

<a name="examples"></a>
## Examples

The following is a set of simple examples to get you started.

### WCHB Buffer

```
R.f-,R.t-,L.e+; [R.e&~L.f&~L.t];
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [~R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1
```

### PCHB Split

```
A.f-,A.t-,B.f-,B.t-,SL.e+; [~S.f & ~S.t & ~L.f & ~L.t & A.e & B.e];
*[
  ([  A.e & S.f & L.f -> A.f+
  [] A.e & S.f & L.t -> A.t+
  [] S.t -> skip
  ] ||
  [ B.e & S.t & L.f -> B.f+
  [] B.e & S.t & L.t -> B.t+
  [] S.f -> skip
  ]); SL.e-;
  (
    [~A.e | ~A.f & ~A.t -> A.f-, A.t-] ||
    [~B.e | ~B.f & ~B.t -> B.f-, B.t-]
  ); [~S.f & ~S.t & ~L.f & ~L.t -> SL.e+]
 ]||

(A.e+; [~A.f & ~A.t];
*[[A.t | A.f]; A.e-; [~A.t & ~A.f]; A.e+] ||

B.e+; [~B.f & ~B.t];
*[[B.t | B.f]; B.e-; [~B.t & ~B.f]; B.e+] ||

L.f-,L.t-; [SL.e];
*[[1 -> L.t+ : 1 -> L.f+]; [~SL.e]; (L.t-||L.f-); [SL.e]] ||

S.f-,S.t-; [SL.e];
*[[1 -> S.t+ : 1 -> S.f+]; [~SL.e]; (S.t-||S.f-); [SL.e]])'1
```

### PCHB Adder

```
S.f-,S.t-,Co.f-,Co.t-,ABCi.e+; [S.e&Co.e&~A.f&~A.t&~B.f&~B.t&~Ci.f&~Ci.t];
*[
    (
        [   S.e & (A.t & B.f & Ci.f | A.f & B.t & Ci.f | A.f & B.f & Ci.t | A.t & B.t & Ci.t) -> S.t+
        []  S.e & (A.t & B.t & Ci.f | A.t & B.f & Ci.t | A.f & B.t & Ci.t | A.f & B.f & Ci.f) -> S.f+
        ] ||
        [   Co.e & (A.t & B.t & Ci.f | A.t & B.f & Ci.t | A.f & B.t & Ci.t | A.t & B.t & Ci.t) -> Co.t+
        []  Co.e & (A.t & B.f & Ci.f | A.f & B.t & Ci.f | A.f & B.f & Ci.t | A.f & B.f & Ci.f) -> Co.f+
        ]
    ); ABCi.e-;
    (
        [~S.e -> S.t-,S.f-] ||
        [~Co.e -> Co.t-,Co.f-]
    ); [~A.t & ~A.f & ~B.t & ~B.f & ~Ci.t & ~Ci.f];
    ABCi.e+
] ||

(S.e+; [~S.f&~S.t]; *[[S.t | S.f]; S.e-; [~S.t & ~S.f]; S.e+] ||

Co.e+; [~Co.f&~Co.t]; *[[Co.t | Co.f]; Co.e-; [~Co.t & ~Co.f]; Co.e+] ||

A.f-,A.t-; [ABCi.e];
*[[ 1 -> A.t+
  : 1 -> A.f+
  ]; [~ABCi.e]; A.t-,A.f-; [ABCi.e]
 ] ||

B.f-,B.t-; [ABCi.e];
*[[ 1 -> B.t+
  : 1 -> B.f+
  ]; [~ABCi.e]; B.t-,B.f-; [ABCi.e]
 ] ||

Ci.f-,Ci.t-; [ABCi.e];
*[[ 1 -> Ci.t+
  : 1 -> Ci.f+
  ]; [~ABCi.e]; Ci.t-,Ci.f-; [ABCi.e]
 ])'1
```

