---
title: How to Build Arbiters
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Build Arbiters

This guide shows you how to implement arbitration logic when multiple requesters compete for a shared resource.

## Goal

By the end of this guide, you'll be able to:
- Create arbiters using `or` and `xor` operators
- Implement fair arbitration schemes
- Handle multiple requesters competing for resources
- Choose between deterministic and non-deterministic arbitration

## Prerequisites

- You understand [processes](../reference/processes)
- You know how to use [channels](../tutorials/channels)
- You understand [composition operators](../reference/operators)
- You've read about [channel communication](../explanations/channel-communication)

## Step 1: Understand Arbitration

An arbiter selects one requester from multiple competing requesters. You need to decide:
- **Deterministic** (`or`): Must be mutually exclusive, errors if multiple ready
- **Non-deterministic** (`xor`): Picks one arbitrarily if multiple ready

## Step 2: Basic Arbiter with `or`

Use `or` for deterministic arbitration when requesters must be mutually exclusive:

```weaver
func arbiter(chan<Request> reqA, chan<Request> reqB) chan<Request> out {
    var Request request
    
    while {
        await reqA {
            request = reqA.recv()
            out.send(request)
        } or await reqB {
            request = reqB.recv()
            out.send(request)
        }
    }
}
```

**Important**: With `or`, if both `reqA` and `reqB` are ready simultaneously, it's an error. They must be mutually exclusive.

## Step 3: Non-Deterministic Arbiter with `xor`

Use `xor` when you want arbitrary selection if multiple requesters are ready:

```weaver
func arbiter(chan<Request> reqA, chan<Request> reqB) chan<Request> out {
    var Request request
    
    while {
        await reqA {
            request = reqA.recv()
            out.send(request)
        } xor await reqB {
            request = reqB.recv()
            out.send(request)
        }
    }
}
```

With `xor`, if both channels are ready, one is chosen arbitrarily. This is useful for load balancing or when order doesn't matter.

## Step 4: Add Grant Signals

Often, you need to notify the requester that they were selected:

```weaver
func arbiter(
    chan<Request> reqA, chan<Request> reqB,
    chan<Grant> grantA, chan<Grant> grantB
) chan<Request> out {
    var Request request
    
    while {
        await reqA {
            request = reqA.recv()
            grantA.send({granted: true})
            out.send(request)
        } or await reqB {
            request = reqB.recv()
            grantB.send({granted: true})
            out.send(request)
        }
    }
}
```

## Step 5: Implement Round-Robin Arbitration

For fair arbitration, implement round-robin (cycle through requesters):

```weaver
type ArbiterState {
    int<2> next  // 0=A, 1=B, 2=C, 3=D
}

func round_robin_arbiter(
    chan<Request> reqA, chan<Request> reqB,
    chan<Request> reqC, chan<Request> reqD
) chan<Request> out {
    var ArbiterState state = {0}  // Start with A
    var Request request
    
    while {
        if state.next == 0 {
            await reqA {
                request = reqA.recv()
                out.send(request)
                state.next = 1  // Next: B
            } or if state.next == 1 {
                await reqB {
                    request = reqB.recv()
                    out.send(request)
                    state.next = 2  // Next: C
                } or if state.next == 2 {
                    await reqC {
                        request = reqC.recv()
                        out.send(request)
                        state.next = 3  // Next: D
                    } or if state.next == 3 {
                        await reqD {
                            request = reqD.recv()
                            out.send(request)
                            state.next = 0  // Next: A (wrap around)
                        }
                    }
                }
            }
        }
    }
}
```

## Step 6: Priority-Based Arbiter

Implement priority where higher-priority requesters are served first:

```weaver
func priority_arbiter(
    chan<Request> high_priority,
    chan<Request> low_priority
) chan<Request> out {
    var Request request
    
    while {
        // Always check high priority first
        await high_priority {
            request = high_priority.recv()
            out.send(request)
        } or await low_priority {
            // Only serve low priority if high is not ready
            request = low_priority.recv()
            out.send(request)
        }
    }
}
```

Note: With `or`, if high priority is ready, low priority won't be checked. This implements strict priority.

## Step 7: Weighted Fair Queuing

For more complex fairness, implement weighted fair queuing:

```weaver
type WFQState {
    int<32> creditA
    int<32> creditB
}

func weighted_fair_queuing(
    chan<Request> reqA,  // Weight 3
    chan<Request> reqB    // Weight 1
) chan<Request> out {
    var WFQState state = {creditA: 3, creditB: 1}
    var Request request
    
    while {
        // Serve A if it has credit and is ready
        if state.creditA > 0 {
            await reqA {
                request = reqA.recv()
                out.send(request)
                state.creditA = state.creditA - 1
                state.creditB = state.creditB + 1  // Recharge B
            } or if state.creditB > 0 {
                await reqB {
                    request = reqB.recv()
                    out.send(request)
                    state.creditB = state.creditB - 1
                    state.creditA = state.creditA + 3  // Recharge A (weight 3)
                }
            }
        } or if state.creditA == 0 && state.creditB == 0 {
            // Reset credits
            state.creditA = 3
            state.creditB = 1
        }
    }
}
```

## Step 8: Complete Example: Memory Arbiter

Here's a complete memory arbiter that handles read and write requests:

```weaver
type MemRequest {
    int<1> op      // 0=read, 1=write
    int<32> addr
    int<32> data   // For writes
}

type MemResponse {
    int<32> data   // For reads
}

func memory_arbiter(
    chan<MemRequest> cpu_req,
    chan<MemRequest> dma_req,
    chan<MemResponse> cpu_resp,
    chan<MemResponse> dma_resp
) {
    var MemRequest request
    var MemResponse response
    
    while {
        // CPU has higher priority
        await cpu_req {
            request = cpu_req.recv()
            if request.op == 0 {  // Read
                response = {data: memory_read(request.addr)}
                cpu_resp.send(response)
            } else {  // Write
                memory_write(request.addr, request.data)
                response = {data: 0}
                cpu_resp.send(response)
            }
        } or await dma_req {
            // DMA only served if CPU not ready
            request = dma_req.recv()
            if request.op == 0 {  // Read
                response = {data: memory_read(request.addr)}
                dma_resp.send(response)
            } else {  // Write
                memory_write(request.addr, request.data)
                response = {data: 0}
                dma_resp.send(response)
            }
        }
    }
}
```

## Choosing Between `or` and `xor`

### Use `or` when:
- Requesters must be mutually exclusive
- You need deterministic behavior
- You can guarantee only one requester is ready at a time
- You want errors if multiple are ready (helps catch bugs)

### Use `xor` when:
- Multiple requesters can be ready simultaneously
- Order doesn't matter
- You want load balancing
- You need non-deterministic selection

## Best Practices

### Keep Arbitration Logic Simple

Complex arbiters are harder to verify and debug. Start simple and add complexity only if needed.

### Document Your Arbitration Policy

Clearly document whether you're using priority, round-robin, weighted, etc.

### Test with Multiple Requesters

Verify your arbiter works correctly when multiple requesters compete.

### Consider Fairness

Ensure your arbitration scheme is fair (or intentionally unfair if that's the goal).

## Verification

Test your arbiter:

1. **Single requester**: Verify it works with one requester
2. **Multiple requesters**: Test with all requesters active
3. **Fairness**: Verify fairness properties (if applicable)
4. **Priority**: Verify priority ordering (if applicable)

## Next Steps

- Learn about [creating pipelines](./how-to-create-pipelines)
- Understand [composition model](../explanations/composition-model)
- Read about [handling backpressure](./how-to-handle-backpressure)
