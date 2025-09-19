# Loom &nbsp; [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.13992088.svg)](https://doi.org/10.5281/zenodo.13992088) [![Documentation](https://img.shields.io/badge/Documentation-blue.svg)](https://broccolimicro.github.io/loom) [![Tests](https://github.com/broccolimicro/loom/actions/workflows/test.yml/badge.svg)](https://github.com/broccolimicro/loom/actions/workflows/test.yml) [![Release](https://github.com/broccolimicro/loom/actions/workflows/release.yml/badge.svg)](https://github.com/broccolimicro/loom/actions/workflows/release.yml)

Loom is a circuit compiler for a language called Weaver. The goal is to make it
easy to design complex computer architectures that are both performant and
energy efficient.

![LoomArchitecture](https://github.com/user-attachments/assets/5a34cbb4-41bf-492b-bc20-511102e78537)

## Table of Contents
1. [Install](#install)
1. [Function Example](#function_example)
1. [Protocol Example](#protocol_example)
3. [Build From Source](#build)
2. [Development Status](#status)

<a name="install"></a>
## Install

This install script downloads the appropriate binaries for your system and places them in `/usr/local` (or `C:\Program Files (x86)\Loom` on windows)
```
curl -sL https://raw.githubusercontent.com/broccolimicro/loom/refs/heads/main/install.sh | sudo bash
```

## Examples

Loom has two separate backends. One compiles from a data-level specification to
verilog and the other compiles from a wire-level specification to QDI circuits
in a spice netlist. On the frontend, these two backends are accessible through
different "dialects". Below we show two different examples with these flows.
It is our long-term goal to unify these backends to be able to select the
timing model when compiling. See [Development Status](#status) for more
information.

<a name="function_example"></a>
### Function Example

Start by creating your project directory and initializing your project.
```
$ mkdir mychip
$ cd mychip
$ lm mod init mychip
```

Then add **top.wv** with the following.
```
func add(chan A, B; chan S) {
	while {
		S.send(A.recv() + B.recv())
	}
}

func split(chan C; chan L; chan A, B) {
	while {
		if C.probe() == 0 {
			A.send(L.recv()) and C.recv()
		} or if C.probe() == 1 {
			B.send(L.recv()) and C.recv()
		}
	}
}
```

And then build your chip, creating **build/rtl/split.v** and **build/rtl/add.v**.
```
$ lm build
```

<a name="protocol_example"></a>
### Protocol Example

Now we're going to edit **top.wv** putting the following at the top.
```
import "buffer"
```

Then we can create a new file **buffer.wv** with the following.
```
func buffer(chan L, R) {
	while {
		R.send(L.recv())
	}
}

proto wchb1b() : buffer {
	region 1 {
		L.f- and L.t-
		await L.e
		while {
			L.f+ xor L.t+
			await ~L.e
			L.f- and L.t-
			await L.e
		}
	} and {
		L.e+ and R.f- and R.t-
		await R.e & ~L.f & ~L.t
		while {
			await R.e & L.f {
				R.f+
			} or await R.e & L.t {
				R.t+
			}
			L.e-
			await ~R.e & ~L.f & ~L.t
			R.f- and R.t-
			L.e+
		}
	} and region 1 {
		R.e+
		await ~R.f & ~R.t
		while {
			await R.f | R.t
			R.e-
			await ~R.f & ~R.t
			R.e+
		}
	}
}
```

You can list out the modules with `lm mod show`.
```
$ lm mod show
buffer:buffer(chan,chan)
buffer:wchb1b()
top:add(chan,chan,chan)
top:split(chan,chan,chan,chan)
```

We can build just the wchb1b, producing `build/ckt/wchb1b.prs`.
```
$ lm build "buffer:wchb1b()"
$ cat build/ckt/wchb1b.prs
require driven, stable, noninterfering
_Reset&L.t&R.e->v3- [keep]
~_Reset|~L.t&~R.e->v3+ [keep]
_Reset&L.f&R.e->v2- [keep]
~_Reset|~L.f&~R.e->v2+ [keep]
_Reset&v0&L.e'1->v1- {v0}
~_Reset|~v0|~L.e'1->v1+
_Reset&v1&L.e'1->v0- {v1}
~_Reset|~v1|~L.e'1->v0+
R.f'1|R.t'1->R.e'1-
~R.t'1&~R.f'1->R.e'1+
v3->R.t-
~v3->R.t+
v2->R.f-
~v2->R.f+
R.f|R.t->L.e-
~R.t&~R.f->L.e+
v1->L.t'1-
~v1->L.t'1+
v0->L.f'1-
~v0->L.f'1+
```

And you can keep doing that with each build result.
```
$ lm build build/ckt/wchb1b.prs
```

Or we can build the whole project again, synthesizing a layout for the `wchb1b`.
```
$ lm build
$ find build/
build/
build/gds
build/gds/wchb1b.gds
build/spi
build/spi/wchb1b.spi
build/rtl
build/rtl/add.v
build/rtl/split.v
build/rtl/buffer.v
build/ckt
build/ckt/wchb1b.prs
```

```
$ klayout build/gds/wchb1b.gds
```

![wchb1b](https://github.com/user-attachments/assets/726b96d3-6ebe-49f3-8830-6ac17941b804)

<a name="build"></a>
## Build and Install

### Linux

Install dependencies
```
sudo apt install ninja-build libqhull-dev libgraphviz-dev opencl-headers ocl-icd-opencl-dev mesa-opencl-icd 
```

Clone the repository
```
git clone https://github.com/broccolimicro/loom.git
cd loom
git submodule update --init --recursive
```

Build
```
make linux
```

Install
```
sudo dpkg -i lm-linux.deb
```

### Windows

Install dependencies
```
pacman -Syu --noconfirm
pacman -S --needed --noconfirm base-devel
pacman -S --needed --noconfirm msys2-runtime-devel
pacman -S --needed --noconfirm mingw-w64-x86_64-toolchain
pacman -S --needed --noconfirm mingw-w64-x86_64-dlfcn
pacman -S --needed --noconfirm mingw-w64-x86_64-cmake
pacman -S --needed --noconfirm mingw-w64-x86_64-ninja
pacman -S --needed --noconfirm mingw-w64-x86_64-qhull
pacman -S --needed --noconfirm mingw-w64-x86_64-zlib
pacman -S --needed --noconfirm mingw-w64-x86_64-graphviz
pacman -S --needed --noconfirm mingw-w64-x86_64-opencl-headers
pacman -S --needed --noconfirm mingw-w64-x86_64-opencl-clhpp
pacman -S --needed --noconfirm mingw-w64-x86_64-opencl-icd
pacman -S --needed --noconfirm zip
pacman -S --needed --noconfirm curl
```

Clone the repository
```
git clone https://github.com/broccolimicro/loom.git
cd loom
git submodule update --init --recursive
```

Build
```
make windows
```

Install
```
unzip lm-windows.zip -d "C:\\Program Files (x86)"
export PATH="C:\\Program Files (x86)\\Loom\\bin:$PATH"
```

### Mac OS

Install dependencies
```
brew install cmake ninja qhull graphviz curl opencl-headers opencl-clhpp-headers ocl-icd
```

Clone the repository
```
git clone https://github.com/broccolimicro/loom.git
cd loom
git submodule update --init --recursive
```

Build
```
make macos
```

Install
```
tar -xzvf lm-macos.tar.gz
cp lm-macos/bin/lm /usr/local/bin
cp lm-macos/share/tech /usr/local/share
chmod +x /usr/local/bin/lm
chown -R root:staff /usr/local/share/tech
chmod -R ug+rw /usr/local/share/tech 
```

### Run Tests

To generate test binaries and then run the tests
```
make test
make check
```

<a name="status"></a>
## Development Status (Sept 17, 2025)

### Synthesis
* **Templating (0%)** parameterize your module specifications.
* **Modules (60%)** be able to break up your circuit into modules and construct larger systems.
* **Process Decomposition (0%)** Break large processes up into pipeline stages.
* **Fold (0%)** fold multiple assignments to get a dynamic single assignment form.
* **Flatten (20%)** flatten multiple conditions into a single stage-wide condition.
* **Map to Flow Templates (95%)** map that pipeline stage onto a flow template.
* **Synthesize Verilog from Flow (95%)** generate verilog logic from that flow template.
* **Synthesize PRS from Flow (0%)** generate QDI logic from that flow template.
* **Arithmetic Simplification (70%)** reduce arithmetic expressions to simpler forms, preserving semantics.
* **Handshake Expansion (20%)** Expand channel actions into handshake protocols and multi-bit operations into transitions on wires.
* **Handshake Reshuffling (0%)** Reorder transitions to simplify the state space, simplify implementation, and improve performance.
* **State Elaboration (90%)** Explore every state and record the state space.
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
* **Device Level Sizing (100%)** Size the transistors in a production rule set.
* **Gate Level Sizing (0%)** Size the gates using logical effort.
* **Netlist Synthesis (100%)** Generate a spice netlist from a production rule set.
* **Cell Generation (100%)** Break large subcircuits into cells for cell-layout.
* **Cell Layout (96%)** Generate the layouts for those cells.
* **Placement (20%)** Place the cells to start the layout of larger subcircuits.
* **Routing (0%)** Route paths finish the layout of larger subcircuits.

### Simulation

* **CHP Simulator (50%)** Simulate channel actions and multi-bit operations in a control flow language.
	- Still has false positive instability errors. Need to implement quantifier
	  elimination with cylindrical algebraic decomposition to be able to test
		whether expressions are tautilogically true/false to correctly handle guards.
* **HSE Simulator (100%)** Simulate transitions on wires in a control flow language.
* **PR Simulator (90%)** Digital simulation of the gates and wires as represented by production rules.
	- There's a bug associated with timing assumptions in this simulator.
* **Spice Simulator Tie-in (0%)** Tie a spice simulator to the binary so that you can simulate at any level.
* **Co-simulation of all levels (0%)** Cosimulate the behavioral spec against the wire-level spec, the digital circuit behavior, and the analog circuit behavior.

### Visualization

* **CHP and HSE (100%)** Render the petri-nets representing CHP or HSE processes.
* **Flow (80%)** Render flow templates.
* **State Space (0%)** Render the state space of HSE or PRS.
* **Transistor Networks (0%)** Render transistor diagrams of the production rule set.
* **Waveforms (100%)** Export to VCD for viewing in GTKWave.
* **Event Rule (0%)** Debug your system using an event rule representation instead of waveforms.

## Design Invariant Goals

* The higher level language cannot opinionate the implementation of a dialect.
* There must be separate submodules for:
	- **parser** - the abstract syntax tree for a particular language and the code to
	  fill the tree based on text file input.
	- **interpreter** - map the abstract syntax tree to the base dialect
	- **base** - the actual implementation of a dialect
	- **binder** - used by the interpreter to handle translation between higher level
	  language variables and types and the variables and types the base dialect
    knows how to handle.
* The higher level language cannot know anything about a given dialect except for the code in the **binder**.
* The **parser** must work independently of any other submodule
* The **interpreter** must work independently of any other submodule other than the **base** and the **parser**
* The **base** must work independently of any other submodule
* The **binder** must work independently of any other submodule other than the **base** and the higher level language
