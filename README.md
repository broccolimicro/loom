# Loom &nbsp; [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.13992088.svg)](https://doi.org/10.5281/zenodo.13992088) [![Documentation](https://img.shields.io/badge/Documentation-blue.svg)](https://broccolimicro.github.io/loom) [![Tests](https://github.com/broccolimicro/loom/actions/workflows/test.yml/badge.svg)](https://github.com/broccolimicro/loom/actions/workflows/test.yml) [![Release](https://github.com/broccolimicro/loom/actions/workflows/release.yml/badge.svg)](https://github.com/broccolimicro/loom/actions/workflows/release.yml)

Loom is a compiler for Quasi-Delay Insensitive (QDI) asynchronous circuits.
While the core compilation kernel is still a work in progress, it can reliably
compile a wire-level specification without state-conflicts down to a
cell-mapped spice netlist and automatically generated custom cell layouts that
are mostly DRC and LVS clean. There is currently limited functionality to
solve state-conflicts which is under active development. See the
[Development Status](#status) section for more details.

![LoomArchitecture](https://github.com/user-attachments/assets/25e92ab3-69bc-474a-ae2a-04ec60623c4f)

## Table of Contents
1. [Install](#install)
1. [Example](#example)
3. [Build From Source](#build)
2. [Development Status](#status)

<a name="install"></a>
## Install

This install script downloads the appropriate binaries for your system and places them in `/usr/local` (or `C:\Program Files (x86)\Loom` on windows)
```
curl -sL https://raw.githubusercontent.com/broccolimicro/loom/refs/heads/main/install.sh | sudo bash
```

<a name="example"></a>
## Example

Write your functional specification.

**wchb1b.cog**
```
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
```

Compile your functional specification to production rules.

```
$ lm build -r wchb1b.cog
$ cat wchb1b.prs
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
$ lm build wchb1b.cog
$ klayout wchb1b.gds
```

![wchb1b](https://github.com/user-attachments/assets/726b96d3-6ebe-49f3-8830-6ac17941b804)

Loom also supports a process calculus called Hand-Shaking Expansions (HSE)

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
## Development Status (May 5, 2025)

### Synthesis
* **Templating (0%)** parameterize your module specifications.
* **Modules (40%)** be able to break up your circuit into modules and construct larger systems.
* **Process Decomposition (0%)** Break large processes up into pipeline stages.
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
