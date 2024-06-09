#pragma once

#include "Circuit.h"
#include <random>

struct Placer;

// This represents a single transistor placement in the stack
struct Device {
	// index into Circuit::mos
	// if negative, then this represents a "dummy transistor," which is an
	// empty slot in the transistor stack (also a diffusion break)
	int device;

	// if flip is false, then [source gate drain]
	// if flip is true, then [drain gate source]
	bool flip;
};

// This placer was written to implement the relation approach documented in the
// following paper:
//
// Stauffer, André, and Ravi Nair. "Optimal CMOS cell transistor placement: a
// relaxation approach." 1988 IEEE International Conference on Computer-Aided
// Design. IEEE Computer Society, 1988.
//
// In this method, an initial transistor ordering/orientation is randomly
// generated for both the nmos and pmos stacks. Then, a gradient descent
// algorithm is run that selection a range of transistors in one or both stacks
// and flips them. They found this range-flipping transformation to converge
// faster than other possible transformations.
//
// For example given a transistor represented by [source gate drain]
// [a b c][g f e][e d c][g h i]
//        |____________| <-- select this range
// Then flip
// [a b c][c d e][e f g][g h i]
struct Placement {
	Placement();
	Placement(const Circuit *base, int b, int l, int w, int g, std::default_random_engine &rand);
	~Placement();

	// These are needed to be able to compute the cost of the ordering
	const Circuit *base;

	// The cost function for the transistor stack ordering is:
	//
	// b*B^2 + l*L + w*W^2 + g*G
	//
	// Where:
	//
	// B is the minimum number of additional dummy transistors required by this
	//   ordering.
	//
	// L is the sum of the horizontal extents of all nets (L = sum(H_k)). The
	//   horizontal extent is the distance between the right most and left most
	//   terminals (measured by index into the stack) of a net
	//   (H_k = max(Idx_N) - min(Idx_N) for net N)
	//
	// W is the incremental width, which is the difference between the total
	//   width of the current placement and the minimum width over all legal
	//   placements (Wmin). Wmin may be computed by inserting the minimum number
	//   of dummy transistors required to make each stack a semi-Eulerian graph (a
	//   graph that contains a Eulerian path). The graph is constructed such that
	//   each net is a node in the graph and each transistor is an arc from the
	//   source node to the drain node.
	//
	// G is the number of (nmos,pmos) transistor pairs in this placement aligned
	//   at the same index that do not have the same net at their gate. This is an
	//   extra parameter in the cost function that is not included in the original
	//   paper.

	// These are the coefficients on B, L, W, and G that are used in the cost
	// function for the transistor stack ordering. Resonable values for these are:
	// b=12, l=1, w=1, g=10
	int b, l, w, g;

	// This is the minimum width over all legal placements. It is computed in
	// the constructor Placement::Placement() and cached in this structure as an
	// optimization.
	int Wmin;

	// If the nmos stack is bigger than the pmos stack, then d[0] is the
	// difference in size and d[1] is 0. If the pmos stack is bigger than the
	// nmos stack, then d[1] is the difference in size and d[0] is 0. This is
	// computed in the constructor Placement::Placement() and cached in this
	// structure as an optimization.
	array<int, 2> d;

	// stack is indexed by transistor type: Model::NMOS, Model::PMOS, then by
	// index into the placement.
	array<vector<Device>, 2> stack;

	void move(vec4i choice);	
	int score();
	static void solve(const Tech &tech, Circuit *base, int starts=100, int b=12, int l=1, int w=1, int g=10, float step=2.0, float rate=0.02);
};

