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

## Design Philosophy

The UNIX design philosophy is a good foundation to build on. However, VLSI CAD tools face distinctly different problems with individual tasks often being very complex and requiring configuration information that is quite involved. Today's CAD tools have a myriad of different configuration and data languages that make it difficult to make them interoperate. Therefore Broccoli will modify the UNIX design philosophy as follows:

1. Write programs that do one thing and do it well.
  a. Write programs in which each step is separable from every other step through the command line interface.
	b. Write programs in C++ for performance.
  c. provide a complete C++ library and Python interface for interoperability.
2. Write programs to work together.
  a. Write programs that consume configuration input through a Python interface, because that is a scriptable interface.
	b. Write programs that consume data input through text streams, because that is a universal interface.
	c. Prefer non-interactive interfaces.
3. Write programs that are easy to contribute to
  a. Use a parsing expression grammar for all input formats
  b. Document thoroughly in comments
  c. Always use public structures (we're all adults here)
  d. Use CamelCase (structs are capitalized, functions and instances are not)

## Documentation Format

**Structs**
*Only needed for more complex structures*
1. Purpose of struct
2. Example usage of struct

**Functions**
*Only needed for more complex functions*
1. Description of operation
2. Inputs (format, purpose)
3. Outputs (format, purpose)
4. Example usage of function

**Design Decisions**
1. Unique label for decision
2. Who made this decision?
3. What did we choose?
4. What were the alternatives?
5. Why did we choose this?
6. What other design decisions interact with this?

```
// DESIGN(edward.bingham) myfunc.1
// ...
```

**Todos**
1. Who created this todo?
2. What needs to be done?
3. Why does this need to be done?

```
// TODO(edward.bingham) description...
```

