# Floret

> *small flower buds at the head of a piece of broccoli.*

Floret is a cell generator designed with advanced nodes in mind. The underlying algorithms are built from the following papers:

1. Stauffer, André, and Ravi Nair. "Optimal CMOS cell transistor placement: a relaxation approach." 1988 IEEE International Conference on Computer-Aided Design. IEEE Computer Society, 1988.
2. Chen, Howard H., and Ernest S. Kuh. "Glitter: A gridless variable-width channel router." IEEE transactions on computer-aided design of integrated circuits and systems 5.4 (1986): 459-465.
3. Deutsch, David N. "A “Dogleg” channel router." Papers on Twenty-five years of electronic design automation. 1988. 111-119.

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

## Status

Floret is being tested on Skywater's 130nm process technology node. It is not yet producing DRC clean layouts, but the layouts are often workable with minor manual alterations. There are two more things that need to be done to generate DRC clean layouts.

1. I need to implement constraints for pins and vias that are next to eachother on the same stack to manage the interaction between via enclosure rules and spacing rules between the via and the pin.
2. I need to implement a more advanced DRC checking engine that can handle more complex DRC rules to check poly against diffusion without causing havok with the transistor placement spacing.
