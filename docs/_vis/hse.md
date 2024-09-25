---
title: Visualizing HSE
author: Edward Bingham
date: 2024-09-24
category: Visualization
layout: post
---
 
Create visual representations of the circuit or behavior.

**Usage:** `lm plot [options] file...`

**Options:**
 - `-o`              Specify the output file name, formats other than 'dot' are passed onto graphviz dot for rendering
 - `-l,--labels`     Show the IDs for each place, transition, and arc
 - `-lr,--leftright` Render the graph from left to right
 - `-e,--effective`  Show the effective encoding of each place
 - `-p,--predicate`  Show the predicate of each place
 - `-r,--raw`        Do not post-process the graph
 - `-s,--sync`       Render half synchronization actions

Use the following command to show the elaborated state space from the generated astg.

```
lm plot -p wchb1b_predicate.astg -o wchb1b.png
```

Use this command to show the labels associated with every place, transition, and arc.

```
lm plot -l wchb1b.hse -o wchb1b.png
```

Use the following command to show the elaborated state space of the complete state coding.

```
lm plot -p wchb1b_complete.astg -o wchb1b.png
```


