---
title: Simulating Handshaking Expansions
author: Edward Bingham
date: 2024-09-24
category: Simulation
layout: post
---

**Usage:** `lm sim [options] <*.hse> [sim-file]`

The following tutorials will use this circuit as the example.

**wchb1b.hse**
```
R.f-,R.t-,L.e+; [R.e&~L.f&~L.t];
*[[  R.e & L.f -> R.f+
  [] R.e & L.t -> R.t+
  ]; L.e-; [~R.e&~L.f&~L.t]; R.f-,R.t-; L.e+
 ]||

(L.f-,L.t-; [L.e];  *[[1->L.f+:1->L.t+]; [~L.e]; L.f-,L.t-; [L.e]]||
R.e+; [~R.f&~R.t]; *[[R.f|R.t]; R.e-; [~R.f&~R.t]; R.e+])'1
```

Run the following command to open up the simulation environment:
```
lm sim wchb1b.hse
```

It will bring you to a prompt that looks like this:

```(hsesim)```

From there you can execute any of the following commands:

`<arg>` specifies a required argument

`(arg=value)` specifies an optional argument with a default value

**General**:
 - `help`, `h`             print this message
 - `seed <n>`            set the random seed for the simulation
 - `source <file>`       source and execute a list of commands from a file
 - `save <file>`         save the sequence of fired transitions to a '.sim' file
 - `load <file>`         load a sequence of transitions from a '.sim' file
 - `clear`, `c`            clear any stored sequence and return to random stepping
 - `quit`, `q`             exit the interactive simulation environment

**Running Simulation**:
 - `tokens`, `t`           list the location and state information of every token
 - `enabled`, `e`          return the list of enabled transitions
 - `fire <i>`, `f<i>`      fire the i'th enabled transition
 - `step (N=1)`, `s(N=1)`  step through N transitions (random unless a sequence is loaded)
 - `reset (i)`, `r(i)`     reset the simulator to the initial marking and re-seed (does not clear)

**Setting/Viewing State**:
 - `set <i> <expr>`      execute a transition as if it were local to the i'th token
 - `set <expr>`          execute a transition as if it were remote to all tokens
 - `force <expr>`        execute a transition as if it were local to all tokens

To begin, view set of possible reset states and then pick one.

```
(hsesim)r
(0) {0 9 15} ~R.f&~R.t&L.e&R.e&~L.f&~L.t&~L.f'1&~L.t'1&L.e'1&R.e'1&~R.f'1&~R.t'1
(hsesim)r0
```

If you are simulating a PRS, then there is no "reset" command like this.
Instead, you have to directly toggle your reset signals. If you have only
`_Reset` in your circuit then that looks like this:
```
(prsim) set _Reset-
(prsim) s100
(prsim) set _Reset+
(prsim) s100
```
If you have both `_Reset` and `Reset`, then you can do the following:
```
(prsim) set _Reset-,Reset+
(prsim) s100
(prsim) set _Reset+,Reset-
(prsim) s100
```

Back to the HSE examples:
```
(hsesim)r
(0) {0 9 15} ~R.f&~R.t&L.e&R.e&~L.f&~L.t&~L.f'1&~L.t'1&L.e'1&R.e'1&~R.f'1&~R.t'1
(hsesim)r0
```

You'll notice the ID's `P0`, `P9`, and `P15`. These refer to specific semicolons or
"places" in the hse. To see what these labels refer to, you may run the
following command outside of the interactive simulation environment. See the
**Visualization** section for more details.

```lm plot -l wchb1b.hse -o wchb1b.png```

Now that you have set the current state to a reset state, you may take a look
at the current state.

```
(hsesim)t
R.f-,R.t-,L.e+,R.e+,L.f-,L.t- {
        (0) P0  L.e+ ; [R.e&L.f->...[]R.e&L.t->...]
}
L.f'1-,L.t'1-,L.e'1+ {
        (1) P9  [L.e'1 -> [1->L.f'1+...[]1->L.t'1+...]
}
R.e'1+,R.f'1-,R.t'1- {
        (2) P15 R.e'1+ ; [R.f'1|R.t'1]
}
```

You can view the list of enabled transitions and fire one.

```
(hsesim)e
(0) T10.0:L.t'1+     (1) T9.0:L.f'1+
(hsesim)f1
0       T9.0    1 -> L.f'1+
(hsesim)e
(0) T1.0:R.f+
(hsesim)f0
1       T1.0    R.e&L.f -> R.f+
(hsesim)e
(0) T4.0:L.e-     (1) T16.0:R.e'1-
(hsesim)f0
2       T4.0    R.f -> L.e-
```

You may also step through the simulation. This has two functions. As you
progress through the simulation, the simulator will remember all of the
transitions that have fired and in what order. If you decide to reset the
simulation after simulating and then step, this will re-execute the remembered
list of transitions. If you step through all of the remembered transitions,
it will continue to step randomly. This will execute the transitions in a 
*random order*.

```
(hsesim)e
(0) T10.0:L.t'1+     (1) T9.0:L.f'1+
(hsesim)f1
0       T9.0    1 -> L.f'1+
(hsesim)e
(0) T1.0:R.f+
(hsesim)f0
1       T1.0    R.e&L.f -> R.f+
(hsesim)e
(0) T4.0:L.e-     (1) T16.0:R.e'1-
(hsesim)f0
2       T4.0    R.f -> L.e-
(hsesim)s6
3       T16.0   R.f'1 -> R.e'1-
4       T12.0   L.f'1&~L.e'1 -> L.f'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
(hsesim)r0
(hsesim)s9
0       T9.0    1 -> L.f'1+
1       T1.0    R.e&L.f -> R.f+
2       T4.0    R.f -> L.e-
3       T16.0   R.f'1 -> R.e'1-
4       T12.0   L.f'1&~L.e'1 -> L.f'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
```

You can save this list of transitions to a file and load it up in a later 
simulation.

```
(hsesim)save test
```

```
(hsesim)load test
(hsesim)r0
(hsesim)s9
0       T9.0    1 -> L.f'1+
1       T1.0    R.e&L.f -> R.f+
2       T4.0    R.f -> L.e-
3       T16.0   R.f'1 -> R.e'1-
4       T12.0   L.f'1&~L.e'1 -> L.f'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
```

If you no longer want to simulate the future steps, you may clear them. This 
will remove any transitions in the list ahead of your current step. However,
transitions before your current step will still be remembered.

```
(hsesim)load test
(hsesim)r0
(hsesim)s9
0       T9.0    1 -> L.f'1+
1       T1.0    R.e&L.f -> R.f+
2       T4.0    R.f -> L.e-
3       T16.0   R.f'1 -> R.e'1-
4       T12.0   L.f'1&~L.e'1 -> L.f'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
(hsesim)r0
(hsesim)s3
0       T9.0    1 -> L.f'1+
1       T1.0    R.e&L.f -> R.f+
2       T4.0    R.f -> L.e-
(hsesim)clear
(hsesim)s6
3       T12.0   L.f'1&~L.e'1 -> L.f'1-
4       T16.0   R.f'1 -> R.e'1-
5       T6.0    ~L.e&~R.e&~L.f&~L.t -> R.f-
6       T8.0    ~R.f&~R.t -> L.e+
7       T9.0    ~L.f'1&~L.t'1&L.e'1 -> L.f'1+
8       T18.0   ~R.e'1&~R.f'1&~R.t'1 -> R.e'1+
```
