# Floret

> *small flower buds at the head of a piece of broccoli.*

Floret is a cell generator designed with advanced nodes in mind. The underlying algorithms are built from the following papers:

1. Stauffer, André, and Ravi Nair. "Optimal CMOS cell transistor placement: a relaxation approach." 1988 IEEE International Conference on Computer-Aided Design. IEEE Computer Society, 1988.
2. Chen, Howard H., and Ernest S. Kuh. "Glitter: A gridless variable-width channel router." IEEE transactions on computer-aided design of integrated circuits and systems 5.4 (1986): 459-465.
3. Deutsch, David N. "A “Dogleg” channel router." Papers on Twenty-five years of electronic design automation. 1988. 111-119.

## Build and Install

Install all dependencies
```
sudo apt install libqhull-dev zlib1g-dev python3.10-dev
git submodule update --init --recursive
```

There are two build systems supported. Tup is preferred, placing floret-linux into the build-linux directory
```
tup init
tup
```

However, we also a Makefile set up, placing floret-linux into the root directory
```
make
```

## Examples

This will create the `cells` directory and run cell layout for all cells in the `test` directory.
```
./build-linux/floret -c cells test/*.spi
```

![cell9](https://github.com/broccolimicro/floret/assets/8902287/7a0c31fb-39dc-45f7-978e-13e30e06b2bb)

## Status

Floret is being tested on Skywater's 130nm process technology node. It is not yet producing DRC clean layouts, but the layouts are often workable with minor manual alterations.
