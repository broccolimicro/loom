---
title: Selection and Choice in Weaver
category: Tutorials
author: Edward Bingham
date: 2026-01-18
layout: post
---

In this tutorial, you'll learn how to control **which operations execute and
when** using Weaver's selection and choice operators. We'll write executable
programs demonstrating `if`, `await`, `or`, and `xor`.

## What You'll Learn

* Deterministic (`or`) vs non-deterministic (`xor`) selection
* Using `await` and `if` for conditional execution
* Combining selection with parallel composition (`and`)
* Compiling and simulating selection behaviors

## Step 1: Write a Deterministic Selection Program

Create `select.wv`:
```weaver
func MergeOr(chan InA, InB, Out) {
	while {
		await InA {
			Out.send(InA.recv())
		} or await InB {
			Out.send(InB.recv())
		}
	}
}
```

Explanation:

- `await InA { ... } or await InB { ... }` Executes exactly one branch. If both
  channels are ready simultaneously, Loom signals an error.
- `Out.send(...)` sends the processed value to the output channel.

## Step 3: Compile and Simulate Deterministic Selection

```bash
lm build
lm sim "select:MergeOr(chan)"
```

In the simulator, send values on `InA` and `InB` to see how the process reacts.

```bash
<chpsim> InA.send(5)
<chpsim> Out.recv()
5
<chpsim> InB.send(10)
<chpsim> Out.recv()
10
<chpsim> InA.send(1)
<chpsim> InB.send(1)
<chpsim> Out.recv()
error: mutual exclusion violation
```

- Only **one branch executes** per iteration.


## Step 4: Write a Non-Deterministic Choice Program

Create `non_deterministic.wv`:

```weaver
func NonDeterministic(chan ReqA, ReqB) {
	while {
		await ReqA {
			ReqA.recv()
		} xor await ReqB {
			ReqB.recv()
		}
	}
}
```

* `xor` allows the hardware to pick arbitrarily if both requests are ready.
* Useful for modeling arbiters or asynchronous contention.

## Step 5: Compile and Simulate Non-Deterministic Choice

```bash
lm build
lm sim "selection-demo:NonDeterministic(chan,chan)"
```

Send requests on `ReqA` and `ReqB`:

```bash
> ReqA.send()
> ReqB.send()
```

## Step 6: Combine `await` with `if` and Parallel Composition

Create `combined.wv`:

```weaver
func Combined(chan In, Out, Flag) {
	var fixed x = 0
	
	while {
		await In and if Flag {
			x = In.recv()
			Out.send(x * 3)
		}
	}
}
```

* Waits for **both data and a condition** to be true before executing.
* Demonstrates parallel composition with selection.

## Step 7: Build and Run the Combined Example

```bash
lm build
lm sim "selection-demo:Combined(chan,chan,chan)"
```

```bash
> send In 4
> send Flag 0
# Nothing happens because Flag is false
> send Flag 1
> recv Out
12
```

## Step 8: Inspect Internal Representation

```bash
lm show "selection-demo:Combined(chan,chan,chan)"
```

Generates a Petri-net visualization of guards and choices.

## What's Next?

You’ve now learned:

* Deterministic (`or`) vs non-deterministic (`xor`) branching
* How `await` waits for data validity and `if` checks conditions
* Combining selection with parallel execution (`and`)
* Running simulations and inspecting internal representations

The next tutorial will cover **communicating processes**, showing how multiple processes interact through channels.

