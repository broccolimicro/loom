---
title: Processes
category: Reference
author: Edward Bingham
date: 2026-01-15
layout: post
---

Processes represent one or more pipeline stages in your design. They are the primary mechanism for describing concurrent, stateful hardware behavior.

## Process Syntax

```weaver
func processName(chan<type1> input1, chan<type2> input2) chan<type3> output {
    // reset statements
    while {
        // process body (perpetual loop)
    }
}
```

## Characteristics

- **Perpetual loops**: All processes have a perpetual outer loop and should never terminate
- **Reset behavior**: All statements that come before the outer loop are handled during reset
- **Pipeline stages**: Processes may be compiled into multiple pipeline stages
- **Channel communication**: Arguments are typically channels along which data is communicated
- **Preserved ordering**: The number and order of communications on input and output channels will be preserved with no-ops and backpressure built-in

## Example: Fetch Unit

```weaver
func fetch(chan Inc, Jmp) chan Addr {
    var uint<32> pc = 0
    while {
        Addr.send(pc)
        await Jmp {
            pc = Jmp.recv()
        } or await Inc {
            Inc.recv()
            pc = pc + 1
        }
    }
}
```

This example shows a fetch unit in a CPU that:
1. Sends the current program counter (`pc`) on the `Addr` channel
2. Waits for either a jump (`Jmp`) or increment (`Inc`) signal
3. Updates `pc` accordingly and continues

## Non-Channel Arguments

While arguments are typically channels, non-channel arguments may be used to represent shared variables in a more complex handshake protocol.

## Reset Initialization

Variables initialized before the `while` loop are set during reset:

```weaver
func counter() chan<int<32>> out {
    var int<32> count = 0  // Initialized during reset
    while {
        out.send(count)
        count = count + 1
    }
}
```

## Pipeline Compilation

The compiler may split a process into multiple pipeline stages. The communication order on channels is preserved, ensuring correct behavior even when pipelined.
