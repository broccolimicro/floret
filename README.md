# Floret

> *Spout-like structures at the head of a piece of broccoli.*

Floret is a cell generator designed for advanced nodes.

## Install

TODO

## Examples

TODO

## Cell Layout

**Data Input:**
* A spice netlist
* Optionally a Rectangle file (partial solution)

**Configuration Input:**
* Design rule set for a process technology node
* Transistor layer specifications
* Heuristic preferences for each step

**Output:**
* Rectangle file

**Steps:**
1. Read input and configuration
2. Determine transistor order for pull-up and pull-down stacks
    * Align transistor gates
    * Align source and drain connections
    * Create sufficient space for vias
3. Route over the stacks
4. Route between stacks
5. Identify DRC violations
6. Modify decisions from earlier steps to fix DRC violation and repeat
7. Draw geometry
8. Emit output

## Status

TODO

