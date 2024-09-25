---
title: Compile HSE to PRS
author: Edward Bingham
date: 2024-09-24
category: Compilation
layout: post
---
 
Synthesize the production rules that implement the behavioral description.

**Usage:** `lm [options] <file>`

**Options:**
 - `--no-cmos`      uses bubble reshuffling instead of state variables to make the production rules cmos implementable, combine with `--rules` to avoid cmos implementability altogether.
 - `--all`          save all intermediate stages
 - `-o,--out`       set the filename prefix for the saved intermediate stages
 - `-g,--graph`     save the elaborated astg
 - `-c,--conflicts` print the conflicts to stdout
 - `-e,--encode`    save the complete state encoded astg
 - `-r,--rules`     save the synthesized production rules
 - `-b,--bubble`    save the bubble reshuffled production rules
 - `-s,--size`      save the sized production rules

**Supported file formats:**
 - `*.chp`          communicating hardware processes
 - `*.hse`          handshaking expansions
 - `*.prs`          production rules
 - `*.astg`         asynchronous signal transition graph
 - `*.spi`          spice netlist

If you just want to generate the final circuit, simply run the following command:

```
lm wchb1b.hse
```

This is the generated production rule set.
```
v0->R.f-
~v0->R.f+
v1->R.t-
~v1->R.t+
R.t|R.f->L.e-
~R.f&~R.t->L.e+
v2->L.f'1-
~v2->L.f'1+
v3->L.t'1-
~v3->L.t'1+
R.t'1|R.f'1->R.e'1-
~R.f'1&~R.t'1->R.e'1+
R.e&L.f&_Reset->v0-
~R.e&~L.f|~_Reset->v0+
R.e&L.t&_Reset->v1-
~R.e&~L.t|~_Reset->v1+
L.e'1&v3&_Reset->v2- {v3}
~L.e'1|~v3|~_Reset->v2+
L.e'1&v2&_Reset->v3- {v2}
~L.e'1|~v2|~_Reset->v3+
```

This will print the production rules out on the console. If you want to see all of the intermediate steps, run this:

```
lm --all --out wchb1b wchb1b.hse
```

This will save the elaborated state space to `wchb1b_predicate.astg`, save the
complete state coded state space to `wchb1b_complete.astg`, and the final
production rules in `wchb1b.prs`. It will also display the set of state
conflicts found on the original specification to the console.

