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

Then add `src/top.wv` with the following.
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

And then build your chip, creating `build/src/split.v` and `build/src/add.v`.
```
$ lm build
```

<a name="protocol_example"></a>
### Protocol Example

Now we're going to edit `src/top.wv` putting the following at the top.
```
import "buffer"
```

Then we can create a new file `src/buffer.wv` with the following.
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
$ cat build/src/wchb1b.prs
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
build/src
build/src/project.gds
build/src/module.spi
build/src/add.v
build/src/split.v
build/src/buffer.v
build/src/wchb1b.prs
```

```
$ klayout build/src/project.gds
```

![wchb1b](https://github.com/user-attachments/assets/726b96d3-6ebe-49f3-8830-6ac17941b804)

<a name="build"></a>
## Build and Install

### Linux

Install dependencies
```
sudo apt install ninja-build libqhull-dev libgraphviz-dev opencl-headers ocl-icd-opencl-dev mesa-opencl-icd python3-dev cmake
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
./install-local.sh
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
./install-local.sh
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
./install-local.sh
```

### Vim Syntax Highlighting

```bash
mkdir -p ~/.vim/syntax
cp weaver.vim ~/.vim/syntax
```

Add this to `~/.vimrc`
```
au BufRead,BufNewFile *.wv set filetype=weaver
au BufRead,BufNewFile *.cog set filetype=weaver
au! Syntax weaver source ~/.vim/syntax/weaver.vim
```

### Run Tests

To generate test binaries and then run the tests
```
make test
make check
```
