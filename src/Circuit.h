#pragma once

#include <map>
#include <string>
#include <vector>
#include "Tech.h"
#include "spice.h"

using namespace std;

struct Transistor {
	Transistor();
	Transistor(int model);
	~Transistor();

	// index into Tech::models
	int model;

	enum Port {
		DRAIN = 0,
		GATE = 1,
		SOURCE = 2,
		BULK = 3
	};

	// loaded in from spice
	map<string, vector<double> > params;
};

struct Index {
	Index();
	Index(int device, int port = 0);
	~Index();

	// index into Circuit::mos or Solution::nodes
	int device;

	// See Transistor::Port, use 0 for vias
	int port;
};

struct Net {
	Net();
	Net(string name);
	~Net();

	string name;
	vector<Index> ports;
};

struct Solution;

struct Circuit {
	Circuit();
	~Circuit();

	string name;

	// Loaded directly from the spice file
	vector<Net> nets;
	vector<Transistor> mos;

	Solution *layout;

	int findNet(string name, bool create=false);

	bool loadDevice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev);
	void loadSubckt(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt);

	void solve(const Tech &tech);
};

