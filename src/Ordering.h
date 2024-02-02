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

	// index into Eulerian::edges
	vector<int> gates;

	// index into Eulerian::edges
	// ports[0] contains all of the unpaired ports if it's size is greater than 2
	// each port set should be kept in sorted order to facilitate search
	vector<vector<int> > ports;

	void addPort(int e);
	void addGate(int e);
};

struct Eulerian {
	Eulerian();
	~Eulerian();

	vector<Vertex> verts;
	vector<Edge> edges;
	int brk;
};

struct Ordering {
	Ordering();
	~Ordering();

	array<Eulerian, 2> mos;

	void build(const Circuit *base);	
	void solve(int radix=2);
};

