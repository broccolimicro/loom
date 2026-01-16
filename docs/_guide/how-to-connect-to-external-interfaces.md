---
title: How to Connect to External Interfaces
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Connect to External Interfaces

This guide shows you how to interface your Weaver designs with external hardware components.

## Goal

By the end of this guide, you'll be able to:
- Connect to external hardware interfaces
- Translate between Weaver channels and external protocols
- Handle different interface types
- Verify interface connections

## Prerequisites

- You understand [channels](../tutorials/channels)
- You know how [processes work](../reference/processes)
- You understand [structures](../reference/structures)

## Step 1: Define Interface Types

Define types that match your external interface:

```weaver
type ExternalInterface {
    int<32> data
    wire valid
    wire ready
}
```

## Step 2: Create Interface Adapter

Create an adapter process that translates between Weaver channels and external interfaces:

```weaver
func interface_adapter(chan<int<32>> weaver_channel) ExternalInterface external {
    var int<32> data
    
    while {
        await weaver_channel {
            data = weaver_channel.recv()
            external.data = data
            external.valid = vdd
            await external.ready {
                external.valid = gnd
            }
        }
    }
}
```

## Step 3: Handle Ready/Valid Protocol

Implement ready/valid handshaking:

```weaver
func ready_valid_adapter(chan<int<32>> in) {
    var int<32> data
    var wire ready_signal
    
    while {
        await in {
            data = in.recv()
            // Drive data and valid
            external_data = data
            external_valid = vdd
            
            // Wait for ready
            await ready_signal {
                external_valid = gnd
            }
        }
    }
}
```

## Step 4: Handle Different Protocols

Adapt to different interface protocols:

### AXI Protocol

```weaver
type AXI {
    int<32> data
    wire valid
    wire ready
    wire last
}

func axi_adapter(chan<int<32>> in) AXI axi {
    var int<32> data
    while {
        await in {
            data = in.recv()
            axi.data = data
            axi.valid = vdd
            axi.last = gnd  // Set appropriately
            await axi.ready {
                axi.valid = gnd
            }
        }
    }
}
```

### Memory Interface

```weaver
type MemoryInterface {
    int<32> address
    int<32> data
    wire read_enable
    wire write_enable
    wire ready
}

func memory_adapter(chan<MemoryRequest> in) MemoryInterface mem {
    var MemoryRequest req
    while {
        await in {
            req = in.recv()
            mem.address = req.addr
            if req.op == READ {
                mem.read_enable = vdd
                await mem.ready {
                    data = mem.data
                    mem.read_enable = gnd
                }
            } else {
                mem.data = req.data
                mem.write_enable = vdd
                await mem.ready {
                    mem.write_enable = gnd
                }
            }
        }
    }
}
```

## Step 5: Use Structures for Interface

Use structures to describe interface connections:

```weaver
struct external_interface(chan<int<32>> weaver_data) {
    // Map Weaver channel to external signals
    weaver_data -> external_data_signal
    external_ready_signal -> weaver_ready
}
```

## Step 6: Verify Interface Timing

Ensure interface timing is correct:

```weaver
func verify_timing(ExternalInterface iface) {
    // Verify setup/hold times
    // Verify protocol compliance
    // Check for timing violations
}
```

## Step 7: Complete Example: External Interface

Here's a complete example:

```weaver
type SPIInterface {
    int<8> data
    wire mosi
    wire miso
    wire sclk
    wire cs
}

func spi_adapter(chan<int<8>> weaver_data) SPIInterface spi {
    var int<8> data
    var int<3> bit_count = 0
    
    while {
        await weaver_data {
            data = weaver_data.recv()
            spi.cs = gnd  // Select
            
            // Clock out data
            while bit_count < 8 {
                spi.mosi = (data >> (7 - bit_count)) & 1
                spi.sclk = vdd
                // Wait
                spi.sclk = gnd
                bit_count = bit_count + 1
            }
            
            spi.cs = vdd  // Deselect
            bit_count = 0
        }
    }
}
```

## Best Practices

### Match Protocol Exactly

Ensure your adapter matches the external protocol exactly.

### Handle All Protocol States

Handle all states and transitions in the protocol.

### Verify Timing

Verify that timing requirements are met.

### Test Interface

Test the interface thoroughly before integration.

## Verification

After connecting interfaces:

1. **Protocol compliance**: Verify protocol is followed correctly
2. **Timing**: Check timing requirements are met
3. **Data integrity**: Verify data is transferred correctly
4. **Error handling**: Test error conditions

## Next Steps

- Learn about [integrating with Verilog](./how-to-integrate-with-verilog)
- Understand [structures](../reference/structures)
