---
title: Building Your First Process
category: Tutorials
author: Edward Bingham
date: 2026-01-15
layout: post
---

In this tutorial, you'll build a more complex process that receives data,
processes it, and sends results. This demonstrates the core concepts of Weaver
processes.

## What You'll Learn

- How to receive data on channels
- Processing data in a loop
- Combining multiple operations

## Step 1: Create a Collatz Process

Let's create a process that receives numbers and emits the entire Collatz sequence for that number.

> [!WARNING]
> There are still quite a few bugs to work through in synthesis, this will generate a lot of errors and produce incorrect results.

> [!WARNING]
> Loom does not yet have a true type system or support for templates. There is only a very basic set of types with manual bitwidths at the moment: `chan`, `fixed`, `ufixed`, `bool`.

```weaver
func Collatz(chan In, Out) {
	var fixed value = 1
	while {
		if value <= 1 {
			value = In.recv()
		} or if value > 1 && value % 2 == 0 {
			value = value / 2
			Out.send(value)
		} or if value > 1 && value % 2 == 1 {
			value = 3 * value + 1
			Out.send(value)
		}
	}
}
```

Let's examine each part:

- `func Collatz(chan In, Out)` - defines the Collatz process with input In and output Out.
- `var fixed value = 1` - stores the current number, starting at 1.
- `while { … }` - runs forever.
- `if value <= 1` - get a new number from `In`.
- `if value > 1 && even` - divide by 2, send result to `Out`.
- `if value > 1 && odd` - multiply by 3 and add 1, send result to `Out`.

## Step 2: Build and Test

Compile your process:

```bash
lm build
```

If successful, you should see no errors. The compiled Verilog will be in
`build/rtl/Collatz.v`. The process is now ready to be used in larger designs.

You can also list your modules:

```bash
lm mod show
```

This will show:
```
top:Collatz(chan,chan)
top:Counter(chan)
```

## Step 3: Create a Complete System

> [!WARNING]
> Process instantiation is not yet functional, `top()` will be empty.

Let's create a system that uses both a counter and our doubler. Update `top.wv`:

```weaver
func Counter(chan Out) {
	var fixed count = 0

	while {
		Out.send(count)
		count = count + 1
		if count == 255 {
			count = 0
		}
	}
}

func Collatz(chan In, Out) {
  var fixed value = 1
  while {
    if value <= 1 {
      value = In.recv() 
    } or if value > 1 && value % 2 == 0 {
      value = value / 2
      Out.send(value)                                    
    } or if value > 1 && value % 2 == 1 {
      value = 3 * value + 1                              
      Out.send(value)
    }
  } 
}

func top(chan Out) {
	var chan C
	Counter(C) and
	Collatz(C, Out)
}
```

In the `top` function, we:
1. Declare channels to connect processes
2. Instantiate the `Counter` process, connecting its output to `C`
3. Instantiate the `Collatz` process, taking `C` as input and producing `Out`

The processes run in parallel, with data flowing from Counter to Collatz.

## Step 4: Build the System

```bash
lm build
```

This compiles the entire system. You'll find:
- `build/rtl/Counter.v`
- `build/rtl/Collatz.v`

## Step 5: List Your Modules

See all available modules:

```bash
lm mod show
```

This will show:
```
top:Counter(chan)
top:Collatz(chan,chan)
top:top(chan)
```

## Step 6: Inspect the System

To generate a guarded petri-net image for all processes:

```bash
lm show
```

This will create:

```
build/dbg/Collatz.png
build/dbg/top.png
build/dbg/Counter.png
```

Or you can see a single process

```bash
lm show "top:Collatz(chan,chan)"
```

which will create

```
./Collatz.png
```

## Key Concepts Learned

- **`In.recv()`** receives and removes data from a channel
- **Processes can be composed** by connecting their channels
- **Multiple processes run in parallel** in Weaver

## What's Next?

Now that you understand basic processes, learn about:
- More advanced channel communication patterns
- Using `or` and `xor` for conditional behavior
- Working with custom types

Continue to [Working with Channels](./channels) to explore channel communication in depth.
