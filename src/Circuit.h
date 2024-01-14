#pragma once

#include <map>
#include <string>
#include <vector>
#include "Tech.h"
#include "spice.h"

using namespace std;

struct Mos {
	Mos();
	Mos(int model, int type);
	~Mos();

	// index into Tech::models
	int model;

	// derived from model
	// Model::NMOS or Model::PMOS
	int type;

	enum Port {
		DRAIN = 0,
		GATE = 1,
		SOURCE = 2,
		BULK = 3
	};

	// index into Circuit::nets
	vector<int> ports;

	// loaded in from spice
	map<string, vector<double> > params;
};

struct Index {
	Index();
	Index(int device, int port = 0);
	~Index();

	// index into Circuit::mos
	// negative numbers for vias
	int device;

	// See Mos::Port, use 0 for vias
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
	vector<Mos> mos;

	Solution *layout;

	int findNet(string name, bool create=false);

	bool loadDevice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev);
	void loadSubckt(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt);

	void solve(const Tech &tech);
};

