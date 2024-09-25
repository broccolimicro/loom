---
title: Languages
author: Edward Bingham
date: 2024-09-24
category: Language
layout: post
order: 1
---

Loom supports multiple different languages for different stages of the synthesis.

## Communicating Hardware Processes (CHP)

Write your behavioral specification in CHP. Your behavior will then be broken
into processes and pipeline stages during synthesis. The semantics of the
language is fairly similar to Golang.

## Hand Shaking Expansions (HSE)

Write your functional specification in HSE. This is a very limited version of
CHP in which only wire-level operations are allowed. Think of this like an
intermediate representation (IR) language for your circuit design.

## Production Rules (PRS)

Write your structural specification in PRS. This specifies the pull up and pull
down gates of each of your signals. This is like an assembly language for
circuit design.

## Spice

This specifies a list of devices. Think of this like the compiled binary.

## GDS

This specifies the layout of your circuit. Its the final output of the toolset.
