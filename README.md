# Floret

> *small flower buds at the head of a piece of broccoli.*

> [!NOTE]  
> This project has been merged into the [Loom Synthesis
> Engine](https://github.com/broccolimicro/loom). Updates to Floret will likely
> be infrequent as a result. See Loom for a more up-to-date version.

Floret is an automated custom cell generator. The underlying algorithms are built from the following papers:

1. Stauffer, André, and Ravi Nair. "Optimal CMOS cell transistor placement: a relaxation approach." 1988 IEEE International Conference on Computer-Aided Design. IEEE Computer Society, 1988.
2. Chen, Howard H., and Ernest S. Kuh. "Glitter: A gridless variable-width channel router." IEEE transactions on computer-aided design of integrated circuits and systems 5.4 (1986): 459-465.
3. Deutsch, David N. "A “Dogleg” channel router." Papers on Twenty-five years of electronic design automation. 1988. 111-119.

## Build and Install

Install all dependencies
```
sudo apt install libqhull-dev zlib1g-dev python3.10-dev
git submodule update --init --recursive
```

Build the executable
```
make
```

## Examples

See [Releases](https://github.com/broccolimicro/floret/releases) for example generated layouts.

This will generate a GDS library with a nand gate in `nand.gds` using the sky130 tech file provided in the tech directory.
```
./floret-linux -p -c cells -t tech/sky130.py test/nand.spi
```

![cell](https://github.com/broccolimicro/floret/assets/8902287/9085fadf-f1ff-4f82-a233-061a880ca9d2)

## Status

Floret is being tested on Skywater's 130nm process technology node, layouts are largely DRC/LVS clean.
