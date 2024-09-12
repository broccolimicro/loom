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

* **CHP Simulator (50%)** Simulate channel actions and multi-bit operations in a control flow language.
	- Still has false positive instability errors. Need to implement quantifier
	  elimination with cylindrical algebraic decomposition to be able to test
		whether expressions are tautilogically true/false to correctly handle guards.
* **Process Decomposition (0%)** Break large processes up into pipeline stages.
* **Handshake Expansion (0%)** Expand channel actions into handshake protocols and multi-bit operations into transitions on wires.
* **Handshake Reshuffling (0%)** Reorder transitions to simplify the state space, simplify implementation, and improve performance.
* **HSE Simulator (100%)** Simulate transitions on wires in a control flow language.
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
* **Automated Sizing (40%)** Size the transistors in a production rule set.
	- This functionality is currently limited to the prsize binary. It has not
	  yet been integrated into the larger synthesis system.
* **PR Simulator (100%)** Digital simulation of the gates and wires as represented by production rules.
* **Netlist Synthesis (0%)** Generate a spice netlist from a production rule set.
* **CHP and HSE Visualization (100%)** Render the petri-nets representing CHP or HSE processes.
* **PRS Visualization (0%)** Render transistor diagrams of the production rule set.
* **Cell Generation (0%)** Break large subcircuits into cells for cell-layout.
* **Cell Layout (80%)** Generate the layouts for those cells.
* **Placement (0%)** Place the cells to start the layout of larger subcircuits.
* **Routing (0%)** Route paths finish the layout of larger subcircuits.

<a name="build"></a>
## Build

Loom is built in two phases: libraries then binaries.

```
sudo apt install ninja-build # for gdstk
git submodule update --init --recursive
cd lib
make
cd ../bin
make
```

