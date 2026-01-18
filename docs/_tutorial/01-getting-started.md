---
title: Getting Started with Weaver
category: Tutorials
author: Edward Bingham
date: 2026-01-15
layout: post
---

In this tutorial, you'll write your first Weaver program, set up a project, and
learn the basics of the Weaver toolchain.

## What You'll Learn

- How to install Loom
- How to set up a Weaver project
- Basic Weaver syntax
- How to compile and simulate your code
- The structure of a simple Weaver program

## Step 1: Install Loom

Loom is the circuit compiler for Weaver. Install it using the install script:

```bash
curl -sL https://raw.githubusercontent.com/broccolimicro/loom/refs/heads/main/install.sh | bash
```

This downloads the appropriate binaries for your system and places them in
`/usr/local` (or `C:\Program Files (x86)\Loom` on Windows).

Verify the installation:

```bash
lm help
```

You should see the Loom help text with available commands.

## Step 2: Initialize a Project

First, let's create a new Weaver project. Open a terminal and navigate to where
you want to create your project:

```bash
mkdir hello
cd hello
lm mod init hello
```

This creates a new Loom module in the current directory.

## Step 3: Write Your First Program

Create a file called `top.wv`:

> [!WARNING]
> Complex types are not yet supported. This is a currently working example, but it is not in-line with other documentation.
>
> TODO(edward.bingham) Update this example once the type system is in place.

```weaver
// top.wv - A simple counter process

func counter(chan Out) {
	var fixed count = 0
	
	while {
		Out.send(count)
		count = count + 1
		if count == 255 {
			count = 0
		}
	}
}
```

Let's break down what this code does:

- `func counter(chan Out)` - Defines a process called `counter` with an output channel `Out`
- Channels in function signatures use `chan` followed by the channel name
- `var int<8> count = 0` - Declares a counter variable, initialized to 0
- `while { ... }` - The perpetual loop that runs forever
- `Out.send(count)` - Sends the current count value on the output channel
- `count = count + 1` - Increments the counter
- `if count == 255 { count = 0 }` - Wraps around when reaching maximum value

## Step 4: Understanding the Process

This is a **process** - it represents a hardware component that runs
continuously. The `while` loop never terminates, which is correct for hardware
that should always be active.

The variable `count` is initialized to `0` before the loop, which means it's
set during hardware reset.

## Step 5: Build Your Program

Compile your Weaver program using the Loom build command:

```bash
lm build
```

By default, `lm build` looks for `top.wv` in the root directory of the project,
which is the directory that contains `lm.mod`. This will:
1. Parse your program
2. Synthesize it into Verilog (for data-level specifications)
3. Generate output files in the `build/` directory

The compiled Verilog will be in `build/rtl/counter.v`.

If there are any syntax errors, `lm build` will report them. Fix any errors and try again.

> [!WARNING]
> There are still quite a few bugs to work through in synthesis, this is a work in progress.

## Step 6: List Your Modules

You can see what modules are available in your project:

```bash
lm mod show
```

This will list all the functions and protocols defined in your module, for
example:
```
top:counter(chan)
```

## Step 7: Visualize Your Circuit

You can visualize the circuit structure:

```bash
lm show "top:counter(chan)"
```

This creates a png file called `counter.png`.

## Step 8: Simulate Your Circuit

> [!WARNING]
> The high level simulator is not yet functional.

If you want to simulate the behavior, you can use:

```bash
lm sim "top:counter(chan)"
```

This will start the simulation environment where you can interact with your circuit.

## What's Next?

You've created your first Weaver process! In the next tutorial, we'll:
- Learn about receiving data on channels
- Create processes that communicate with each other
- Build more complex behaviors

Continue to [Building Your First Process](./first-process) to learn more.

## Key Concepts

- **Processes** run forever in a `while` loop
- **Channels** (`chan`) are used to send data between processes
- **Function syntax**: `func name(chan Input1, Input2) chan Output` - input channels before return type, output channel as return type
- **Variables** declared before the loop are initialized during reset
- **`lm build`** compiles your Weaver code into hardware (looks for `top.wv` by default)
- **Output files** are placed in `build/rtl/` for Verilog or `build/ckt/` for production rules
- **`lm mod show`** lists all modules and functions in your project
