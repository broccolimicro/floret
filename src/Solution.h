#pragma once

#include "Circuit.h"

struct Index {
	Index();
	Index(int type, int pin);
	~Index();

	// index into Solution::stack (Model::NMOS or Model::PMOS)
	int type;

	// index into Solution::stack[type], pin number from left to right
	int pin;
};

// Represents Transistors and Contacts
struct Pin {
	Pin();
	Pin(int device, int outNet, int leftNet=-1, int rightNet=-1);
	~Pin();

	// inNet == outNet == gateNet for Contacts
	// inNet and outNet represent source and drain depending on flip for Transistors
	int leftNet;
	int outNet;
	int rightNet;

	// index into Circuit::mos for Transistors
	// negative for Contacts
	int device;

	//-------------------------------
	// Layout Information
	//-------------------------------
	int layer;
	int width;
	int height;
	int off; // from previous pin
	int pos; // absolute position in stack, computed from off
};

// Represents a wire between two Devices
struct Wire {
	Wire();
	Wire(int net);
	~Wire();

	int net;
	// index into Solution::stack
	vector<Index> pins;

	//-------------------------------
	// Layout Information
	//-------------------------------
	int layer;
	int height;
	int pos; // absolute vertical position in cell, top down

	int left;
	int right;
};

struct Constraint {
	Constraint();
	Constraint(Index pin0, int wire0, Index pin1, int wire1, int base);
	~Constraint();

	// index into Circuit::wires
	int wires[2];
	// index into Solution::stack
	Index pins[2];

	// index into wires
	// undirected: base = -1
	// directed: from = wires[base], to = wires[1-base]
	int base;

	// derived by Solution::solve
	int select;

	//-------------------------------
	// Layout Information
	//-------------------------------
	int off;
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
	void build(const Tech &tech);

	//-------------------------------------------
	// CONSTRAINT GRAPH
	//-------------------------------------------
	// This is determined by device ordering
	// stack is indexed by transistor type: Model::NMOS, Model::PMOS
	vector<Pin> stack[2];
	vector<Wire> routes;

	// channel routing constraint graph
	vector<Constraint> constraints;

	int cost;

	// Solve the constraint and circuit graph, filling out layers and constraints
	void solve(const Tech &tech, int minCost);

	// Draw the solution by walking the circuit graph
	void draw(const Tech &tech);
};
