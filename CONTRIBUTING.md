# Contributing

## Design Philosophy

The UNIX design philosophy is a good foundation to build on. However, VLSI CAD tools face distinctly different problems with individual tasks often being very complex and requiring configuration information that is quite involved. Today's CAD tools have a myriad of different configuration and data languages that make it difficult to make them interoperate. Therefore Broccoli will modify the UNIX design philosophy as follows:

1. Write programs that do one thing and do it well.
    * Write programs in which each step is separable from every other step through the command line interface.
    * Write programs in C++ for performance.
    * provide a complete C++ library and Python interface for interoperability.
2. Write programs to work together.
    * Write programs that consume configuration input through a Python interface, because that is a scriptable interface.
    * Write programs that consume data input through text streams, because that is a universal interface.
    * Prefer non-interactive interfaces.
3. Write programs that are easy to contribute to
    * Use a parsing expression grammar for all input formats
    * Grammars should prefer specificity over generalization to reduce ambiguity
    * Document thoroughly in comments
    * Always use public structures (we're all adults here)
    * Use CamelCase (structs are capitalized, functions and instances are not)

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

