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
	vector<int> ports;
};

struct Vertex {
	Vertex();
	~Vertex();

	// number of ports in this stack minus the number of ports in the opposite stack
	int score;

	// index into Eulerian::edges
	vector<int> gates;

	// index into Eulerian::edges
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

struct Eulerian {
	Eulerian();
	~Eulerian();

	vector<Vertex> verts;
	vector<Edge> edges;

	vector<Token> next(Token t);

	void buildSupernodes();
	vector<vector<int> > findCycles();
	void breakCycles(vector<vector<int> > cycles);
	vector<Sequence> buildSequences();
	void print(const Circuit *base=nullptr); 
};

struct Placer {
	Placer();
	~Placer();

	array<Eulerian, 2> mos;

	void build(const Circuit *base);

	vector<array<Token, 2> > findStart();
	void matchSequencing();
	void breakCycles();
	void buildSequences();
	void buildConstraints();
	void solveConstraints();
	void fixDangling();

	void solve(int radix=2);
	void print(const Circuit *base=nullptr);
};

