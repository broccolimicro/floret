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

bool operator<(const Index &i0, const Index &i1);
bool operator==(const Index &i0, const Index &i1);
bool operator!=(const Index &i0, const Index &i1);

// Represents Transistors and Contacts
struct Pin {
	Pin(const Tech &tech);
	// This pin is a contact
	Pin(const Tech &tech, int outNet);
	// This pin is a transistor
	Pin(const Tech &tech, int device, int outNet, int leftNet, int rightNet);
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
	Layout layout;
	int layer;
	int width;
	int height;

	int align;

	// minimum offset from other pins following spacing rules
	// <from, offset>
	// |==|==|
	//  ---->
	//  ->
	map<Index, int> toPin;
	int pos; // current absolute position, computed from off, pin alignment, via constraints
	int lo;
	int hi;

	void offsetToPin(Index pin, int value);

	bool isGate() const;
	bool isContact() const;
};

struct Contact {
	Contact(const Tech &tech);
	Contact(const Tech &tech, Index idx);
	~Contact();
	
	Index idx;
	int left;
	int right;

	Layout layout;

	// <from, offset>
	// |==|==|
	//       O
	//  ---->
	//     ->
	map<Index, int> fromPin;

	// <to, offset>
	// |==|==|
	// O
	//  ---->
	//  ->
	map<Index, int> toPin;

	void offsetFromPin(Index Pin, int value);
	void offsetToPin(Index Pin, int value);
};

bool operator<(const Contact &c0, const Contact &c1);
bool operator==(const Contact &c0, const Contact &c1);
bool operator!=(const Contact &c0, const Contact &c1);

// DESIGN(edward.bingham) use this to keep Wire::pins sorted
struct CompareIndex {
	CompareIndex(const Circuit *s);
	~CompareIndex();

	const Circuit *s;

	bool operator()(const Index &i0, const Index &i1);
	bool operator()(const Contact &c0, const Index &i1);
	bool operator()(const Contact &c0, const Contact &c1);
};

// Represents a wire between two Devices
struct Wire {
	Wire(const Tech &tech);
	Wire(const Tech &tech, int net);
	~Wire();

	// flip(stack ID) for a given stack (Model::NMOS, Model::PMOS, etc), index into Circuit::nets otherwise
	int net;

	// index into Circuit::stack
	// DESIGN(edward.bingham) We should always keep this array sorted based on
	// horizontal location of the pin in the cell from left to right. This helps
	// us pick pins to dogleg when breaking cycles.
	vector<Contact> pins;
	vector<int> level;

	// Used to choose how to break a route to fix a cycle
	int left;
	int right;

	//-------------------------------
	// Layout Information
	//-------------------------------
	Layout layout;

	int pOffset;
	int nOffset;
	
	// <index into Circuit::routes, indices into Router::viaConstraints>
	unordered_set<int> prevNodes;

	void addPin(const Circuit *s, Index pin);
	bool hasPin(const Circuit *s, Index pin, vector<Contact>::iterator *out = nullptr);
	void resortPins(const Circuit *s);
	int getLevel(int i) const;
	bool hasPrev(int r) const;
	bool hasGate(const Circuit *s) const;
	vector<bool> pinTypes() const;
};

struct Stack {
	Stack();
	Stack(int type);
	~Stack();

	int type;
	vector<Pin> pins;
	int route;
	
	void push(const Circuit *ckt, int device, bool flip);
	void draw(const Tech &tech, const Circuit *base, Layout &dst);
};

struct Circuit {
	Circuit(const Tech &tech);
	~Circuit();

	const Tech &tech;

	string name;

	// Loaded directly from the spice file
	vector<Net> nets;
	vector<Mos> mos;

	// Computed by the placement system
	// The third stack in the array is used for virtual pins that facilitate dogleg routing
	array<Stack, 3> stack;

	// Computed by the routing system
	vector<Wire> routes;

	int cellHeight;

	int findNet(string name, bool create=false);
	const Pin &pin(Index i) const;
	Pin &pin(Index i);

	bool loadDevice(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev);
	void loadSubckt(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt);

	int pinWidth(Index i) const;
	int pinHeight(Index i) const;

	void draw(Layout &dst);
};