To prepare googletest
```
cd googletest
mkdir build
cmake .. -DBUILD_GMOCK=OFF
make
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

<a name="simulation"></a>
### Circuit Simulation

**Usage:** `lm sim [options] <lm-file> [sim-file]`

This will open up the simulator specific to the file format you give it
(`.chp`, `.hse`, `.astg`, `.prs`). The following documentation is for the HSE
simulator.

Run the following command to open up the simulation environment:

```
lm sim wchb1b.hse
```

It will bring you to a prompt that looks like this:

```(hsesim)```

From there you can execute any of the following commands:

`<arg>` specifies a required argument

`(arg=value)` specifies an optional argument with a default value

**General**:
 - `help`, `h`             print this message
 - `seed <n>`            set the random seed for the simulation
 - `source <file>`       source and execute a list of commands from a file
 - `save <file>`         save the sequence of fired transitions to a '.sim' file
 - `load <file>`         load a sequence of transitions from a '.sim' file
 - `clear`, `c`            clear any stored sequence and return to random stepping
 - `quit`, `q`             exit the interactive simulation environment

**Running Simulation**:
 - `tokens`, `t`           list the location and state information of every token
 - `enabled`, `e`          return the list of enabled transitions
 - `fire <i>`, `f<i>`      fire the i'th enabled transition
 - `step (N=1)`, `s(N=1)`  step through N transitions (random unless a sequence is loaded)
 - `reset (i)`, `r(i)`     reset the simulator to the initial marking and re-seed (does not clear)

**Setting/Viewing State**:
 - `set <i> <expr>`      execute a transition as if it were local to the i'th token
 - `set <expr>`          execute a transition as if it were remote to all tokens
 - `force <expr>`        execute a transition as if it were local to all tokens

To begin, view set of possible reset states and then pick one.

```
(hsesim)r
(0) {0 9 15} ~R.f&~R.t&L.e&R.e&~L.f&~L.t&~L.f'1&~L.t'1&L.e'1&R.e'1&~R.f'1&~R.t'1
(hsesim)r0
```

If you are simulating a PRS, then there is no "reset" command like this.
Instead, you have to directly toggle your reset signals. If you have only
`_Reset` in your circuit then that looks like this:
```
(prsim) set _Reset-
(prsim) s100
(prsim) set _Reset+
(prsim) s100
```
If you have both `_Reset` and `Reset`, then you can do the following:
```
(prsim) set _Reset-,Reset+
(prsim) s100
(prsim) set _Reset+,Reset-
(prsim) s100
```

Back to the HSE examples:
```
(hsesim)r
(0) {0 9 15} ~R.f&~R.t&L.e&R.e&~L.f&~L.t&~L.f'1&~L.t'1&L.e'1&R.e'1&~R.f'1&~R.t'1
(hsesim)r0
```

You'll notice the ID's `P0`, `P9`, and `P15`. These refer to specific semicolons or
"places" in the hse. To see what these labels refer to, you may run the
following command outside of the interactive simulation environment. See the
**Visualization** section for more details.

```lm plot -l wchb1b.hse -o wchb1b.png```

Now that you have set the current state to a reset state, you may take a look
at the current state.

```
(hsesim)t
R.f-,R.t-,L.e+,R.e+,L.f-,L.t- {
        (0) P0  L.e+ ; [R.e&L.f->...[]R.e&L.t->...]
}
L.f'1-,L.t'1-,L.e'1+ {
        (1) P9  [L.e'1 -> [1->L.f'1+...[]1->L.t'1+...]
}
R.e'1+,R.f'1-,R.t'1- {
        (2) P15 R.e'1+ ; [R.f'1|R.t'1]
}
```

You can view the list of enabled transitions and fire one.

```
(hsesim)e
(0) T10.0:L.t'1+     (1) T9.0:L.f'1+
(hsesim)f1
0       T9.0    1 -> L.f'1+
(hsesim)e
(0) T1.0:R.f+
(hsesim)f0
1       T1.0    R.e&L.f -> R.f+
(hsesim)e
(0) T4.0:L.e-     (1) T16.0:R.e'1-
(hsesim)f0
2       T4.0    R.f -> L.e-
```

You may also step through the simulation. This has two functions. As you
progress through the simulation, the simulator will remember all of the
transitions that have fired and in what order. If you decide to reset the
simulation after simulating and then step, this will re-execute the remembered
list of transitions. If you step through all of the remembered transitions,
it will continue to step randomly. This will execute the transitions in a 
*random order*.

```
(hsesim)e
(0) T10.0:L.t'1+     (1) T9.0:L.f'1+
(hsesim)f1
0       T9.0    1 -> L.f'1+
(hsesim)e
(0) T1.0:R.f+
(hsesim)f0
1       T1.0    R.e&L.f -> R.f+
(hsesim)e
(0) T4.0:L.e-     (1) T16.0:R.e'1-
(hsesim)f0
2       T4.0    R.f -> L.e-
(hsesim)s6
3       T16.0   R.f'1 -> R.e'1-
4       T12.0   L.f'1&~L.e'1 -> L.f'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
(hsesim)r0
(hsesim)s9
0       T9.0    1 -> L.f'1+
1       T1.0    R.e&L.f -> R.f+
2       T4.0    R.f -> L.e-
3       T16.0   R.f'1 -> R.e'1-
4       T12.0   L.f'1&~L.e'1 -> L.f'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
```

You can save this list of transitions to a file and load it up in a later 
simulation.

```
(hsesim)save test
```

```
(hsesim)load test
(hsesim)r0
(hsesim)s9
0       T9.0    1 -> L.f'1+
1       T1.0    R.e&L.f -> R.f+
2       T4.0    R.f -> L.e-
3       T16.0   R.f'1 -> R.e'1-
4       T12.0   L.f'1&~L.e'1 -> L.f'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
```

If you no longer want to simulate the future steps, you may clear them. This 
will remove any transitions in the list ahead of your current step. However,
transitions before your current step will still be remembered.

```
(hsesim)load test
(hsesim)r0
(hsesim)s9
0       T9.0    1 -> L.f'1+
1       T1.0    R.e&L.f -> R.f+
2       T4.0    R.f -> L.e-
3       T16.0   R.f'1 -> R.e'1-
4       T12.0   L.f'1&~L.e'1 -> L.f'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
(hsesim)r0
(hsesim)s3
0       T9.0    1 -> L.f'1+
1       T1.0    R.e&L.f -> R.f+
2       T4.0    R.f -> L.e-
(hsesim)clear
(hsesim)s6
3       T12.0   L.f'1&~L.e'1 -> L.f'1-
4       T16.0   R.f'1 -> R.e'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
```

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

<a name="syntax"></a>
## Syntax Documentation
 
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

<a name="state"></a>
### Internal Representation of State

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

<a name="regions"></a>
### Isochronic Regions

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

<a name="reset"></a>
### Reset Behavior

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

<a name="nesting"></a>
### Limited Non-Proper Nesting

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
