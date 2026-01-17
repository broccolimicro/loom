---
title: Process Architecture
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

Processes are Weaver's primary mechanism for describing concurrent, stateful hardware behavior. Understanding why they work the way they do helps you design better hardware.

## The Problem: Describing Concurrent State

Hardware is inherently concurrent and stateful. A CPU has multiple units (fetch, decode, execute, writeback) that all run simultaneously, each maintaining their own state. Traditional programming languages struggle with this because they assume sequential execution.

Weaver's processes solve this by providing a model that naturally describes concurrent, stateful hardware components.

## Why Perpetual Loops?

All processes have a perpetual outer loop that never terminates:

```weaver
func fetch() chan Addr {
    var uint<32> pc = 0
    while {
        Addr.send(pc)
        pc = pc + 1
    }
}
```

This might seem strange coming from software, where functions typically return. But hardware doesn't stop—a CPU fetch unit runs forever, continuously fetching instructions.

### Hardware Doesn't Terminate

In hardware, components are always active. They don't "finish" and return—they keep running as long as power is applied. The perpetual loop models this reality.

### Pipeline Stages

Processes represent one or more pipeline stages. A pipeline stage is a repeating pattern:
1. Receive input
2. Process it
3. Send output
4. Repeat

The perpetual loop captures this pattern naturally.

### Reset Behavior

Statements before the loop handle reset:

```weaver
func counter() chan<int<32>> out {
    var int<32> count = 0  // Reset: initialize count
    while {
        out.send(count)
        count = count + 1
    }
}
```

During reset, `count` is initialized to 0. Then the loop starts, and the process runs forever.

## Why Channels for Communication?

Processes communicate through channels, not shared memory:

```weaver
func producer(chan<int<8>> out) {
    // ...
}

func consumer(chan<int<8>> in) {
    // ...
}
```

This design choice has several advantages:

### Explicit Data Flow

Channels make data flow explicit. You can see exactly how data moves through your design by following channel connections. This makes designs easier to understand and verify.

### Natural Backpressure

Channels provide natural backpressure. If a receiver isn't ready, the sender blocks. This prevents buffer overflows and makes flow control automatic.

### Composable Design

Channels enable composable design. You can connect processes through channels without worrying about shared state or synchronization. This makes it easy to build complex systems from simple components.

### Hardware Mapping

Channels map naturally to hardware communication primitives:
- Handshake protocols (ready/valid)
- FIFOs and buffers
- Network-on-chip interconnects
- Bus protocols

The compiler can translate channel operations to appropriate hardware primitives for the target backend.

## Process Arguments and Returns

Processes take channels as arguments and return channels:

```weaver
func fetch(chan Inc, Jmp) chan Addr {
    // ...
}
```

This signature says:
- `fetch` takes two input channels: `Inc` and `Jmp`
- `fetch` returns one output channel: `Addr`

### Why Channels, Not Values?

Processes operate on streams of data, not single values. A fetch unit doesn't fetch one instruction and stop—it continuously fetches instructions. Channels model these streams naturally.

### Non-Channel Arguments

While channels are typical, processes can also take non-channel arguments for shared variables in complex handshake protocols:

```weaver
func process(chan Data in, shared_var) chan Data out {
    // Can read/write shared_var for handshaking
}
```

This is less common but useful for certain communication patterns.

## Pipeline Compilation

The compiler may split a process into multiple pipeline stages. This is transparent to you—you write the process as a single unit, and the compiler optimizes it.

### Preserved Ordering

Even when pipelined, the compiler preserves the order of channel communications:

```weaver
func process(chan A in) chan B out {
    while {
        var data = in.recv()
        out.send(process(data))
    }
}
```

The compiler ensures that:
1. `in.recv()` happens before `out.send()`
2. The order is preserved even if the process is split into stages
3. Backpressure is handled automatically

### No-Ops and Backpressure

If a process is pipelined, the compiler inserts no-ops and backpressure logic to maintain correct ordering. This is handled automatically—you don't need to think about it.

## Process Composition

Processes compose through channels:

```weaver
func stage1() chan<int<16>> out {
    // ...
}

func stage2(chan<int<16>> in) chan<int<16>> out {
    // ...
}

func stage3(chan<int<16>> in) {
    // ...
}

// Compose them
var chan<int<16>> s1_out, s2_out
stage1() -> s1_out
stage2(s1_out) -> s2_out
stage3(s2_out)
```

This creates a pipeline where data flows: stage1 → stage2 → stage3.

### Why This Works

Process composition works because:
- Processes are independent (no shared state)
- Channels provide clean interfaces
- The compiler handles timing and synchronization

You can compose processes without worrying about low-level details like clock domains or handshake protocols.

## Processes vs Functions

Weaver has both processes and functions. Understanding the difference is important:

### Processes

- Never terminate (perpetual loop)
- Stateful (maintain state across iterations)
- Communicate through channels
- Represent hardware components

### Functions

- Terminate (return a value)
- Stateless (no side effects)
- Compute values, don't communicate
- Represent pure computation

Functions are inlined and may be time-multiplexed. Processes are instantiated as separate hardware components.

## Design Patterns

### Producer-Consumer

```weaver
func producer(chan<int<8>> out) {
    while {
        out.send(generate_data())
    }
}

func consumer(chan<int<8>> in) {
    while {
        await in {
            process(in.recv())
        }
    }
}
```

### Pipeline

```weaver
func stage1() chan<Data> out { /* ... */ }
func stage2(chan<Data> in) chan<Data> out { /* ... */ }
func stage3(chan<Data> in) chan<Data> out { /* ... */ }
```

### State Machine

```weaver
func state_machine() {
    var State state = IDLE
    while {
        await event {
            if state == IDLE {
                state = ACTIVE
            } or if state == ACTIVE {
                state = DONE
            }
        }
    }
}
```

## Design Trade-offs

The process model has trade-offs:

### Advantages

- **Natural hardware model**: Maps directly to hardware components
- **Composable**: Processes compose cleanly through channels
- **Explicit concurrency**: Makes parallelism clear
- **Verifiable**: Channel interfaces are easy to verify

### Disadvantages

- **Learning curve**: Different from software functions
- **Verbosity**: More verbose than sequential code
- **Compiler complexity**: Compiler must handle pipelining and optimization

Overall, the advantages outweigh the disadvantages. The process model makes hardware designs clearer and more maintainable.

## Summary

Processes are Weaver's way of describing concurrent, stateful hardware components. The perpetual loop models hardware that never stops. Channels provide explicit, composable communication. The compiler handles pipelining and optimization automatically.

Understanding the process model helps you design better hardware. Think of processes as independent hardware components that communicate through well-defined interfaces. This mental model maps directly to how hardware actually works.
