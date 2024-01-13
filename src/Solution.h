#pragma once

#include "Circuit.h"

// Represents a wire between two Devices
struct Wire {
	int net;
	Index from;
	Index to;
};

struct VerticalConstraint {
	// index into Circuit::wires
	int wires[2];

	// index into wires
	// undirected: base = -1
	// directed: from = wires[base], to = wires[1-base]
	int base;
};

struct HorizontalConstraint {
	// index into Circuit::devs
	int devices[2];
};

struct LayerAssignment {
	// index into OrderingSolution::wires
	int wire;

	int layer;
};

struct ConstraintAssignment {
	// index into OrderingSolution::vcon
	int constraint;

	int select;
};

struct Solution {
	Solution();
	Solution(const Circuit *ckt);
	~Solution();

	const Circuit *base;

	// This is determined by device ordering
	// index into Circuit::mos
	// -1 means that this is a via
	vector<int> nodes;

	vector<Wire> wires;

	// channel routing constraint graph
	vector<VerticalConstraint> vcon;
	vector<HorizontalConstraint> hcon;

	vector<LayerAssignment> layers;
	vector<ConstraintAssignment> constraints;

	int cost;

	// Push another transistor into the circuit solution
	// place on pmos or nmos stack depending on type
	// create contacts as needed
	// flip device as needed
	void push(int node);

	// Create the constraint graph, filling out vcon and hcon
	void build();

	// Solve the constraint and circuit graph, filling out layers and constraints
	void solve(const Tech &tech, int minCost);

	// Draw the solution by walking the circuit graph
	void draw(const Tech &tech);
};
