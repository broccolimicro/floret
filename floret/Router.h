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

// DESIGN(edward.bingham) Two opposing pins on the two stacks will
// create an ordering constraint on their associated routes.
//
//  |=|==
//  O-|--
//    O--
//  O---- 
//  | O--
//  |=|==
//
// This forces the routes associated with the pins on the lower stack
// to be below the routes associated with the pins on the upper
// stack. If a route has pins in both the upper and lower stacks,
// then it is possible for these ordering constraints to form cycles.
// These cycles must be separated by breaking up the route into two
// separate routes, thus separating the conflicting ordering
// constraints between two different routes. See Router::breakCycles
// for more information about this process.
//
// All of these constraints are directional arcs that point from a
// PMOS pin to an NMOS pin. The participating routes are determined
// in situ. This makes it more expensive to walk the constraint
// graph, but ensures that we can operate on routes without needing
// to re-index the pin constraints.
struct PinConstraint {
	PinConstraint();
	PinConstraint(int from, int to);
	~PinConstraint();

	int from; // index into Router::stack[Model::PMOS]
	int to;   // index into Router::stack[Model::NMOS]
};

// DESIGN(edward.bingham) The following relations between pins are
// not allowed if the three pins are too close for the via
// enclosure and via-pin spacing rules.
//
//                   |====   ====|
//                   |   O   O   |
// |=|=|    O   O    | O |   | O |
// | O |    | O |    O | |   | | O
// O   O    |=|=|    ==|=|   |=|==
//
// The remaining relations are allowed:
//                                                           O 
// |=|=|   |=|=|   |=|=|   O           O                   |=|=|
// | | O   O | |   O | O   | O       O |     O             O   O
// | O       O |     O     | | O   O | |   O | O   O   O
// O           O           |=|=|   |=|=|   |=|=|   |=|=|
//                                                   O
// If this is unavoidable as a result of a pin constraint or
// combination of via constraints, then we need to push the pins
// away from eachother.
//
// |==|==|    O     O
// |  O  |    |  O  |
// O     O    |==|==|
//
// Or go back and look for a new placement.
struct ViaConstraint {
	ViaConstraint();
	ViaConstraint(Index idx);
	~ViaConstraint();

	struct Pin {
		// index to the pin in this constraint
		Index idx;
		// horizontal offset between the previous pin's gate and this pin's via to the route
		int off;
	};

	// index into Router::stack
	Index idx;
	array<vector<ViaConstraint::Pin>, 2> side;
};

// DESIGN(edward.bingham) If two routes are on the same layer, then
// there must be an ordering between them. One must be below the
// other.
//
// O----O----O--O
// | O--|--O |  |
// |=|==|==|=|==|
//
//    OR
//
//   O-----O
// O-|--O--|-O--O
// |=|==|==|=|==|
//
// So, route constraints are initialized with the constraint
// direction unassigned (select=-1), but with the spacing information
// (off) precomputed using our DRC engine (ruler). We then use a
// greedy algorithm to determine a reasonable ordering for these
// constraints within the bounds of the pin and via constraints. See
// Router::assignRouteConstraints for more information about this
// process.
struct RouteConstraint {
	RouteConstraint();
	RouteConstraint(int a, int b, int off0=0, int off1=0, int select=-1);
	~RouteConstraint();

	// index into Router::wires
	int wires[2];

	// derived by Router::solve
	// from = this->wires[select], to = this->wires[1-select]
	int select;

	// pre-computed spacing information. This is the offset from one
	// Wire::layout's origin to the other Wire::layout's origin along
	// the vertical axis
	int off[2];
};

struct Router {
	Router();
	Router(Circuit *base);
	~Router();

	Circuit *base;

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
	void findAndBreakPinCycles();
	void findAndBreakViaCycles();
	int alignPins(int maxDist = -1);
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
};