For example, this is what `wchb1b_complete.astg` looks like:
```
.model hse
.internal R.f R.t L.e R.e L.f L.t L.f'1 L.t'1 L.e'1 R.e'1 R.f'1 R.t'1 v0 v1 v2 v3
.predicate
p0 ~R.f&~R.t&L.e&~L.t&v0&v1|~R.f&~R.t&L.e&~L.f&L.t&v0&v1
p1 R.f&~R.t&~L.e&~L.t&~v0&v1
p2 R.f&~R.t&L.e&L.f&~L.t&~v0&v1|~R.f&R.t&L.e&~L.f&L.t&v0&~v1
p3 ~R.f&~R.t&~L.e&~L.f&~L.t&v0&v1|~R.f&R.t&~L.e&~R.e&~L.f&~L.t&v0&v1|~R.f&R.t&~L.e&~L.f&v0&~v1
p4 ~R.f&~R.t&~L.e&~L.f&~L.t&v0&v1|R.f&~R.t&~L.e&~L.t&~v0&v1|R.f&~R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p5 L.f'1&~L.t'1&~v2&v3
p6 ~L.f'1&L.t'1&v2&~v3|~L.f'1&~L.t'1&v2&v3|~L.f'1&L.t'1&~L.e'1&v2&v3
p7 ~L.f'1&~L.t'1&v2&v3|L.f'1&~L.t'1&~L.e'1&v2&v3|L.f'1&~L.t'1&~v2&v3
p8 R.e'1&~R.f'1|R.e'1&R.f'1&~R.t'1
p9 ~R.e'1&~R.f'1|~R.e'1&R.f'1&~R.t'1
p10 ~R.f&R.t&~L.e&~L.f&v0&~v1
p11 ~L.f'1&L.t'1&v2&~v3
p12 ~R.f&~R.t&L.e&R.e&L.f&~L.t&~v0&v1
p13 R.f&~R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p14 ~R.f&~R.t&L.e&R.e&~L.f&L.t&v0&~v1
p15 ~R.f&R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p16 ~L.f'1&~L.t'1&L.e'1&~v2&v3
p17 L.f'1&~L.t'1&~L.e'1&v2&v3
p18 ~L.f'1&~L.t'1&L.e'1&v2&~v3
p19 ~L.f'1&L.t'1&~L.e'1&v2&v3
.effective
p0 ~R.f&~R.t&L.e&~L.f&~L.t&v0&v1|~R.f&~R.t&L.e&~R.e&L.f&~L.t&v0&v1|~R.f&~R.t&L.e&~R.e&~L.f&L.t&v0&v1
p1 R.f&~R.t&~L.e&R.e&~L.t&~v0&v1|R.f&~R.t&~L.e&~R.e&L.f&~L.t&~v0&v1
p3 ~R.f&R.t&~L.e&~L.f&v0&~v1|~R.f&R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p4 R.f&~R.t&~L.e&~L.t&~v0&v1|R.f&~R.t&~L.e&~R.e&~L.f&~L.t&v0&v1
p5 L.f'1&~L.t'1&L.e'1&~v2&v3
p6 ~L.f'1&L.t'1&v2&~v3|~L.f'1&~L.e'1&v2&v3
p7 L.f'1&~L.t'1&~v2&v3|~L.t'1&~L.e'1&v2&v3
p8 R.e'1&~R.f'1&~R.t'1
p9 ~R.e'1&R.f'1&~R.t'1|~R.e'1&~R.f'1&R.t'1
p10 ~R.f&R.t&~L.e&~L.f&L.t&v0&~v1|~R.f&R.t&~L.e&R.e&~L.f&~L.t&v0&~v1
p11 ~L.f'1&L.t'1&L.e'1&v2&~v3
.graph
1->R.f+/0 p2
1->R.t+/1 p2
1->L.e-/2 p1 p10
1->R.f-/3 p3
1->R.t-/4 p4
1->L.e+/5 p0
1->L.f'1+/6 p5 p11
1->L.t'1+/7 p5 p11
1->L.f'1-/8 p6
1->L.t'1-/9 p7
R.f'1|R.t'1->R.e'1-/10 p9
~R.f'1&~R.t'1->R.e'1+/11 p8
R.e&L.f->v0-/12 p12
~R.e&~L.f&~L.t->v0+/13 p13
R.e&L.t->v1-/14 p14
~R.e&~L.f&~L.t->v1+/15 p15
L.e'1->v2-/16 p16
~L.e'1->v2+/17 p17
L.e'1->v3-/18 p18
~L.e'1->v3+/19 p19
p0 R.e&L.f->v0-/12 R.e&L.t->v1-/14
p1 ~R.e&~L.f&~L.t->v0+/13
p2 1->L.e-/2
p3 1->L.e+/5
p4 1->L.e+/5
p5 ~L.e'1->v2+/17
p6 L.e'1->v2-/16 L.e'1->v3-/18
p7 L.e'1->v2-/16 L.e'1->v3-/18
p8 R.f'1|R.t'1->R.e'1-/10
p9 ~R.f'1&~R.t'1->R.e'1+/11
p10 ~R.e&~L.f&~L.t->v1+/15
p11 ~L.e'1->v3+/19
p12 1->R.f+/0
p13 1->R.f-/3
p14 1->R.t+/1
p15 1->R.t-/4
p16 1->L.f'1+/6
p17 1->L.f'1-/8
p18 1->L.t'1+/7
p19 1->L.t'1-/9
.arbiter {p6 p7}
.marking {[R.f-,R.t-,L.e+,R.e+,L.f-,L.t-,L.f'1-,L.t'1-,L.e'1+,R.e'1+,R.f'1-,R.t'1-,v0+,v1+,v2+,v3+] p0 p8 p7 p6}
.end
```

This command also accepts `*.spi` files for automated cell layout.

```
lm nand.spi sky130.py
klayout nand.gds &
```

See [sky130.py](https://github.com/broccolimicro/floret/blob/main/tech/sky130.py) 

