---
title: Building Your First Process
category: Tutorials
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Building Your First Process

In this tutorial, you'll build a more complex process that receives data, processes it, and sends results. This demonstrates the core concepts of Weaver processes.

## What You'll Learn

- How to receive data on channels
- Using `await` to wait for valid data
- Processing data in a loop
- Combining multiple operations

## Step 1: Create a Doubler Process

Let's create a process that receives numbers and doubles them. Update `top.wv`:

```weaver
// top.wv - Doubles incoming values

func doubler(chan Input) chan Output {
    var int<16> value
    
    while {
        await Input {
            value = Input.recv()
            Output.send(value * 2)
        }
    }
}
```

## Step 2: Understanding the Code

Let's examine each part:

- `func doubler(chan Input) chan Output` - Function signature with input channel `Input` and output channel `Output`
- Channels in function signatures are declared as `chan ChannelName` (the type is inferred from usage)
- `var int<16> value` - A variable to hold the received value
- `await Input { ... }` - Waits until `Input` has valid data
- `value = Input.recv()` - Receives the value from the channel
- `Output.send(value * 2)` - Sends the doubled value

## Step 3: The `await` Statement

The `await` statement is crucial in Weaver. It blocks execution until the condition becomes **valid**. In this case, it waits until there's data available on the `input` channel.

Think of `await` as waiting for an event - a transition from "no data" to "data available".

## Step 4: Build and Test

Compile your process:

```bash
lm build
```

If successful, you should see no errors. The compiled Verilog will be in `build/rtl/doubler.v`. The process is now ready to be used in larger designs.

You can also list your modules:

```bash
lm mod show
```

This will show:
```
top:doubler(chan,chan)
```

## Step 5: Create a Complete System

Let's create a system that uses both a counter and our doubler. Update `top.wv`:

```weaver
// top.wv - A complete system with counter and doubler

func counter() chan<int<16>> out {
    var int<16> count = 0
    
    while {
        out.send(count)
        count = count + 1
        if count == 100 {
            count = 0
        }
    }
}

func doubler(chan<int<16>> input) chan<int<16>> output {
    var int<16> value
    
    while {
        await input {
            value = input.recv()
            output.send(value * 2)
        }
    }
}

// Main system that connects counter to doubler
func main() {
    var chan<int<16>> counter_out, doubler_out
    
    counter() -> counter_out
    doubler(counter_out) -> doubler_out
}
```

## Step 6: Understanding Process Composition

In the `main` function, we:
1. Declare channels to connect processes
2. Instantiate the `counter` process, connecting its output to `counter_out`
3. Instantiate the `doubler` process, taking `counter_out` as input and producing `doubler_out`

The processes run in parallel, with data flowing from counter → doubler.

## Step 7: Build the System

```bash
lm build
```

This compiles the entire system. You'll find:
- `build/rtl/counter.v`
- `build/rtl/doubler.v`

## Step 8: List Your Modules

See all available modules:

```bash
lm mod show
```

This will show:
```
top:counter(chan)
top:doubler(chan,chan)
```

## Step 9: Visualize the System

See how the processes are structured:

```bash
lm show top.wv -o system.dot
```

## Key Concepts Learned

- **`await`** waits for data to become valid
- **`input.recv()`** receives and removes data from a channel
- **Processes can be composed** by connecting their channels
- **Multiple processes run in parallel** in Weaver

## Common Patterns

### Pattern: Process with Reset Logic

```weaver
func example() chan<int<8>> out {
    var int<8> state = 0  // Reset value
    
    while {
        // Process logic here
    }
}
```

### Pattern: Waiting for Multiple Channels

```weaver
await channel1 & channel2 {
    var int<16> a = channel1.recv()
    var int<16> b = channel2.recv()
    // Process both values
}
```

## What's Next?

Now that you understand basic processes, learn about:
- More advanced channel communication patterns
- Using `or` and `xor` for conditional behavior
- Working with custom types

Continue to [Working with Channels](./channels) to explore channel communication in depth.
