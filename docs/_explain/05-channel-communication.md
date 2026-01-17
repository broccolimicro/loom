---
title: Channel Communication
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

# Channel Communication

Channels are Weaver's primary mechanism for inter-process communication. Understanding why channels were chosen and how they work helps you design better hardware.

## The Problem: Inter-Process Communication

In hardware, different components need to communicate. A CPU fetch unit sends addresses to memory. An ALU receives operands and sends results. A cache controller coordinates with multiple memory banks.

Traditional approaches include:
- **Shared memory**: Multiple components access the same memory
- **Buses**: Components share a communication bus
- **Point-to-point wires**: Direct connections between components

Each has trade-offs. Weaver chose channels as the primary communication mechanism. Why?

## Why Channels?

Channels provide several advantages over other communication mechanisms:

### Explicit Data Flow

Channels make data flow explicit. You can see exactly how data moves through your design:

```weaver
func fetch() chan Addr { /* ... */ }
func memory(chan Addr addr) chan Data { /* ... */ }
func decode(chan Data data) { /* ... */ }

// Data flows: fetch → memory → decode
fetch() -> addr
memory(addr) -> data
decode(data)
```

This explicitness makes designs easier to understand and verify.

### Natural Synchronization

Channels provide natural synchronization through blocking:

```weaver
sender.send(value)    // Blocks until receiver is ready
receiver.recv()        // Blocks until sender sends
```

This eliminates the need for explicit synchronization primitives like mutexes or semaphores. The channel itself handles synchronization.

### Automatic Backpressure

Channels provide automatic backpressure. If a receiver isn't ready, the sender blocks. This prevents buffer overflows and makes flow control automatic:

```weaver
func fast_producer(chan<int> out) {
    while {
        out.send(data)  // Blocks if receiver is slow
    }
}

func slow_consumer(chan<int> in) {
    while {
        await in {
            process_slowly(in.recv())
        }
    }
}
```

The fast producer automatically slows down to match the slow consumer.

### Composable Design

Channels enable composable design. You can connect processes through channels without worrying about shared state:

```weaver
func stage1() chan<Data> out { /* ... */ }
func stage2(chan<Data> in) chan<Data> out { /* ... */ }
func stage3(chan<Data> in) { /* ... */ }

// Compose: stage1 → stage2 → stage3
stage1() -> s1_out
stage2(s1_out) -> s2_out
stage3(s2_out)
```

Each process is independent and can be tested in isolation.

## Channel Operations

Weaver provides three channel operations:

### Send

```weaver
channel.send(value)
```

Sends a value on the channel. Blocks until the receiver is ready to receive.

### Receive

```weaver
value = channel.recv()
```

Receives a value from the channel. Removes the value and unblocks the sender.

### Peek

```weaver
value = channel
```

Reads the value without removing it. The sender remains blocked. Useful for conditional operations.

## Channel Semantics

### Blocking Behavior

Channels are blocking by default:
- **Sender blocks** until receiver calls `recv()` or peeks
- **Receiver blocks** (in `await`) until sender calls `send()`

This blocking ensures proper synchronization. You can't have race conditions because operations are atomic.

### Ordering

Channels preserve ordering. If you send values A, B, C, the receiver will receive them in that order. This is guaranteed even if the process is pipelined.

### Buffering

Weaver channels are unbuffered by default. Each send requires a corresponding receive. This makes data flow explicit and prevents hidden buffering issues.

The compiler may insert buffers during optimization, but this is transparent to you. The semantics remain the same.

## Channel Patterns

### Producer-Consumer

The most common pattern:

```weaver
func producer(chan<int> out) {
    while {
        out.send(generate())
    }
}

func consumer(chan<int> in) {
    while {
        await in {
            process(in.recv())
        }
    }
}
```

### Pipeline

Multiple stages connected in sequence:

```weaver
func stage1() chan<Data> out { /* ... */ }
func stage2(chan<Data> in) chan<Data> out { /* ... */ }
func stage3(chan<Data> in) { /* ... */ }
```

### Multiplexing

Multiple producers, one consumer:

```weaver
func mux(chan<int> a, chan<int> b) chan<int> out {
    while {
        await a {
            out.send(a.recv())
        } or await b {
            out.send(b.recv())
        }
    }
}
```

### Demultiplexing

One producer, multiple consumers:

```weaver
func demux(chan<int> in) (chan<int> a, chan<int> b) {
    while {
        await in {
            var int value = in.recv()
            if route_to_a(value) {
                a.send(value)
            } or if route_to_b(value) {
                b.send(value)
            }
        }
    }
}
```

## Channel Implementation

The compiler translates channel operations to appropriate hardware primitives based on the target backend:

### Quasi-Delay Insensitive (QDI)

In QDI, channels map to handshake protocols:
- `send()` asserts data and request
- `recv()` asserts acknowledge
- Completion signals coordinate timing

### Synchronous (Verilog)

In synchronous designs, channels might map to:
- Ready/valid interfaces
- FIFOs with flow control
- Bus protocols with handshaking

### Other Models

The channel abstraction is flexible enough to support other communication models. The compiler handles the translation.

## Design Trade-offs

Channels have trade-offs compared to other communication mechanisms:

### Advantages

- **Explicit data flow**: Easy to see how data moves
- **Automatic synchronization**: No need for explicit locks
- **Composable**: Processes connect cleanly
- **Verifiable**: Channel interfaces are easy to verify
- **Hardware-native**: Maps to common hardware patterns

### Disadvantages

- **Overhead**: Channels have some overhead compared to direct wires
- **Learning curve**: Different from shared memory models
- **Compiler complexity**: Compiler must translate to target primitives

Overall, the advantages outweigh the disadvantages, especially for complex systems where explicit data flow and composability are crucial.

## When Not to Use Channels

Channels are great for inter-process communication, but sometimes other mechanisms are better:

### Shared Variables

For simple shared state in a single process, use variables:

```weaver
func process() {
    var int counter = 0
    while {
        // Use counter directly
    }
}
```

### Direct Wires

For simple point-to-point connections, direct assignment might be simpler:

```weaver
struct simple(chan<int> in) chan<int> out {
    out = in  // Direct connection
}
```

But for most cases, channels are the right choice.

## Summary

Channels are Weaver's primary mechanism for inter-process communication. They provide explicit data flow, automatic synchronization, and composable design. Understanding how channels work helps you design better hardware.

Channels map naturally to hardware communication primitives, and the compiler handles the translation to target backends. This gives you the benefits of high-level abstractions while maintaining the ability to target different hardware styles.
