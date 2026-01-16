---
title: How to Implement State Machines
category: How-to Guides
author: Edward Bingham
date: 2026-01-15
layout: post
---

# How to Implement State Machines

This guide shows you how to implement state machines using Weaver processes.

## Goal

By the end of this guide, you'll be able to:
- Create state machines in Weaver
- Handle state transitions correctly
- Implement state-dependent behavior
- Reset state machines properly

## Prerequisites

- You understand [processes](../reference/processes)
- You know how to use [channels](../tutorials/channels)
- You understand [validity and truthiness](../reference/validity-truthiness)

## Step 1: Define Your States

First, define the states your machine will have. Use a custom type or constants:

```weaver
type State {
    int<2> value  // 0=IDLE, 1=ACTIVE, 2=WAITING, 3=DONE
}

// Or use constants:
// IDLE = 0, ACTIVE = 1, WAITING = 2, DONE = 3
```

## Step 2: Create the State Variable

Create a state variable in your process and initialize it during reset:

```weaver
func state_machine() {
    var State state = {0}  // Start in IDLE state
    while {
        // State machine logic here
    }
}
```

## Step 3: Implement State Transitions

Use conditional composition (`or`) to handle state transitions. Only one transition should be active at a time:

```weaver
func state_machine(chan<Event> events) chan<Response> responses {
    var State state = {0}  // IDLE
    var Event event
    var Response response
    
    while {
        await events {
            event = events.recv()
            
            // State transitions - mutually exclusive
            if state.value == 0 {  // IDLE
                state.value = 1  // Transition to ACTIVE
                response = {1}  // Response type 1
                responses.send(response)
            } or if state.value == 1 {  // ACTIVE
                if event.type == 2 {
                    state.value = 2  // Transition to WAITING
                } or if event.type == 3 {
                    state.value = 3  // Transition to DONE
                }
            } or if state.value == 2 {  // WAITING
                state.value = 1  // Back to ACTIVE
            } or if state.value == 3 {  // DONE
                state.value = 0  // Back to IDLE
            }
        }
    }
}
```

## Step 4: Handle State-Dependent Behavior

Different states may have different behaviors. Use the state to control what happens:

```weaver
func state_machine(chan<Request> requests) chan<Response> responses {
    var State state = {0}  // IDLE
    var Request req
    var Response resp
    
    while {
        await requests {
            req = requests.recv()
            
            if state.value == 0 {  // IDLE
                // Accept new requests
                resp = process_request(req)
                responses.send(resp)
                state.value = 1  // Go to ACTIVE
            } or if state.value == 1 {  // ACTIVE
                // Queue request or reject
                if can_handle(req) {
                    resp = process_request(req)
                    responses.send(resp)
                } else {
                    resp = {error: true}
                    responses.send(resp)
                }
            } or if state.value == 2 {  // WAITING
                // Ignore requests
                // (or queue them for later)
            }
        }
    }
}
```

## Step 5: Add Timeout or External Triggers

State machines often need to transition based on timeouts or external events:

```weaver
func state_machine(chan<Event> events, chan<Timeout> timeouts) {
    var State state = {0}
    var Event event
    
    while {
        await events {
            event = events.recv()
            if state.value == 1 {  // ACTIVE
                state.value = 2  // Go to WAITING
            }
        } or await timeouts {
            timeouts.recv()
            if state.value == 2 {  // WAITING
                state.value = 0  // Timeout: back to IDLE
            }
        }
    }
}
```

## Step 6: Implement a Complete Example

Here's a complete state machine for a simple cache controller:

```weaver
type CacheState {
    int<2> value  // 0=IDLE, 1=LOOKUP, 2=FETCH, 3=WRITE
}

type CacheRequest {
    int<1> op      // 0=read, 1=write
    int<32> addr
    int<32> data   // For writes
}

type CacheResponse {
    int<1> hit     // 0=miss, 1=hit
    int<32> data   // For reads
}

func cache_controller(chan<CacheRequest> req) chan<CacheResponse> resp {
    var CacheState state = {0}  // IDLE
    var CacheRequest request
    var CacheResponse response
    
    while {
        await req {
            request = req.recv()
            
            if state.value == 0 {  // IDLE
                state.value = 1  // Start lookup
            } or if state.value == 1 {  // LOOKUP
                if request.op == 0 {  // Read
                    if cache_hit(request.addr) {
                        response = {hit: 1, data: cache_read(request.addr)}
                        resp.send(response)
                        state.value = 0  // Back to IDLE
                    } else {
                        state.value = 2  // Miss: go to FETCH
                    }
                } else {  // Write
                    state.value = 3  // Go to WRITE
                }
            } or if state.value == 2 {  // FETCH
                cache_fetch(request.addr)
                response = {hit: 0, data: memory_read(request.addr)}
                resp.send(response)
                state.value = 0  // Back to IDLE
            } or if state.value == 3 {  // WRITE
                cache_write(request.addr, request.data)
                response = {hit: 1, data: 0}
                resp.send(response)
                state.value = 0  // Back to IDLE
            }
        }
    }
}
```

## Step 7: Handle Reset

Ensure your state machine resets to the initial state:

```weaver
func state_machine() {
    var State state = {0}  // Reset to IDLE
    // Reset logic before the loop
    
    while {
        // State machine logic
    }
}
```

The state variable is initialized during reset, so it starts in the correct state.

## Best Practices

### Use `or` for Mutually Exclusive States

Always use `or` for state transitions to ensure only one path executes:

```weaver
if state == IDLE {
    // ...
} or if state == ACTIVE {
    // ...
}
```

### Keep State Transitions Clear

Make state transitions explicit and easy to follow. Document what triggers each transition.

### Handle All States

Ensure every state has a way to transition (or a reason to stay). Avoid dead-end states unless intentional.

### Test State Transitions

Verify that all valid state transitions work and invalid ones are prevented.

## Verification

Test your state machine:

1. **Test each state**: Verify behavior in each state
2. **Test transitions**: Ensure all valid transitions work
3. **Test edge cases**: What happens with unexpected inputs?
4. **Test reset**: Verify it returns to initial state

## Common Patterns

### Pattern: Linear State Machine

States progress in order: A → B → C → D

```weaver
if state == A {
    state = B
} or if state == B {
    state = C
} or if state == C {
    state = D
}
```

### Pattern: State Machine with Loops

States can loop back: A → B → C → A

```weaver
if state == A {
    state = B
} or if state == B {
    state = C
} or if state == C {
    state = A  // Loop back
}
```

### Pattern: Hierarchical States

Use nested conditions for hierarchical state machines:

```weaver
if state.major == IDLE {
    if state.minor == WAITING {
        // Sub-state behavior
    }
}
```

## Next Steps

- Learn about [building arbiters](./how-to-build-arbiters)
- Understand [process architecture](../explanations/process-architecture)
- Read about [composition operators](../explanations/composition-model)
