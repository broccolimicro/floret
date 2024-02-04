#pragma once

#include <ruler/Layout.h>
#include "Circuit.h"
#include "Placer.h"
#include <set>
#include <unordered_set>
#include <array>
#include <vector>

using namespace std;

struct Router;

struct Index {
	Index();
	Index(int type, int pin);
	~Index();

	// index into Router::stack (Model::NMOS or Model::PMOS)
	int type;

	// index into Router::stack[type], pin number from left to right
	int pin;
};

// DESIGN(edward.bingham) use this to keep Wire::pins sorted
struct CompareIndex {
	CompareIndex(const Router *s);
	~CompareIndex();

	const Router *s;

	bool operator()(const Index &i0, const Index &i1);
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

// Represents a wire between two Devices
struct Wire {
	Wire();
	Wire(int net);
	~Wire();

	int net;
	// index into Router::stack
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
	unordered_set<int> prevNodes;

	void addPin(const Router *s, Index pin);
	bool hasPin(const Router *s, Index pin, vector<Index>::iterator *out = nullptr);
	int getLevel(int i) const;
};

struct PinConstraint {
	PinConstraint();
	PinConstraint(int from, int to);
	~PinConstraint();

	int from; // index into Router::stack[Model::PMOS]
	int to;   // index into Router::stack[Model::NMOS]
};

struct ViaConstraint {
	ViaConstraint();
	ViaConstraint(int type, int idx, int fromIdx, int fromOff, int toIdx, int toOff);
	~ViaConstraint();

	struct Pin {
		// index to the pin in this constraint
		int idx;
		// horizontal offset between the previous pin's gate and this pin's via to the route
		int off;
	};

	int type;
	// index into Router::stack[type]
	int idx;

	ViaConstraint::Pin from;
	ViaConstraint::Pin to;

	// direction of the constraints
	int select;
};

struct RouteConstraint {
	RouteConstraint();
	RouteConstraint(int a, int b, int off0=0, int off1=0, int select=-1);
	~RouteConstraint();

	// index into Router::wires
	int wires[2];

	// derived by Router::solve
	// from = wires[select], to = wires[1-select]
	int select;

	//-------------------------------
	// Layout Information
	//-------------------------------
	int off[2];
};

struct Router {
	Router();
	Router(const Circuit *ckt);
	~Router();

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
	array<vector<int>, 2> dangling;
	array<Token, 2> curr;

	// Push another transistor into the circuit solution place on pmos or nmos
	// stack depending on type. Create any necessary contacts, and flip the
	// device when possible. These functions build the constraint graph.
	// type is either Model::NMOS or Model::PMOS
	// index is a location in dangling[type]
	bool tryLink(vector<Router> &dst, int type, int index);
	bool push(vector<Router> &dst, int type, int index);

	//-------------------------------------------
	// CONSTRAINT GRAPH
	//-------------------------------------------
	// This is determined by device ordering
	// stack is indexed by transistor type: Model::NMOS, Model::PMOS
	array<vector<Pin>, 2> stack;
	array<int, 2> alignIdx;
	vector<Wire> routes;
	// Route pairs that need to be connected via A*
	// index into Router::routes
	vector<pair<int, int> > aStar;
	enum {
		PMOS_STACK=-1,
		NMOS_STACK=-2,
	};
	array<Layout, 2> stackLayout;

	// channel routing constraint graph
	vector<PinConstraint> pinConstraints;
	vector<RouteConstraint> routeConstraints;
	vector<ViaConstraint> viaConstraints;

	int cycleCount;
	int cellHeight;
	int cost;
	
	const Pin &pin(Index i) const;
	Pin &pin(Index i);
	int pinWidth(const Tech &tech, Index i) const;
	int pinHeight(Index i) const;

	// Finish building the constraint graph, filling out vcon and hcon.
	void delRoute(int route);
	void buildPins(const Tech &tech);
	int countAligned();
	int alignPins(int coeff=2);
	void updatePinPos();
	void buildPinConstraints(const Tech &tech);
	void buildViaConstraints(const Tech &tech);
	void buildRoutes();
	bool findCycles(vector<vector<int> > &cycles, int maxCycles = -1);
	void breakRoute(int route, set<int> cycleRoutes);
	void breakCycles(vector<vector<int> > cycles);
	bool findAndBreakCycles(int maxCycles);
	void drawStacks(const Tech &tech);
	void drawRoutes(const Tech &tech);
	void buildStackConstraints(const Tech &tech);
	void buildRouteConstraints(const Tech &tech);
	vector<int> findTop();
	vector<int> findBottom();
	void zeroWeights();
	void clearPrev();
	void resetGraph(const Tech &tech);
	void buildPrevNodes(vector<int> start=vector<int>(1, PMOS_STACK));
	void buildPOffsets(const Tech &tech, vector<int> start=vector<int>(1, PMOS_STACK));
	void buildNOffsets(const Tech &tech, vector<int> start=vector<int>(1, NMOS_STACK));
	void assignRouteConstraints(const Tech &tech);
	void lowerRoutes();
	void updateRouteConstraints(const Tech &tech);
	bool computeCost(int maxCost);

	// Solve the constraint and circuit graph, filling out layers and constraints
	bool solve(const Tech &tech, int maxCost, int maxCycles);

	// Print the solution description
	void print();
	void draw(const Tech &tech, Layout &dst); 
};