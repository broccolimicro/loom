---
title: How to Integrate with Verilog
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Integrate with Verilog

This guide shows you how to use Weaver designs with existing Verilog code.

## Goal

By the end of this guide, you'll be able to:
- Connect Weaver processes to Verilog modules
- Translate between Weaver channels and Verilog interfaces
- Use Verilog IP blocks in Weaver designs
- Verify integration

## Prerequisites

- You understand [channels](../tutorials/channels)
- You know how [processes work](../reference/processes)
- You have basic Verilog knowledge
- You understand [structures](../reference/structures)

## Step 1: Understand Compilation Target

Weaver compiles to different backends. For Verilog integration, understand how Weaver channels map to Verilog signals:

- Channels → Ready/Valid interfaces or handshakes
- Processes → Verilog modules
- Types → Verilog wire/reg types

## Step 2: Create Verilog Interface Wrapper

Create a wrapper that translates between Weaver and Verilog:

```weaver
struct verilog_wrapper(chan<int<32>> weaver_channel) {
    // Map Weaver channel to Verilog interface
    // This is compiler-generated, but you define the interface
}
```

## Step 3: Define Verilog Module Interface

Define the interface your Verilog module expects:

```weaver
// Verilog module interface
type VerilogInterface {
    int<32> data_in
    wire valid_in
    wire ready_out
    int<32> data_out
    wire valid_out
    wire ready_in
}
```

## Step 4: Create Adapter Process

Create a process that adapts between Weaver channels and Verilog interface:

```weaver
func verilog_adapter(
    chan<int<32>> weaver_in,
    chan<int<32>> weaver_out
) VerilogInterface verilog {
    var int<32> data
    
    // Input side: Weaver -> Verilog
    while {
        await weaver_in {
            data = weaver_in.recv()
            verilog.data_in = data
            verilog.valid_in = vdd
            await verilog.ready_out {
                verilog.valid_in = gnd
            }
        }
    } and {
        // Output side: Verilog -> Weaver
        while {
            await verilog.valid_out {
                if verilog.valid_out == vdd {
                    weaver_out.send(verilog.data_out)
                    verilog.ready_in = vdd
                    await ~verilog.valid_out {
                        verilog.ready_in = gnd
                    }
                }
            }
        }
    }
}
```

## Step 5: Instantiate Verilog Module

Use structures to instantiate Verilog modules:

```weaver
struct verilog_integration(
    chan<int<32>> weaver_input,
    chan<int<32>> weaver_output
) {
    // Instantiate Verilog module
    var VerilogModule verilog_inst
    
    // Connect Weaver to Verilog
    verilog_adapter(weaver_input, weaver_output) -> verilog_inst
    
    // Verilog module is connected through the adapter
}
```

## Step 6: Handle Clock Domains

If Verilog module uses different clock domains:

```weaver
func clock_domain_crossing(
    chan<int<32>> clk_a_domain,
    chan<int<32>> clk_b_domain
) {
    // Implement clock domain crossing
    // Use FIFO or handshake protocol
    var int<32> fifo[8]
    // ... clock domain crossing logic
}
```

## Step 7: Map Types Correctly

Ensure Weaver types map correctly to Verilog:

```weaver
// Weaver types map to Verilog:
// int<32> -> reg [31:0] or wire [31:0]
// bool -> reg or wire (1 bit)
// wire -> wire
// chan -> ready/valid interface
```

## Step 8: Complete Example: Verilog Integration

Here's a complete example:

```weaver
// Weaver process
func weaver_process(chan<int<32>> in) chan<int<32>> out {
    var int<32> value
    while {
        await in {
            value = in.recv()
            out.send(value * 2)
        }
    }
}

// Verilog interface
type VerilogIF {
    int<32> data
    wire valid
    wire ready
}

// Adapter
func verilog_adapter(
    chan<int<32>> weaver_ch,
    VerilogIF verilog_if
) {
    var int<32> data
    while {
        await weaver_ch {
            data = weaver_ch.recv()
            verilog_if.data = data
            verilog_if.valid = vdd
            await verilog_if.ready {
                verilog_if.valid = gnd
            }
        }
    }
}

// Integration
struct integrated_system(
    chan<int<32>> input,
    chan<int<32>> output
) {
    var chan<int<32>> intermediate
    var VerilogIF verilog_interface
    
    weaver_process(input) -> intermediate
    verilog_adapter(intermediate, verilog_interface)
    // Verilog module connects to verilog_interface
}
```

## Best Practices

### Match Interfaces Exactly

Ensure Weaver interface matches Verilog module interface exactly.

### Handle Timing

Verify timing requirements are met between Weaver and Verilog.

### Test Integration

Test the integration thoroughly before deployment.

### Document Interface

Clearly document the interface between Weaver and Verilog.

## Verification

After integration:

1. **Interface check**: Verify interfaces match
2. **Timing verification**: Check timing requirements
3. **Functional test**: Test functionality end-to-end
4. **Simulation**: Run co-simulation if available

## Next Steps

- Learn about [connecting to external interfaces](./how-to-connect-to-external-interfaces)
- Understand [structures](../reference/structures)
