#pragma once

#include "Circuit.h"

struct Placer;

struct Device {
	int device;
	bool flip;
};

struct Placement {
	Placement();
	Placement(const Circuit *base, int b, int l, int w, int g);
	~Placement();

	// These are needed to be able to compute the cost of the ordering
	const Circuit *base;
	int b, l, w, g;
	int Wmin;
	array<int, 2> d;

	// stack is indexed by transistor type: Model::NMOS, Model::PMOS
	array<vector<Device>, 2> stack;

	void move(vec4i choice);	
	int score();
	static void solve(const Tech &tech, Circuit *base, int starts=100, int b=12, int l=1, int w=1, int g=3, float step=1.0, float rate=0.1);
};

