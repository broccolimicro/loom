# Loom &nbsp; [![Build Status](https://github.com/broccolimicro/loom/actions/workflows/makefile.yml/badge.svg)](https://github.com/broccolimicro/loom/actions/workflows/makefile.yml) [![License](https://img.shields.io/badge/Licence-GNU_GPU_v3-blue.svg)](./LICENSE) [![Documentation](https://img.shields.io/badge/Documentation-green.svg)](https://broccolimicro.github.io/loom)

Loom is a collection of tools for the design and verification of
asynchronous circuits. Not all of the tools are complete.

## Table of Contents
1. [Example](#example)
3. [Build and Install](#build)
2. [Development Status](#status)

<a name="example"></a>
## Example

Write your functional specification.

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

Compile your functional specification to production rules.

```
$ lm wchb1b.hse
require driven, stable, noninterfering
@_12&R.t<1>|_Reset<3>&L.t<3>&R.e<3>->v3-
@_13&~R.t<1>|~_Reset<1>|~L.t<2>&~R.e<2>->v3+
@_12&R.f<1>|_Reset<3>&L.f<3>&R.e<3>->v2-
@_13&~R.f<1>|~_Reset<1>|~L.f<2>&~R.e<2>->v2+
_Reset<3>&v0<3>&L.e'1<3>->v1- {v0}
~_Reset<1>|~v0<1>|~L.e'1<1>->v1+
_Reset<3>&v1<3>&L.e'1<3>->v0- {v1}
~_Reset<1>|~v1<1>|~L.e'1<1>->v0+
R.f'1<1>|R.t'1<1>->R.e'1-
~R.t'1<2>&~R.f'1<2>->R.e'1+
v3<1>->R.t-
~v3<1>->R.t+
v2<1>->R.f-
~v2<1>->R.f+
R.f<1>|R.t<1>->L.e-
~R.t<2>&~R.f<2>->L.e+
v1<1>->L.t'1-
~v1<1>->L.t'1+
v0<1>->L.f'1-
~v0<1>->L.f'1+
Vdd<0.1>->_12- [weak]
~GND<0.1>->_13+ [weak]
```

Or do layout.

```
$ lm wchb1b.hse sky130.py
```

<a name="build"></a>
## Build and Install

Build the executable.
```
sudo apt install ninja-build libqhull-dev libgraphviz-dev
git submodule update --init --recursive
make
```

Install for Linux:
```
make linux
sudo dpkg -i lm-linux.deb
```

Install for Windows:
```
make windows
# this creates lm-windows.zip
```

Install for Mac OS:
```
make macos
# this creates lm-macos
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


