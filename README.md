# Floret

> *small flower buds at the head of a piece of broccoli.*

Floret is a cell generator designed for advanced nodes.

## Build and Install

Install all dependencies
```
sudo apt install libqhull-dev zlib1g-dev
```

This will build the `floret-linux` binary in the `build-linux` directory:
```
git submodule update --init --recursive
tup init
tup
```

This will install it to `/usr/local/bin`:
```
cp build-linux/floret /usr/local/bin
```

## Examples

This will create the `cells` directory and run cell layout for all cells in the `test` directory.
```
./build-linux/floret -c cells test/*.spi
```

## Cell Layout

**Data Input:**
* A spice netlist
* Optionally a GDS file (partial solution)

**Configuration Input:**
* Design rule set for a process technology node
* Transistor layer specifications
* Heuristic preferences for each step

**Output:**
* GDS file

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

