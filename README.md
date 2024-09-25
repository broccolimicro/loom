# Loom

Loom is a collection of tools for the design and verification of
asynchronous circuits. Not all of the tools are complete.

## Table of Contents
1. [Status of Tools](#status)
2. [Build](#build)
3. [Language Reference](#reference)
4. [Usage](#usage)
	1. [Circuit Synthesis](#synthesis)
	2. [Circuit Simulation](#simulation)
	3. [Visualization](#visualization)
5. [Examples](#examples)
6. [Syntax Documentation](#syntax)
	1. [Internal Representation of State](#state)
	2. [Reset Behavior](#reset)
	3. [Limited Non-Proper Nesting](#nesting)

<a name="status"></a>
## Status of Tools

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

<a name="synthesis"></a>
### Circuit Synthesis

Synthesize the production rules that implement the behavioral description.

**Usage:** `lm [options] <file>`

**Options:**
 - `--no-cmos`      uses bubble reshuffling instead of state variables to make the production rules cmos implementable, combine with `--rules` to avoid cmos implementability altogether.
 - `--all`          save all intermediate stages
 - `-o,--out`       set the filename prefix for the saved intermediate stages
 - `-g,--graph`     save the elaborated astg
 - `-c,--conflicts` print the conflicts to stdout
 - `-e,--encode`    save the complete state encoded astg
 - `-r,--rules`     save the synthesized production rules
 - `-b,--bubble`    save the bubble reshuffled production rules
 - `-s,--size`      save the sized production rules

**Supported file formats:**
 - `*.chp`          communicating hardware processes
 - `*.hse`          handshaking expansions
 - `*.prs`          production rules
 - `*.astg`         asynchronous signal transition graph
 - `*.spi`          spice netlist

If you just want to generate the final circuit, simply run the following command:

```
lm wchb1b.hse
```

This is the generated production rule set.
```
v0->R.f-
~v0->R.f+
v1->R.t-
~v1->R.t+
R.t|R.f->L.e-
~R.f&~R.t->L.e+
v2->L.f'1-
~v2->L.f'1+
v3->L.t'1-
~v3->L.t'1+
R.t'1|R.f'1->R.e'1-
~R.f'1&~R.t'1->R.e'1+
R.e&L.f&_Reset->v0-
~R.e&~L.f|~_Reset->v0+
R.e&L.t&_Reset->v1-
~R.e&~L.t|~_Reset->v1+
L.e'1&v3&_Reset->v2- {v3}
~L.e'1|~v3|~_Reset->v2+
L.e'1&v2&_Reset->v3- {v2}
~L.e'1|~v2|~_Reset->v3+
```

This will print the production rules out on the console. If you want to see all of the intermediate steps, run this:

```
lm --all --out wchb1b wchb1b.hse
```

This will save the elaborated state space to `wchb1b_predicate.astg`, save the
complete state coded state space to `wchb1b_complete.astg`, and the final
production rules in `wchb1b.prs`. It will also display the set of state
conflicts found on the original specification to the console.

For example, this is what `wchb1b_complete.astg` looks like:
```
.model hse
.internal R.f R.t L.e R.e L.f L.t L.f'1 L.t'1 L.e'1 R.e'1 R.f'1 R.t'1 v0 v1 v2 v3
.predicate
p0 ~R.f&~R.t&L.e&~L.t&v0&v1|~R.f&~R.t&L.e&~L.f&L.t&v0&v1
p1 R.f&~R.t&~L.e&~L.t&~v0&v1
p2 R.f&~R.t&L.e&L.f&~L.t&~v0&v1|~R.f&R.t&L.e&~L.f&L.t&v0&~v1
p3 ~R.f&~R.t&~L.e&~L.f&~L.t&v0&v1|~R.f&R.t&~L.e&~R.e&~L.f&~L.t&v0&v1|~R.f&R.t&~L.e&~L.f&v0&~v1
p4 ~R.f&~R.t&~L.e&~L.f&~L.t&v0&v1|R.f&~R.t&~L.e&~L.t&~v0&v1|R.f&~R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p5 L.f'1&~L.t'1&~v2&v3
p6 ~L.f'1&L.t'1&v2&~v3|~L.f'1&~L.t'1&v2&v3|~L.f'1&L.t'1&~L.e'1&v2&v3
p7 ~L.f'1&~L.t'1&v2&v3|L.f'1&~L.t'1&~L.e'1&v2&v3|L.f'1&~L.t'1&~v2&v3
p8 R.e'1&~R.f'1|R.e'1&R.f'1&~R.t'1
p9 ~R.e'1&~R.f'1|~R.e'1&R.f'1&~R.t'1
p10 ~R.f&R.t&~L.e&~L.f&v0&~v1
p11 ~L.f'1&L.t'1&v2&~v3
p12 ~R.f&~R.t&L.e&R.e&L.f&~L.t&~v0&v1
p13 R.f&~R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p14 ~R.f&~R.t&L.e&R.e&~L.f&L.t&v0&~v1
p15 ~R.f&R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p16 ~L.f'1&~L.t'1&L.e'1&~v2&v3
p17 L.f'1&~L.t'1&~L.e'1&v2&v3
p18 ~L.f'1&~L.t'1&L.e'1&v2&~v3
p19 ~L.f'1&L.t'1&~L.e'1&v2&v3
.effective
p0 ~R.f&~R.t&L.e&~L.f&~L.t&v0&v1|~R.f&~R.t&L.e&~R.e&L.f&~L.t&v0&v1|~R.f&~R.t&L.e&~R.e&~L.f&L.t&v0&v1
p1 R.f&~R.t&~L.e&R.e&~L.t&~v0&v1|R.f&~R.t&~L.e&~R.e&L.f&~L.t&~v0&v1
p3 ~R.f&R.t&~L.e&~L.f&v0&~v1|~R.f&R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p4 R.f&~R.t&~L.e&~L.t&~v0&v1|R.f&~R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p5 L.f'1&~L.t'1&L.e'1&~v2&v3
p6 ~L.f'1&L.t'1&v2&~v3|~L.f'1&~L.e'1&v2&v3
p7 L.f'1&~L.t'1&~v2&v3|~L.t'1&~L.e'1&v2&v3
p8 R.e'1&~R.f'1&~R.t'1
p9 ~R.e'1&R.f'1&~R.t'1|~R.e'1&~R.f'1&R.t'1
p10 ~R.f&R.t&~L.e&~L.f&L.t&v0&~v1|~R.f&R.t&~L.e&R.e&~L.f&~L.t&v0&~v1
p11 ~L.f'1&L.t'1&L.e'1&v2&~v3
.graph
1->R.f+/0 p2
1->R.t+/1 p2
1->L.e-/2 p1 p10
1->R.f-/3 p3
1->R.t-/4 p4
1->L.e+/5 p0
1->L.f'1+/6 p5 p11
1->L.t'1+/7 p5 p11
1->L.f'1-/8 p6
1->L.t'1-/9 p7
R.f'1|R.t'1->R.e'1-/10 p9
~R.f'1&~R.t'1->R.e'1+/11 p8
R.e&L.f->v0-/12 p12
~R.e&~L.f&~L.t->v0+/13 p13
R.e&L.t->v1-/14 p14
~R.e&~L.f&~L.t->v1+/15 p15
L.e'1->v2-/16 p16
~L.e'1->v2+/17 p17
L.e'1->v3-/18 p18
~L.e'1->v3+/19 p19
p0 R.e&L.f->v0-/12 R.e&L.t->v1-/14
p1 ~R.e&~L.f&~L.t->v0+/13
p2 1->L.e-/2
p3 1->L.e+/5
p4 1->L.e+/5
p5 ~L.e'1->v2+/17
p6 L.e'1->v2-/16 L.e'1->v3-/18
p7 L.e'1->v2-/16 L.e'1->v3-/18
p8 R.f'1|R.t'1->R.e'1-/10
p9 ~R.f'1&~R.t'1->R.e'1+/11
p10 ~R.e&~L.f&~L.t->v1+/15
p11 ~L.e'1->v3+/19
p12 1->R.f+/0
p13 1->R.f-/3
p14 1->R.t+/1
p15 1->R.t-/4
p16 1->L.f'1+/6
p17 1->L.f'1-/8
p18 1->L.t'1+/7
p19 1->L.t'1-/9
.arbiter {p6 p7}
.marking {[R.f-,R.t-,L.e+,R.e+,L.f-,L.t-,L.f'1-,L.t'1-,L.e'1+,R.e'1+,R.f'1-,R.t'1-,v0+,v1+,v2+,v3+] p0 p8 p7 p6}
.end
```

This command also accepts `*.spi` files for automated cell layout.

```
lm nand.spi sky130.py
klayout nand.gds &
```

See [sky130.py](https://github.com/broccolimicro/floret/blob/main/tech/sky130.py) 

<a name="visualization"></a>
### Visualization

Create visual representations of the circuit or behavior.

**Usage:** `lm plot [options] file...`

**Options:**
 - `-o`              Specify the output file name, formats other than 'dot' are passed onto graphviz dot for rendering
 - `-l,--labels`     Show the IDs for each place, transition, and arc
 - `-lr,--leftright` Render the graph from left to right
 - `-e,--effective`  Show the effective encoding of each place
 - `-p,--predicate`  Show the predicate of each place
 - `-r,--raw`        Do not post-process the graph
 - `-s,--sync`       Render half synchronization actions

Use the following command to show the elaborated state space from the generated astg.

```
lm plot -p wchb1b_predicate.astg -o wchb1b.png
```

Use this command to show the labels associated with every place, transition, and arc.

```
lm plot -l wchb1b.hse -o wchb1b.png
```

Use the following command to show the elaborated state space of the complete state coding.

```
lm plot -p wchb1b_complete.astg -o wchb1b.png
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

