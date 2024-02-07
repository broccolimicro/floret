#pragma once

#include <map>
#include <string>
#include <vector>
#include <unordered_set>

#include <ruler/Tech.h>
#include <ruler/Layout.h>
#include <ruler/vector.h>

#include "spice.h"

using namespace ruler;
using namespace std;

struct Circuit;

struct Mos {
	Mos();
	Mos(int model, int type);
	~Mos();

	// index into Tech::models
	int model;

	// derived from model
	// Model::NMOS or Model::PMOS
	int type;

	// index into Circuit::nets
	int gate;
	vector<int> ports; // source, drain
	int bulk;

	// loaded in from spice
	map<string, vector<double> > params;
	vec2i size; // length x width
};

struct Net {
	Net();
	Net(string name, bool isIO=false);
	~Net();

	string name;
	// TODO(edward.bingham) split out gates and ports
	array<int, 2> gates;
	array<int, 2> ports;
	bool isIO;
};

struct Index {
	Index();
	Index(int type, int pin);
	~Index();

	// index into Circuit::stack (Model::NMOS or Model::PMOS)
	int type;

	// index into Circuit::stack[type], pin number from left to right
	int pin;
};

// Represents Transistors and Contacts
struct Pin {
	Pin();
	// This pin is a contact
	Pin(int outNet);
	// This pin is a transistor
	Pin(int device, int outNet, int leftNet, int rightNet);
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
	Layout pinLayout;
	Layout conLayout;
	int layer;
	int width;
	int height;
	int off; // from previous pin
	int pos; // absolute position in stack, computed from off
};

// DESIGN(edward.bingham) use this to keep Wire::pins sorted
struct CompareIndex {
	CompareIndex(const Circuit *s);
	~CompareIndex();

	const Circuit *s;

	bool operator()(const Index &i0, const Index &i1);
};

// Represents a wire between two Devices
struct Wire {
	Wire();
	Wire(int net);
	~Wire();

	int net;
	// index into Circuit::stack
	// DESIGN(edward.bingham) We should always keep this array sorted based on
	// horizontal location of the pin in the cell from left to right. This helps
	// us pick pins to dogleg when breaking cycles.
	vector<Index> pins;
	vector<int> level;

	//-------------------------------
	// Layout Information
	//-------------------------------
	Layout layout;

	int left;
	int right;

	int pOffset;
	int nOffset;
	//int pos;
	unordered_set<int> prevNodes;

	void addPin(const Circuit *s, Index pin);
	bool hasPin(const Circuit *s, Index pin, vector<Index>::iterator *out = nullptr);
	int getLevel(int i) const;
};

struct Stack {
	Stack();
	Stack(int type);
	~Stack();

	int type;
	vector<Pin> pins;
	Layout layout;
	
	void push(const Circuit *ckt, int device, bool flip);
	void draw(const Tech &tech);
};

struct Circuit {
	Circuit();
	~Circuit();

	string name;

	// Loaded directly from the spice file
	vector<Net> nets;
	vector<Mos> mos;

	// Computed by the placement system
	array<Stack, 2> stack;

	// Computed by the routing system
	vector<Wire> routes;

	int cellHeight;

	int findNet(string name, bool create=false);
	const Pin &pin(Index i) const;
	Pin &pin(Index i);

	bool loadDevice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev);
	void loadSubckt(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt);

	int pinWidth(const Tech &tech, Index i) const;
	int pinHeight(Index i) const;
	void buildPins(const Tech &tech);
	int alignPins(int coeff=2);
	void updatePinPos();

	void draw(const Tech &tech, Layout &dst);
};

