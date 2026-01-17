---
title: Processes and Message Passing
category: Explanations
author: Edward Bingham
date: 2026-01-15
layout: post
---

In Weaver, computation is expressed as **network of long-lived stateful
processes**. This is a deliberate break from software languages, where
computation is organized around short-lived function calls. This reflects a
fundamental difference between hardware and software.

## Processes, not Functions

Software languages organize computation around functions because functions
align with the software execution model. Execution starts, local state is
created, a result is computed, and execution ends. This model assumes that
persistence is either short-lived (stack frames) or hidden behind abstractions
like heaps, globals, or runtimes. That assumption breaks down immediately in
hardware.

Hardware components do not terminate. A counter, an arbiter, a pipeline stage,
or a protocol endpoint exists for as long as the chip is powered. Its state is
not an implementation detail but the *thing itself*. Attempting to express such
components as functions forces a mismatch. Either the function must never
return, or the persistent state must be smuggled in through globals, implicit
state machines, or a hidden scheduler that decides when the function "runs."
All of these approaches obscure where state lives and when it changes.

```weaver
func counter() chan<int<32>> out {
    var int<32> count = 0  // reset
    while {
        out.send(count)
        count = count + 1
    }
}
```

Consider a simple counter that continuously produces values. There is no
meaningful "return value" here, and no natural call boundary. The behavior is
not a computation over time but a *presence over time*. By making this a
process, Weaver forces persistence to be explicit and unavoidable. If something
has state across time, it must be represented as a process. This removes
ambiguity. Designers do not have to infer which variables persist, which reset,
or which are shared across invocations. The structure of the program directly
reflects the structure of the hardware.

## Message Passing, not Shared Memory

Shared memory works in software because software assumes a total or near-total
ordering of events. Instructions execute in sequence, memory operations are
serialized by a memory model, and conflicts are resolved by mechanisms such as
locks, caches, and coherence protocols. These mechanisms are expensive,
complex, and fundamentally *designed artifacts*. They do not exist by default
in hardware.

In raw hardware, multiple components operate concurrently. Signals change
simultaneously. There is no implicit notion of “who went first,” and no natural
place to hide arbitration or mutual exclusion. When two components read or
write the same state, the designer must explicitly build the logic that defines
how conflicts are resolved. Treating shared memory as a primitive assumes away
this work and hides essential structure.

Channels replace shared memory with explicit communication paths. A channel
represents a physical connection that exists as long as the connected
components exist. It defines not just what data is transferred, but *when* that
transfer is allowed to occur. When one process sends and another receives,
there is a well-defined causal relationship. The send cannot complete unless
the receive is possible, and the receive cannot occur unless something was
sent. No implicit ordering, arbitration, or visibility rules are required
beyond the channel itself.

This maps directly to real hardware interfaces. Backpressure, flow control, and
synchronization are not layered on top of communication. They *are* the
communication. Blocking on a channel does not mean "pause execution until
scheduled again," as it would in software. It means "the physical conditions
for transfer are not yet satisfied." The model describes constraints, not
control flow.

By rejecting shared memory and embracing channels, Weaver forces communication
to be explicit and local. If buffering, arbitration, or broadcast is needed, it
must be constructed deliberately, either by the designer or by the compiler as
a visible transformation. Time, storage, and causality are no longer hidden
behind a global abstraction.

## Synchronization, not Scheduling

When there does need to be a shared resource, Weaver relies upon
synchronization across channels rather than locks. Locks, mutexes, and similar
mechanisms exist to schedule access to shared state in software systems, where
many independent threads are time-multiplexed onto a small number of execution
resources. They answer the question of *who is allowed to run next* and rely on
a runtime to enforce that decision by blocking, waking, and rescheduling
threads.

Hardware does not operate this way. There is no scheduler deciding which
component may proceed, and no notion of suspending one piece of hardware so
another can "take a turn." All components exist and operate simultaneously.
If multiple components interact with the same resource, the resolution of that
interaction must be expressed as logic, not as a policy enforced by a runtime.

In Weaver, shared resources are therefore built as explicit processes that
mediate access through channels. Arbitration is not hidden behind a mutex; it
is a piece of hardware with defined behavior. A client does not acquire a lock
and enter a critical section. Instead, it synchronizes by sending a request and
waiting for a response. Progress occurs only when the necessary communication
can take place.

```weaver
func arbiter(chan<Req> a, chan<Req> b) chan<Grant> out {
	while {
		await a {
			out.send(handle(a.recv()))
		} xor await b {
			out.send(handle(b.recv()))
		}
	}
}
```

This is not scheduling in the software sense. Nothing is being paused or
resumed by an external authority. The blocking behavior reflects physical
constraints: the arbiter can only accept one request at a time, and a client
can only proceed once its request has been serviced. The synchronization
expresses *when* interaction is possible, not *who* owns the resource.

By expressing coordination through synchronization rather than scheduling,
Weaver keeps all contention explicit and structural. If access must be
serialized, the serialization logic is visible. If fairness, priority, or
throughput matter, they are properties of the arbitration process itself, not
emergent behavior from a runtime scheduler. This makes shared resources easier
to reason about, easier to verify, and faithful to the realities of hardware
execution.

