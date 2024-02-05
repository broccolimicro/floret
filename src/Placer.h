#pragma once

#include "Circuit.h"

struct Edge {
	Edge();
	Edge(int mos, int gate, vector<int> ports, vec2i size); 
	~Edge();

	// In most cases, this should list a single device
	// However, if there are two devices that are identical, then they should be
	// both counted in the same edge.
	vector<int> mos;

	int gate;
	vec2i size;
	vector<int> verts;
};

struct Vertex {
	Vertex();
	~Vertex();

	// number of ports in this stack minus the number of ports in the opposite stack
	int score;

	// index into Network::edges
	vector<int> gates;

	// index into Network::edges
	// ports[0] contains all of the unpaired ports if it's size is greater than 2
	// each port set should be kept in sorted order to facilitate search
	vector<vector<int> > ports;

	void addPort(int e);
	void addGate(int e);
};

struct Sequence {
	vector<int> mos;
	int source;
	int drain;
};

struct Token {
	Token();
	Token(int edge, int port);
	~Token();

	int edge;
	int port;
};

struct Network {
	Network();
	~Network();

	vector<Vertex> verts;
	vector<Edge> edges;

	vector<Token> next(Token t);

	void buildSupernodes();
	vector<vector<int> > findCycles();
	void breakCycles(vector<vector<int> > cycles);
	vector<Sequence> buildSequences();
	void print(const Circuit *base=nullptr); 
};

struct Placement {
	Placement();
	Placement(const Circuit *base);
	~Placement();

	const Circuit *base;
	// This is determined by device ordering
	// stack is indexed by transistor type: Model::NMOS, Model::PMOS
	array<Stack, 2> stack;

	// This keeps track of the transistors in the Circuit we haven't placed on
	// the stack yet. This is filled by the constructor, then successively
	// removed as transistor ordering is done and we place wires in the
	// constraint graph.
	// index into Circuit::mos
	// dangling is indexed by transistor type: Model::NMOS, Model::PMOS
	array<vector<int>, 2> dangling;

	array<Token, 2> curr;
	array<int, 2> alignIdx;

	const Pin &pin(Index i) const;
	Pin &pin(Index i);

	// Push another transistor into the circuit solution place on pmos or nmos
	// stack depending on type. Create any necessary contacts, and flip the
	// device when possible. These functions build the constraint graph.
	// type is either Model::NMOS or Model::PMOS
	// index is a location in dangling[type]
	bool tryLink(vector<Placement> &dst, int type, int index);
	bool push(vector<Placement> &dst, int type, int index);

	int pinWidth(const Tech &tech, Index i) const;
	int pinHeight(Index i) const;

	void buildPins(const Tech &tech);
	int countAligned();
	int alignPins(int coeff=2);
	void updatePinPos();
};

struct Placer {
	Placer();
	Placer(Circuit *base);
	~Placer();

	Circuit *base;

	array<Network, 2> stack;

	void build(const Circuit *base);

	vector<array<Token, 2> > findStart();
	void matchSequencing();
	void breakCycles();
	void buildSequences();
	void buildConstraints();
	void solveConstraints();
	void fixDangling();
	Placement searchOrderings(const Tech &tech);

	void solve(const Tech &tech);
	void print();
};

