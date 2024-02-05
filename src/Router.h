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
	Router(Circuit *base);
	~Router();

	Circuit *base;

	// Route pairs that need to be connected via A*
	// index into Router::routes
	vector<pair<int, int> > aStar;
	enum {
		PMOS_STACK=-1,
		NMOS_STACK=-2,
	};

	// channel routing constraint graph
	vector<PinConstraint> pinConstraints;
	vector<RouteConstraint> routeConstraints;
	vector<ViaConstraint> viaConstraints;

	vector<Wire> routes;

	int cellHeight;
	int cycleCount;
	int cost;
	
	// Finish building the constraint graph, filling out vcon and hcon.
	void delRoute(int route);
	void buildPinConstraints(const Tech &tech);
	void buildViaConstraints(const Tech &tech);
	void buildRoutes();
	void findCycles(vector<vector<int> > &cycles);
	void breakRoute(int route, set<int> cycleRoutes);
	void breakCycles(vector<vector<int> > cycles);
	void findAndBreakCycles();
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
	int computeCost();

	// Solve the constraint and circuit graph, filling out layers and constraints
	int solve(const Tech &tech);

	// Print the solution description
	void print();
	void draw(const Tech &tech, Layout &dst); 
};
