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

	// TODO
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

	//-------------------------------------------
	// CONSTRAINT GRAPH CONSTRUCTION
	//-------------------------------------------
	// This keeps track of the transistors in the Circuit we haven't placed on
	// the stack yet. This is filled by the constructor, then successively
	// removed as transistor ordering is done and we place wires in the
	// constraint graph.
	// index into Circuit::mos
	// dangling is indexed by transistor type: Model::NMOS, Model::PMOS
	vector<int> dangling[2];
	// Count the number of contacts we've created so that we can index them in
	// our Wires in the constraint graph.
	int numContacts;

	// Push another transistor into the circuit solution place on pmos or nmos
	// stack depending on type. Create any necessary contacts, and flip the
	// device when possible. These functions build the constraint graph.
	// type is either Model::NMOS or Model::PMOS
	// index is a location in dangling[type]
	bool tryLink(vector<Solution*> &dst, int type, int index);
	bool push(vector<Solution*> &dst, int type, int index);

	// Finish building the constraint graph, filling out vcon and hcon.
	void build();

	//-------------------------------------------
	// CONSTRAINT GRAPH
	//-------------------------------------------
	// This is determined by device ordering
	// stack is indexed by transistor type: Model::NMOS, Model::PMOS
	vector<Wire> stack[2];
	vector<Wire> routes;

	// channel routing constraint graph
	vector<VerticalConstraint> vcon;
	vector<HorizontalConstraint> hcon;

	//-------------------------------------------
	// CONSTRAINT GRAPH SOLVING
	//-------------------------------------------
	vector<LayerAssignment> layers;
	vector<ConstraintAssignment> constraints;

	int cost;

	// Solve the constraint and circuit graph, filling out layers and constraints
	void solve(const Tech &tech, int minCost);

	// Draw the solution by walking the circuit graph
	void draw(const Tech &tech);
};
