#include <algorithm>
#include <unordered_set>
#include <set>
#include <list>

#include "Router.h"
#include "Timer.h"
#include "Draw.h"

int flip(int idx) {
	return -idx-1;
}

PinConstraint::PinConstraint() {
	from = -1;
	to = -1;
}

PinConstraint::PinConstraint(int from, int to) {
	this->from = from;
	this->to = to;
}

PinConstraint::~PinConstraint() {
}

RouteConstraint::RouteConstraint() {
	wires[0] = -1;
	wires[1] = -1;
	select = -1;
	off[0] = 0;
	off[1] = 0;
}

RouteConstraint::RouteConstraint(int a, int b, int off0, int off1, int select) {
	this->wires[0] = a;
	this->wires[1] = b;
	this->select = select;
	this->off[0] = off0;
	this->off[1] = off1;
}

RouteConstraint::~RouteConstraint() {
}

ViaConstraint::ViaConstraint() {
}

ViaConstraint::ViaConstraint(Index idx) {
	this->idx = idx;
}

ViaConstraint::~ViaConstraint() {
}

RouteGroupConstraint::RouteGroupConstraint() {
	wire = -1;
}

RouteGroupConstraint::RouteGroupConstraint(int wire, Index pin) {
	this->wire = wire;
	this->pin = pin;
}

RouteGroupConstraint::~RouteGroupConstraint() {
}

Router::Router() {
	base = nullptr;
	cycleCount = 0;
	cellHeight = 0;
	cost = 0;
}

Router::Router(Circuit *base) {
	this->base = base;
	this->cycleCount = 0;
	this->cellHeight = 0;
	this->cost = 0;
}

Router::~Router() {
}


void Router::delRoute(int route) {
	for (int i = (int)routeConstraints.size()-1; i >= 0; i--) {
		if (routeConstraints[i].wires[0] == route or routeConstraints[i].wires[1] == route) {
			routeConstraints.erase(routeConstraints.begin()+i);
		} else {
			if (routeConstraints[i].wires[0] > route) {
				routeConstraints[i].wires[0]--;
			}
			if (routeConstraints[i].wires[1] > route) {
				routeConstraints[i].wires[1]--;
			}
		}
	}

	routes.erase(routes.begin()+route);
}

void Router::buildPinConstraints(const Tech &tech, int level) {
	pinConstraints.clear();
	// Compute the pin constraints
	// TODO(edward.bingham) this could be more efficiently done as a 1d rectangle overlap problem
	for (int p = 0; p < (int)base->stack[Model::PMOS].pins.size(); p++) {
		for (int n = 0; n < (int)base->stack[Model::NMOS].pins.size(); n++) {
			int off = 0;
			Pin &pmos = base->stack[Model::PMOS].pins[p];
			Pin &nmos = base->stack[Model::NMOS].pins[n];
			if (level == 0 and pmos.outNet != nmos.outNet and
				minOffset(&off, tech, 1, pmos.layout.layers, pmos.pos,
				                         nmos.layout.layers, nmos.pos,
				          Layout::IGNORE, Layout::MERGENET)) {
				pinConstraints.push_back(PinConstraint(p, n));
			} /*else if (level == 1 and pmos.outNet != nmos.outNet and
				(minOffset(&off, tech, 1, pmos.conLayout.layers, pmos.pos,
				                         nmos.layout.layers, nmos.pos,
				          Layout::IGNORE, Layout::MERGENET) or
				minOffset(&off, tech, 1, pmos.layout.layers, pmos.pos,
				                         nmos.conLayout.layers, nmos.pos,
				          Layout::IGNORE, Layout::MERGENET))) {
				pinConstraints.push_back(PinConstraint(p, n));
			} else if (level == 2 and pmos.outNet != nmos.outNet and
				minOffset(&off, tech, 1, pmos.conLayout.layers, pmos.pos,
				                         nmos.conLayout.layers, nmos.pos,
				          Layout::IGNORE, Layout::MERGENET)) {
				pinConstraints.push_back(PinConstraint(p, n));
			}*/
		}
	}
}

void Router::buildViaConstraints(const Tech &tech) {
	viaConstraints.clear();
	// Compute via constraints
	/*for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			if (base->stack[type].pins[i].conLayout.layers.size() == 0) {
				continue;
			}

			viaConstraints.push_back(ViaConstraint(Index(type, i)));
	
			for (int j = i-1; j >= 0; j--) {
				int off = 0;
				if (minOffset(&off, tech, 0, base->stack[type].pins[j].layout.layers, 0, base->stack[type].pins[i].conLayout.layers, base->stack[type].pins[j].height/2, Layout::IGNORE, Layout::MERGENET)) {
					viaConstraints.back().side[0].push_back(ViaConstraint::Pin{Index(type, j), off});
				}
			}

			for (int j = i+1; j < (int)base->stack[type].pins.size(); j++) {
				int off = 0;
				if (minOffset(&off, tech, 0, base->stack[type].pins[i].conLayout.layers, base->stack[type].pins[j].height/2, base->stack[type].pins[j].layout.layers, 0, Layout::IGNORE, Layout::MERGENET)) {
					viaConstraints.back().side[1].push_back(ViaConstraint::Pin{Index(type, j), off});
				}
			}

			if (viaConstraints.back().side[0].empty() or viaConstraints.back().side[1].empty()) {
				viaConstraints.pop_back();
			}
		}
	}*/
}

void Router::buildRoutes() {
	routes.clear();

	// Create initial routes
	routes.reserve(base->nets.size()+2);
	for (int i = 0; i < (int)base->nets.size(); i++) {
		routes.push_back(Wire(i));
	}
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			if (base->stack[type].pins[i].outNet < (int)base->nets.size()) {
				routes[base->stack[type].pins[i].outNet].addPin(base, Index(type, i));
			} else {
				printf("outNet is out of bounds\n");
			}
		}
	}
	for (int i = (int)routes.size()-1; i >= 0; i--) {
		if (routes[i].pins.size() < 2 and not base->nets[routes[i].net].isIO) {
			delRoute(i);
		}
	}

	for (int type = 0; type < 2; type++) {
		base->stack[type].route = (int)routes.size();
		routes.push_back(Wire(flip(type)));
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			routes.back().addPin(base, Index(type, i));
		}
	}
}

void Router::buildPinBounds() {
	for (int type = 0; type < (int)base->stack.size(); type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			base->stack[type].pins[i].lo = numeric_limits<int>::max();
			base->stack[type].pins[i].hi = numeric_limits<int>::min();
		}
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			Pin &pin = base->pin(routes[i].pins[j].idx);
			if (routes[i].pOffset < pin.lo) {
				pin.lo = routes[i].pOffset;
			}
			if (routes[i].pOffset > pin.hi) {
				pin.hi = routes[i].pOffset;
			}
		}
	}
}

void Router::findCycles(vector<vector<int> > &cycles) {
	// DESIGN(edward.bingham) There can be multiple cycles with the same set of
	// nodes as a result of multiple pin constraints. This function does not
	// differentiate between those cycles. Doing so could introduce an
	// exponential blow up, and we can ensure that we split those cycles by
	// splitting on the node in the cycle that has the most pin constraints
	// (maximising min(in.size(), out.size()))

	if (routes.size() == 0) {
		return;
	}
	
	vector<vector<int> > A(routes.size(), vector<int>());
	
	// build the adjacency list
	// traverse the pin constraints
	vector<int> pins;
	for (int i = 0; i < (int)pinConstraints.size(); i++) {
		vector<int> from, to;
		for (int r = 0; r < (int)routes.size(); r++) {
			if (routes[r].net >= 0) {
				if (routes[r].hasPin(base, Index(Model::PMOS, pinConstraints[i].from))) {
					from.push_back(r);
				} else if (routes[r].hasPin(base, Index(Model::NMOS, pinConstraints[i].to))) {
					to.push_back(r);
				}
			}
		}

		for (auto j = from.begin(); j != from.end(); j++) {
			int size = A[*j].size();
			A[*j].insert(A[*j].end(), to.begin(), to.end());
			inplace_merge(A[*j].begin(), A[*j].begin()+size, A[*j].end());
		}
	}

	// traverse the route constraints
	for (int i = 0; i < (int)routeConstraints.size(); i++) {
		int select = routeConstraints[i].select;
		int from = routeConstraints[i].wires[select];
		int to = routeConstraints[i].wires[1-select];
		if (select >= 0) {
			auto iter = lower_bound(A[from].begin(), A[from].end(), to);
			A[from].insert(iter, to);
		}
	}

	// remove duplicates from adjacency lists
	for (int i = 0; i < (int)A.size(); i++) {
		A[i].erase(unique(A[i].begin(), A[i].end()), A[i].end());
	}

	//vector<int> B;
	//vector<bool> blocked(routes.size(), false);

	unordered_set<int> seen;
	unordered_set<int> staged;
	
	list<vector<int> > tokens;
	tokens.push_back(vector<int>(1, 0));
	staged.insert(0);
	while (not tokens.empty()) {
		while (not tokens.empty()) {
			vector<int> curr = tokens.front();
			tokens.pop_front();
			int i = curr.back();
			for (int j = 0; j < (int)A[i].size(); j++) {
				auto loop = find(curr.begin(), curr.end(), A[i][j]);
				if (loop != curr.end()) {
					cycles.push_back(curr);
					cycles.back().erase(cycles.back().begin(), cycles.back().begin()+(loop-curr.begin()));
					vector<int>::iterator minElem = min_element(cycles.back().begin(), cycles.back().end());
					rotate(cycles.back().begin(), minElem, cycles.back().end());
					if (find(cycles.begin(), (cycles.end()-1), cycles.back()) != (cycles.end()-1)) {
						cycles.pop_back();
					}
				} else if (seen.find(A[i][j]) == seen.end()) {
					tokens.push_back(curr);
					tokens.back().push_back(A[i][j]);
					staged.insert(A[i][j]);
				}
			}
		}

		seen.insert(staged.begin(), staged.end());
		staged.clear();
		for (int i = 0; i < (int)routes.size(); i++) {
			if (seen.find(i) == seen.end()) {
				tokens.push_back(vector<int>(1, i));
				staged.insert(i);
				break;
			}
		}
	}
}

void Router::breakRoute(int route, set<int> cycleRoutes) {
	// DESIGN(edward.bingham) There are two obvious options to mitigate cycles
	// in the constraint graph by splitting a route. Either way, one pin needs
	// to be shared across the two routes to handle the vertical connection
	// between them.
	// 1. Split the net vertically, putting all nmos pins in one route and all
	//    pmos in the other. We can really pick any pin to be the split point for
	//    this strategy.
	// 2. Split the net horizontally, picking a pin to be the split point and
	//    putting all the left pins in one and right pins in the other. The
	//    split point will be the shared pin.
	//
	// Either way, we should share the pin with the fewest pin constraints
	// as the vertical route. We may also want to check the number of
	// route constraints and distance to other pins in the new net.

	int left = min(base->stack[0].pins[0].pos, base->stack[1].pins[0].pos);
	int right = max(base->stack[0].pins.back().pos, base->stack[1].pins.back().pos);
	int center = (left + right)/2;

	Wire wp(routes[route].net);
	Wire wn(routes[route].net);
	vector<int> count(routes[route].pins.size(), 0);
	bool wpHasGate = false;
	bool wnHasGate = false;

	// Move all of the pins that participate in pin constraints associated
	// with the cycle.	
	for (int i = 0; i < (int)pinConstraints.size(); i++) {
		vector<Contact>::iterator from = routes[route].pins.end();
		vector<Contact>::iterator to = routes[route].pins.end();
		// A pin cannot have both a pin constraint in and a pin
		// constraint out because a pin is either PMOS or NMOS and pin
		// constraints always go out PMOS pins and in NMOS pins.

		// TODO(edward.bingham) This assumption may be invalidated by adding stack
		// constraints or via constraints because it is specific to pin
		// constraints.

		// First, check if this pin constraint is connected to any of the pins
		// in this route.
		bool hasFrom = routes[route].hasPin(base, Index(Model::PMOS, pinConstraints[i].from), &from);
		// error checking version
		//bool hasTo = routes[route].hasPin(base, Index(Model::NMOS, pinConstraints[i].to), &to);
		//if (not hasFrom and not hasTo) {
		//	continue;
		//} else if (hasFrom and hasTo) {
		//	printf("unitary cycle\n");
		//}

		// optimized version
		bool hasTo = false;
		if (not hasFrom) {
			hasTo = routes[route].hasPin(base, Index(Model::NMOS, pinConstraints[i].to), &to);
			if (not hasTo) {
				continue;
			}
		}

		int fromIdx = from-routes[route].pins.begin();
		int toIdx = to-routes[route].pins.begin();

		if (hasFrom) {
			count[fromIdx]++;
		}
		if (hasTo) {
			count[toIdx]++;
		}

		// Second, check if this pin constraint is connected to any of the
		// pins in any of the other routes that were found in the cycle.
		bool found = false;
		for (auto other = cycleRoutes.begin(); not found and other != cycleRoutes.end(); other++) {
			found = ((hasFrom and routes[*other].hasPin(base, Index(Model::NMOS, pinConstraints[i].to))) or
							 (hasTo and routes[*other].hasPin(base, Index(Model::PMOS, pinConstraints[i].from))));
		}
		if (not found) {
			continue;
		}

		// Move the pin to wp or wn depending on hasFrom and hasTo
		if (hasFrom) {
			wp.addPin(base, from->idx);
			wpHasGate = wpHasGate or base->pin(from->idx).device >= 0;
			count.erase(count.begin()+fromIdx);
			routes[route].pins.erase(from);
		} else if (hasTo) {
			wn.addPin(base, to->idx);
			wnHasGate = wnHasGate or base->pin(to->idx).device >= 0;
			count.erase(count.begin()+toIdx);
			routes[route].pins.erase(to);
		}
	}

	//printf("Step 1: w={");
	//for (int i = 0; i < (int)routes[route].pins.size(); i++) {
	//	printf("(%d,%d) ", routes[route].pins[i].type, routes[route].pins[i].pin);
	//}
	//printf("} wp={");
	//for (int i = 0; i < (int)wp.pins.size(); i++) {
	//	printf("(%d,%d) ", wp.pins[i].type, wp.pins[i].pin);
	//}
	//printf("} wn={");
	//for (int i = 0; i < (int)wn.pins.size(); i++) {
	//	printf("(%d,%d) ", wn.pins[i].type, wn.pins[i].pin);
	//}
	//printf("}\n");

	// DESIGN(edward.bingham) Pick one of the remaining pins to be a shared pin.
	// Pick the remaining pin that has the fewest pin constraints, is not a
	// gate, and is furthest toward the outer edge of the cell. Break ties
	// arbitrarily. This will move the vertical route out of the way as much as
	// possible from all of the other constraint problems. If there are no
	// remaining pins, then we need to record wp and wn for routing with A*
	int sharedPin = -1;
	int sharedCount = -1;
	bool sharedIsGate = true;
	int sharedDistanceFromCenter = 0;
	for (int i = 0; i < (int)routes[route].pins.size(); i++) {
		const Pin &pin = base->pin(routes[route].pins[i].idx);
		int distanceFromCenter = abs(pin.pos-center);

		if ((sharedCount < 0 or count[i] < sharedCount) or
		    (count[i] == sharedCount and ((sharedIsGate and pin.device < 0) or
		    distanceFromCenter > sharedDistanceFromCenter))) {
		//if (sharedCount < 0 or (count[i] < sharedCount or
		//    (count[i] == sharedCount and ((sharedIsGate and pin.device < 0) or
		//    (sharedIsGate == (pin.device >= 0) and distanceFromCenter > sharedDistanceFromCenter))))) {
			sharedPin = i;
			sharedCount = count[i];
			sharedIsGate = pin.device >= 0;
			sharedDistanceFromCenter = distanceFromCenter;
		}
	}

	if (sharedPin >= 0) {
		// TODO(edward.bingham) bug in which a non-cycle is being split resulting in redundant vias on various vertical routes
		wp.addPin(base, routes[route].pins[sharedPin].idx);
		wn.addPin(base, routes[route].pins[sharedPin].idx);
		routes[route].pins.erase(routes[route].pins.begin()+sharedPin);
		count.erase(count.begin()+sharedPin);
		wpHasGate = wpHasGate or sharedIsGate;
		wnHasGate = wnHasGate or sharedIsGate;
	} else {
		// Add a virtual pin to facilitate dogleg routing where no current pin is useable because of some constraint conflict
		// TODO(edward.bingham) Two things, I need to determine the horizontal
		// position of the pin by looking for available vertical tracks. The
		// problem is that I don't know the ordering of the routes at this point in
		// time, so the only really safe vertical track at the moment is all the
		// way at the end of the cell. I also need to create functionality for
		// saving the vertical position of the pin so the drawing functionality
		// knows where to draw the vertical path.
		Index virtPin(2, (int)base->stack[2].pins.size());
		base->stack[2].pins.push_back(Pin(routes[route].net));
		base->stack[2].pins.back().pos = -50;
		wp.addPin(base, virtPin);
		wn.addPin(base, virtPin);
	}

	//printf("Step 2: w={");
	//for (int i = 0; i < (int)routes[route].pins.size(); i++) {
	//	printf("(%d,%d) ", routes[route].pins[i].type, routes[route].pins[i].pin);
	//}
	//printf("} wp={");
	//for (int i = 0; i < (int)wp.pins.size(); i++) {
	//	printf("(%d,%d) ", wp.pins[i].type, wp.pins[i].pin);
	//}
	//printf("} wn={");
	//for (int i = 0; i < (int)wn.pins.size(); i++) {
	//	printf("(%d,%d) ", wn.pins[i].type, wn.pins[i].pin);
	//}
	//printf("}\n");

	// DESIGN(edward.bingham) If it is possible to avoid putting a gate pin in
	// one of wp or wn and put all of the PMOS in wp and all of the NMOS in wn,
	// try to do so. This will allow us to route whichever route it is over the
	// transistor stack.
	
	// DESIGN(edward.bingham) prefer routing overtop the pmos stack since it is
	// often wider as a result of the PN ratio.
	if (not wpHasGate) {
		// Put all non-gate PMOS pins into wp and all remaining pins into wn
		for (int i = (int)routes[route].pins.size()-1; i >= 0; i--) {
			Index pin = routes[route].pins[i].idx;
			bool isGate = base->stack[pin.type].pins[pin.pin].device < 0;
			if (pin.type == Model::PMOS and not isGate) {
				wp.addPin(base, pin);
			} else {
				wn.addPin(base, pin);
				wnHasGate = wnHasGate or isGate;
			}
			routes[route].pins.pop_back();
			count.pop_back();
		}
	}	else if (not wnHasGate) {
		// Put all non-gate NMOS pins into wn and all remaining pins into wp
		for (int i = (int)routes[route].pins.size()-1; i >= 0; i--) {
			Index pin = routes[route].pins[i].idx;
			bool isGate = base->stack[pin.type].pins[pin.pin].device < 0;
			if (pin.type == Model::NMOS and not isGate) {
				wn.addPin(base, pin);
			} else {
				wp.addPin(base, pin);
				wpHasGate = wpHasGate or isGate;
			}
			routes[route].pins.pop_back();
			count.pop_back();
		}
	} else {
		for (int i = (int)routes[route].pins.size()-1; i >= 0; i--) {
			Index pin = routes[route].pins[i].idx;
			// Determine horizontal location of wp and wn relative to sharedPin, then
			// place all pins on same side of sharedPin as wp or wn into that wp or wn
			// respectively.
			bool isGate = base->stack[pin.type].pins[pin.pin].device < 0;
			int pinPos = base->stack[pin.type].pins[pin.pin].pos;
			if (pinPos >= wn.left and pinPos >= wp.left and pinPos <= wn.right and pinPos <= wp.right) {
				if (pin.type == Model::PMOS) {
					wp.addPin(base, pin);
					wpHasGate = wpHasGate or isGate;
				} else {
					wn.addPin(base, pin);
					wnHasGate = wnHasGate or isGate;
				}
			} else if (pinPos >= wn.left and pinPos <= wn.right) {
				wn.addPin(base, pin);
				wnHasGate = wnHasGate or isGate;
			} else if (pinPos >= wp.left and pinPos <= wp.right) {
				wp.addPin(base, pin);
				wpHasGate = wpHasGate or isGate;
			} else if (min(abs(pinPos-wn.right),abs(pinPos-wn.left)) < min(abs(pinPos-wp.right),abs(pinPos-wp.left))) {
				wn.addPin(base, pin);
				wnHasGate = wnHasGate or isGate;
			} else {
				wp.addPin(base, pin);
				wpHasGate = wpHasGate or isGate;
			}
			routes[route].pins.pop_back();
			count.pop_back();
		}
	}

	//printf("Step 3: w={");
	//for (int i = 0; i < (int)routes[route].pins.size(); i++) {
	//	printf("(%d,%d) ", routes[route].pins[i].type, routes[route].pins[i].pin);
	//}
	//printf("} wp={");
	//for (int i = 0; i < (int)wp.pins.size(); i++) {
	//	printf("(%d,%d) ", wp.pins[i].type, wp.pins[i].pin);
	//}
	//printf("} wn={");
	//for (int i = 0; i < (int)wn.pins.size(); i++) {
	//	printf("(%d,%d) ", wn.pins[i].type, wn.pins[i].pin);
	//}
	//printf("}\n");

	routes[route].net = wp.net;
	routes[route].pins = wp.pins;
	routes[route].left = wp.left;
	routes[route].right = wp.right;
	routes[route].pOffset = wp.pOffset;
	routes[route].nOffset = wp.nOffset;
	routes.push_back(wn);
}

void Router::breakCycles(vector<vector<int> > cycles) {
	//int startingRoutes = (int)routes.size();

	// count up cycle participation for heuristic
	vector<vector<int> > cycleCount(routes.size(), vector<int>());
	for (int i = 0; i < (int)cycles.size(); i++) {
		for (int j = 0; j < (int)cycles[i].size(); j++) {
			cycleCount[cycles[i][j]].push_back(i);
		}
	}

	// compute pin constraint density for heuristic
	vector<int> numIn(routes.size(), 0);
	vector<int> numOut(routes.size(), 0);
	for (int i = 0; i < (int)pinConstraints.size(); i++) {
		for (int j = 0; j < (int)routes.size(); j++) {
			if (routes[j].hasPin(base, Index(Model::PMOS, pinConstraints[i].from))) {
				numOut[j]++;
			}
			if (routes[j].hasPin(base, Index(Model::NMOS, pinConstraints[i].to))) {
				numIn[j]++;
			}
		}
	}

	//printf("Starting Cycles\n");
	//for (int i = 0; i < (int)cycles.size(); i++) {
	//	printf("cycle {");
	//	for (int j = 0; j < (int)cycles[i].size(); j++) {
	//		if (j != 0) {
	//			printf(" ");
	//		}
	//		printf("%d", cycles[i][j]);
	//	}
	//	printf("}\n");
	//}

	//printf("NMOS\n");
	//for (int i = 0; i < (int)base->stack[0].pins.size(); i++) {
	//	printf("pin %d %d->%d->%d: %dx%d %d %d\n", base->stack[0].pins[i].device, base->stack[0].pins[i].leftNet, base->stack[0].pins[i].outNet, base->stack[0].pins[i].rightNet, base->stack[0].pins[i].width, base->stack[0].pins[i].height, base->stack[0].pins[i].off, base->stack[0].pins[i].pos);
	//}

	//printf("\nPMOS\n");
	//for (int i = 0; i < (int)base->stack[1].pins.size(); i++) {
	//	printf("pin %d %d->%d->%d: %dx%d %d %d\n", base->stack[1].pins[i].device, base->stack[1].pins[i].leftNet, base->stack[1].pins[i].outNet, base->stack[1].pins[i].rightNet, base->stack[1].pins[i].width, base->stack[1].pins[i].height, base->stack[1].pins[i].off, base->stack[1].pins[i].pos);
	//}

	//printf("\nRoutes\n");
	//for (int i = 0; i < (int)routes.size(); i++) {
	//	printf("wire %d %d->%d: ", routes[i].net, routes[i].left, routes[i].right);
	//	for (int j = 0; j < (int)routes[i].pins.size(); j++) {
	//		printf("(%d,%d) ", routes[i].pins[j].type, routes[i].pins[j].pin);
	//	}
	//	printf("\n");
	//}

	//printf("\nConstraints\n");
	//for (int i = 0; i < (int)pinConstraints.size(); i++) {
	//	printf("vert %d -> %d: %d\n", pinConstraints[i].from, pinConstraints[i].to, pinConstraints[i].off);
	//}
	//for (int i = 0; i < (int)routeConstraints.size(); i++) {
	//	printf("horiz %d -- %d: %d\n", routeConstraints[i].wires[0], routeConstraints[i].wires[1], routeConstraints[i].off);
	//}

	while (cycles.size() > 0) {
		// DESIGN(edward.bingham) We have multiple cycles and a route may
		// participate in more than one. It's unclear whether we want to minimize
		// the number of doglegs or not. Introducing a dogleg requires adding
		// another via, which may make some layouts more difficult, but it also
		// frees up constraints which may make other layouts easier. What we do
		// know is that cycles consist of two types of pins: PMOS and NMOS, and all
		// the PMOS pins are closer to eachother than they are to the NMOS pins and
		// visa versa. We also know that there can be multiple constraint arcs from
		// one node to another or back, and we ultimately need to break all of the
		// cycles represented by those constraint arcs.
		//
		//     a <--- d        a b a b a c b d  .
		//   ^^ |||   ^        | | | | | | | |  .
		//   || vvv   |        v v v v v v v v  .
		//     b ---> c        b a b a b d c a  .
		//
		//
		//  a1 -> a0 <--- d    a b a b a c b d   nodes   .
		//  \\\   ^^      ^    | | | | | | | |           .
		//   vvv //       |  o-o-|-o-|-o | | |     a1    .
		//      b ------> c  |   |   |   | | |    vvv\   .
		//                   | o-o-o-o-o-|-o |     b  \  .
		//                   | |   |   | |   |     v\\|  .
		//                   | |   |   | o-o |     c|||  .
		//                   | |   |   |   | |     v|||  .
		//                   | |   |   | o-|-o     d|||  .
		//                   | |   |   | | |       vvvv  .
		//                   o---o-|-o-|-|-|-o     a0    .
		//                     | | | | | | | |           .
		//                     b a b a b d c a           .
		//
		// In this example, there were no pins in a that we could share between a0
		// and a1 to connect the two nets. Sharing any of the pins would have
		// re-introduced the cycle. In this case, we need to use A* to route the
		// connection from a1 to a0, expanding the width of the cell if necessary.
		//
		// Therefore, we prefer to share pins if we can, but we can only share pins
		// if they themselves don't participate in the cycle.
		int maxCycleCount = -1;
		int maxDensity = -1;
		int route = -1;
		for (int i = 0; i < (int)cycleCount.size(); i++) {
			int density = min(numIn[i], numOut[i]);
			if (routes[i].net >= 0 and (((int)cycleCount[i].size() > maxCycleCount or
					((int)cycleCount[i].size() == maxCycleCount and density > maxDensity)))) {
				route = i;
				maxCycleCount = cycleCount[i].size();
				maxDensity = density;
			}
		}

		set<int> cycleRoutes;
		for (int i = 0; i < (int)cycleCount[route].size(); i++) {
			cycleRoutes.insert(cycles[cycleCount[route][i]].begin(), cycles[cycleCount[route][i]].end());
		}
		cycleRoutes.erase(route);

		//printf("Breaking Route %d: ", route);
		//for (auto i = cycleRoutes.begin(); i != cycleRoutes.end(); i++) {
		//	printf("%d ", *i);
		//}
		//printf("\n");
		breakRoute(route, cycleRoutes);
		//printf("wire %d %d->%d: ", routes[route].net, routes[route].left, routes[route].right);
		//for (int j = 0; j < (int)routes[route].pins.size(); j++) {
		//	printf("(%d,%d) ", routes[route].pins[j].type, routes[route].pins[j].pin);
		//}
		//printf("\n");
		//printf("wire %d %d->%d: ", routes.back().net, routes.back().left, routes.back().right);
		//for (int j = 0; j < (int)routes.back().pins.size(); j++) {
		//	printf("(%d,%d) ", routes.back().pins[j].type, routes.back().pins[j].pin);
		//}
		//printf("\n");

		// recompute cycles and cycleCount
		while (cycleCount[route].size() > 0) {		
			int cycle = cycleCount[route].back();
			cycles.erase(cycles.begin()+cycle);
			for (int i = 0; i < (int)cycleCount.size(); i++) {
				for (int j = (int)cycleCount[i].size()-1; j >= 0; j--) {
					if (cycleCount[i][j] > cycle) {
						cycleCount[i][j]--;
					} else if (cycleCount[i][j] == cycle) {
						cycleCount[i].erase(cycleCount[i].begin()+j);
					}
				}
			}
		}
	}

	/*cycles = findCycles();
	if (cycles.size() > 0) {
		printf("error: cycles not broken %d -> %d\n", startingRoutes, (int)routes.size());
		for (int i = 0; i < (int)cycles.size(); i++) {
			printf("cycle {");
			for (int j = 0; j < (int)cycles[i].size(); j++) {
				if (j != 0) {
					printf(" ");
				}
				printf("%d", cycles[i][j]);
			}
			printf("}\n");
		}

		printf("\n\n");
	}*/
}

void Router::findAndBreakViaCycles() {
	/*for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			base->stack[type].pins[i].viaToPin.clear();
			base->stack[type].pins[i].pinToVia.clear();
		}
	}
	
	// <index into Circuit::stack[via->type], index into Router::viaConstraints>
	vector<vector<ViaConstraint>::iterator> active;
	for (auto via = viaConstraints.begin(); via != viaConstraints.end(); via++) {
		for (auto s0p = via->side[0].begin(); s0p != via->side[0].end(); s0p++) {
			for (auto s1p = via->side[1].begin(); s1p != via->side[1].end(); s1p++) {
				// Because routes have been broken up at this point in order to
				// fix pin constraint cycles, a pin could participate in
				// multiple routes. We need to check all via relations.
				array<vector<int>, 2> hasSide;
				vector<int> hasMid;
				for (int i = 0; i < (int)routes.size(); i++) {
					if (routes[i].hasPin(base, s0p->idx)) {
						hasSide[0].push_back(i);
					}
					if (routes[i].hasPin(base, s1p->idx)) {
						hasSide[1].push_back(i);
					}
					if (routes[i].hasPin(base, via->idx)) {
						hasMid.push_back(i);
					}
				}

				// Identify potentially violated via constraints.
				//
				// DESIGN(edward.bingham) This could probably be done more
				// intelligently by setting up a graph structure of vias and
				// navigating that to look for violations, but I suspect that
				// the number of vias we need to check across the three vectors
				// will be quite low...  likely just a single via in each list
				// most of the time. Occationally two vias in one of the lists.
				// So just brute forcing the problem shouldn't cause too much of
				// an issue.
				bool found = false;
				for (auto s0 = hasSide[0].begin(); not found and s0 != hasSide[0].end(); s0++) {
					for (auto s1 = hasSide[1].begin(); not found and s1 != hasSide[1].end(); s1++) {
						for (auto m = hasMid.begin(); not found and m != hasMid.end(); m++) {
							found = found or (
								((s0p->idx.type == Model::PMOS and routes[*s0].hasPrev(*m)) or
								 (s0p->idx.type == Model::NMOS and routes[*m].hasPrev(*s0))) and
								((s1p->idx.type == Model::PMOS and routes[*s1].hasPrev(*m)) or
								 (s1p->idx.type == Model::NMOS and routes[*m].hasPrev(*s1)))
							);
						}
					}
				}

				if (found) {
					base->pin(via->idx).addOffset(Pin::PINTOVIA, s0p->idx, s0p->off);
					base->pin(s1p->idx).addOffset(Pin::VIATOPIN, via->idx, s1p->off);
				}
			}
		}
	}*/
}

struct Alignment {
	Alignment() {}
	Alignment(const Circuit *base, int pmos, int nmos) {
		this->base = base;
		this->pin[Model::PMOS] = pmos;
		this->pin[Model::NMOS] = nmos;
	}
	~Alignment() {}

	const Circuit *base;
	array<int, 2> pin;

	int dist() const {
		array<bool, 2> startOfStack = {
			(pin[0] == 0 or base->stack[0].pins[1-pin[0]].device < 0) and base->stack[0].pins[pin[0]].device < 0,
			(pin[1] == 0 or base->stack[1].pins[1-pin[1]].device < 0) and base->stack[1].pins[pin[1]].device < 0
		};

		int result = base->stack[1].pins[pin[1]].pos - base->stack[0].pins[pin[0]].pos;
		if ((result < 0 and startOfStack[1]) or (result > 0 and startOfStack[0])) {
			// We can move the start of the stack around as much as we want
			result = 1;
		}
		result = (result < 0 ? -result : result) + 1;

		int coeff = 1;
		// If the two pins aren't on the same layer, we don't care as much
		if (base->stack[1].pins[pin[1]].layer != base->stack[1].pins[pin[1]].layer) {
			coeff = 3;
		// Prefer matching up gates rather than source or drain
		} else if (base->stack[1].pins[pin[1]].layer > 0) {
			coeff = 2;
		}

		return result * coeff;
	}

	bool conflictsWith(const Alignment &a0) {
		return pin[1] == a0.pin[1] or pin[0] == a0.pin[0] or
			    (pin[1] < a0.pin[1] and pin[0] > a0.pin[0]) or
			    (pin[1] > a0.pin[1] and pin[0] < a0.pin[0]);
	}
};

bool operator>(const Alignment &a0, const Alignment &a1) {
	return a0.dist() > a1.dist();
}

void Router::buildPins(const Tech &tech) {
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			Pin &pin = base->stack[type].pins[i];
			pin.width = base->pinWidth(tech, Index(type, i));
			pin.height = base->pinHeight(Index(type, i));

			pin.layout.clear();
			drawPin(tech, pin.layout, base, base->stack[type], i);
		}
	}
}

void Router::buildContacts(const Tech &tech) {
	for (int i = 0; i < (int)routes.size(); i++) {
		if (routes[i].net < 0) {
			continue;
		}

		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			Pin &pin = base->pin(routes[i].pins[j].idx);
			int prevLevel = routes[i].getLevel(j-1);
			int nextLevel = routes[i].getLevel(j);
			int maxLevel = max(pin.layer, max(nextLevel, prevLevel));
			int minLevel = min(pin.layer, min(nextLevel, prevLevel));

			routes[i].pins[j].layout.clear();
			drawViaStack(tech, routes[i].pins[j].layout, routes[i].net, minLevel, maxLevel, vec2i(0, 0), vec2i(0,0), vec2i(0,0));
		}
	}
}

void Router::buildHorizConstraints(const Tech &tech) {
	for (int type = 0; type < (int)base->stack[type].pins.size(); type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			base->stack[type].pins[i].toPin.clear();
		}
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			routes[i].pins[j].fromPin.clear();
			routes[i].pins[j].toPin.clear();
		}
	}

	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			Pin &pin = base->stack[type].pins[i];

			int off = 0;
			if (i+1 < (int)base->stack[type].pins.size()) {
				Pin &next = base->stack[type].pins[i+1];
				int substrateMode = pin.device >= 0 or next.device >= 0 ? Layout::MERGENET : Layout::DEFAULT;
				if (minOffset(&off, tech, 0, pin.layout.layers, 0, next.layout.layers, 0, substrateMode, Layout::DEFAULT)) {
					pin.offsetToPin(Index(type, i+1), off);
				} else {
					printf("error: no offset found at pin (%d,%d)\n", type, i+1);
				}
			}

			for (int j = 0; j < (int)routes.size(); j++) {
				if (routes[j].net < 0 or routes[j].hasPin(base, Index(type, i))) {
					continue;
				}

				for (int k = 0; k < (int)routes[j].pins.size(); k++) {
					int off = 0;
					if ((routes[j].pins[k].idx.type != type or i < routes[j].pins[k].idx.pin) and
					    minOffset(&off, tech, 0, pin.layout.layers, 0, routes[j].pins[k].layout.layers, 0, Layout::IGNORE, Layout::MERGENET)) {
						routes[j].pins[k].offsetFromPin(Index(type, i), off);
					}

					off = 0;
					if ((routes[j].pins[k].idx.type != type or routes[j].pins[k].idx.pin < i) and
					    minOffset(&off, tech, 0, routes[j].pins[k].layout.layers, 0, pin.layout.layers, 0, Layout::IGNORE, Layout::MERGENET)) {
						routes[j].pins[k].offsetToPin(Index(type, i), off);
					}
				}
			}
		}
	}
}

void Router::updatePinPos() {
	for (int type = 0; type < (int)base->stack.size(); type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			base->stack[type].pins[i].pos = 0;
		}
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			routes[i].pins[j].left = numeric_limits<int>::min();
			routes[i].pins[j].right = numeric_limits<int>::max();
		}
	}

	vector<Index> stack;
	stack.reserve(20);
	if (base->stack[0].pins.size() > 0 and base->stack[1].pins.size() > 0) {
		if (base->stack[0].pins[0].align < 0) {
			stack.push_back(Index(0, 0));
			stack.push_back(Index(1, 0));
		} else {
			stack.push_back(Index(1, 0));
			stack.push_back(Index(0, 0));
		}
	} else if (base->stack[0].pins.size() > 0) {
		stack.push_back(Index(0, 0));
	} else if (base->stack[1].pins.size() > 0) {
		stack.push_back(Index(1, 0));
	}

	while (not stack.empty()) {
		// handle pin to pin constraints and alignment constraints
		Index curr = stack.back();
		stack.pop_back();
		Pin &pin = base->pin(curr);

		for (auto off = pin.toPin.begin(); off != pin.toPin.end(); off++) {
			Pin &next = base->pin(off->first);
			int pos = pin.pos+off->second;
			if (next.pos < pos) {
				printf("pinToPin (%d,%d)->%d->(%d,%d) %d->%d\n", curr.type, curr.pin, off->second, off->first.type, off->first.pin, next.pos, pos);
				next.pos = pos;
				stack.push_back(off->first);
			}
		}
		if (not stack.empty()) {
			sort(stack.rbegin(), stack.rend());
			stack.erase(unique(stack.begin(), stack.end()), stack.end());
			continue;
		}

		// handle pin alignments
		for (int type = 0; type < 2; type++) {
			for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
				Pin &pin = base->stack[type].pins[i];
				if (pin.align >= 0) {
					Pin &align = base->stack[1-type].pins[pin.align];
					if (pin.pos < align.pos) {
						printf("pinAlign (%d,%d)--(%d,%d) %d->%d\n", type, i, 1-type, pin.align, pin.pos, align.pos);
						pin.pos = align.pos;
						stack.push_back(Index(type, i));
					} else if (align.pos < pin.pos) {
						printf("pinAlign (%d,%d)--(%d,%d) %d->%d\n", 1-type, pin.align, type, i, align.pos, pin.pos);
						align.pos = pin.pos;
						stack.push_back(Index(1-type, pin.align));
					}
				}
			}
		}
		if (not stack.empty()) {
			sort(stack.rbegin(), stack.rend());
			stack.erase(unique(stack.begin(), stack.end()), stack.end());
			continue;
		}

		// handle pin to via and via to pin constraints
		for (int i = 0; i < (int)routes.size(); i++) {
			if (routes[i].net < 0) {
				continue;
			}

			for (int j = 0; j < (int)routes[i].pins.size(); j++) {
				Pin &pin = base->pin(routes[i].pins[j].idx);

				for (auto off = routes[i].pins[j].fromPin.begin(); off != routes[i].pins[j].fromPin.end(); off++) {
					Pin &prev = base->pin(off->first);
					if (prev.pos < pin.pos and routes[i].pOffset >= prev.lo and routes[i].pOffset <= prev.hi) {
						int pos = prev.pos+off->second;
						if (routes[i].pins[j].left < pos) {
							printf("pinToVia (%d,%d)->%d->(%d,%d) %d->%d\n", off->first.type, off->first.pin, off->second, i, j, routes[i].pins[j].left, pos);
							routes[i].pins[j].left = pos;
						}
					}
				}

				for (auto off = routes[i].pins[j].toPin.begin(); off != routes[i].pins[j].toPin.end(); off++) {
					Pin &next = base->pin(off->first);
					if (pin.pos < next.pos and routes[i].pOffset >= next.lo and routes[i].pOffset <= next.hi) {
						int pos = routes[i].pins[j].left+off->second;
						if (next.pos < pos) {
							printf("viaToPin (%d,%d)->%d->(%d,%d) %d->%d\n", i, j, off->second, off->first.type, off->first.pin, next.pos, pos);
							next.pos = pos;
							stack.push_back(off->first);
						}
					}
				}
			}
		}
		sort(stack.rbegin(), stack.rend());
		stack.erase(unique(stack.begin(), stack.end()), stack.end());
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		if (routes[i].net < 0) {
			continue;
		}

		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			Pin &pin = base->pin(routes[i].pins[j].idx);
			for (auto off = routes[i].pins[j].toPin.begin(); off != routes[i].pins[j].toPin.end(); off++) {
				Pin &next = base->pin(off->first);
				if (pin.pos < next.pos and routes[i].pOffset >= next.lo and routes[i].pOffset <= next.hi) {
					int pos = next.pos-off->second;
					if (routes[i].pins[j].right > pos) {
						routes[i].pins[j].right = pos;
					}
				}
			}
		}
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		if (routes[i].net >= 0) {
			routes[i].resortPins(base);
		}
	}
}

int Router::alignPins(int maxDist) {
	for (int type = 0; type < (int)base->stack.size(); type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			base->stack[type].pins[i].align = -1;
		}
	}

	vector<Alignment> align;
	if (routes.empty()) {
		for (int i = 0; i < (int)base->nets.size(); i++) {
			if (base->nets[i].gates[0] == 1 and base->nets[i].gates[1] == 1) {
				array<int, 2> ports;
				for (int type = 0; type < 2; type++) {
					for (ports[type] = 0; ports[type] < (int)base->stack[type].pins.size() and (base->stack[type].pins[ports[type]].device < 0 or base->stack[type].pins[ports[type]].outNet != i); ports[type]++);
				}

				if (ports[0] < (int)base->stack[0].pins.size() and ports[1] < (int)base->stack[1].pins.size()) {
					align.push_back(Alignment(base, ports[Model::PMOS], ports[Model::NMOS]));
				}
			}
			if (base->nets[i].ports[0] == 1 and base->nets[i].ports[1] == 1) {
				array<int, 2> ports;
				for (int type = 0; type < 2; type++) {
					for (ports[type] = 0; ports[type] < (int)base->stack[type].pins.size() and (base->stack[type].pins[ports[type]].device >= 0 or base->stack[type].pins[ports[type]].outNet != i); ports[type]++);
				}

				if (ports[0] < (int)base->stack[0].pins.size() and ports[1] < (int)base->stack[1].pins.size()) {
					align.push_back(Alignment(base, ports[Model::PMOS], ports[Model::NMOS]));
				}
			}
		}
	} else {
		array<vector<int>, 2> ends;
		for (int i = 0; i < (int)routes.size(); i++) {
			if (routes[i].net < 0) {
				continue;
			}

			// TODO(edward.bingham) optimize this
			ends[0].clear();
			ends[1].clear();
			for (int j = 0; j < (int)routes[i].pins.size(); j++) {
				const Index &pin = routes[i].pins[j].idx;

				if (pin.type >= 2) {
					continue;
				}

				if ((int)ends[pin.type].size() < 2) {
					ends[pin.type].push_back(pin.pin);
				} else {
					ends[pin.type][1] = pin.pin;
				}
			}

			if ((int)ends[Model::PMOS].size() == 1 and (int)ends[Model::NMOS].size() == 1) {
				align.push_back(Alignment(base, ends[Model::PMOS][0], ends[Model::NMOS][0]));
			}
		}
	}

	int matches = 0;
	while (not align.empty()) {
		sort(align.begin(), align.end(), std::greater<>{});
		bool found = false;
		Alignment curr;
		for (int i = (int)align.size()-1; i >= 0; i--) {
			if (maxDist < 0 or align[i].dist() <= maxDist) {
				curr = align[i];
				align.erase(align.begin()+i);
				found = true;
				break;
			}
		}
		if (not found) {
			break;
		}

		for (int type = 0; type < 2; type++) {
			base->stack[type].pins[curr.pin[type]].align = curr.pin[1-type];
		}
		matches++;

		updatePinPos();
		for (int i = (int)align.size()-1; i >= 0; i--) {
			if (align[i].conflictsWith(curr)) {
				align.erase(align.begin()+i);
			}
		}
	}

	return matches;
}

void Router::drawRoutes(const Tech &tech) {
	for (int i = 0; i < (int)routes.size(); i++) {
		routes[i].layout.clear();
	}

	// Draw the routes
	for (int i = 0; i < (int)routes.size(); i++) {
		if (routes[i].net >= 0) {
			drawWire(tech, routes[i].layout, base, routes[i]);
		} else {
			base->stack[flip(routes[i].net)].draw(tech, base, routes[i].layout);
		}
	}
}

void Router::buildRouteConstraints(const Tech &tech, bool allowOverCell) {
	routeConstraints.clear();
	// TODO(edward.bingham) There's a bug here where poly routes are placed too
	// close to the diffusion. This is because the DRC rule involved is more than
	// just a spacing rule between two single layers. It requires a larger DRC
	// engine to be able to check that. Simply adding a spacing rule between poly
	// and diffusion causes havok with the transistor placement in the nmos and
	// pmos stacks.

	// Compute route constraints
	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = i+1; j < (int)routes.size(); j++) {
			// TODO(edward.bingham) I need to check pin overlap of routes. If any of
			// the pins of either route has a minOffset conflict with the other
			// route, then their order is determined by which stack the pin is
			// associated with. This is routing only on the pin side vs routing and
			// non routing on the route side. That comparison mode is not yet
			// supported by the DRC engine.

			int routingMode = (routes[i].net < 0 and routes[j].net >= 0) or (routes[i].net >= 0 and routes[j].net < 0) ? Layout::MERGENET : Layout::DEFAULT;
			int off[2] = {0,0};
			bool fromto = minOffset(off+0, tech, 1, routes[i].layout.layers, 0, routes[j].layout.layers, 0, Layout::DEFAULT, routingMode);
			bool tofrom = minOffset(off+1, tech, 1, routes[j].layout.layers, 0, routes[i].layout.layers, 0, Layout::DEFAULT, routingMode);
			if (not allowOverCell and (routes[i].net < 0 or routes[j].net < 0)) {
				routeConstraints.push_back(RouteConstraint(i, j, off[0], off[1]));
				routeConstraints.back().select = (flip(routes[j].net) == Model::PMOS or flip(routes[i].net) == Model::NMOS);
			} else if (fromto or tofrom) {
				routeConstraints.push_back(RouteConstraint(i, j, off[0], off[1]));

				array<vector<bool>, 2> hasType = {routes[i].pinTypes(), routes[j].pinTypes()};
				if ((routes[i].net < 0 and routes[j].net < 0) or
				    (routes[i].net < 0 and hasType[1][1-flip(routes[i].net)]) or
				    (routes[j].net < 0 and hasType[0][1-flip(routes[j].net)]))  {
					routeConstraints.back().select = (flip(routes[j].net) == Model::PMOS or flip(routes[i].net) == Model::NMOS);
				}
			}
		}
	}
}

void Router::buildGroupConstraints(const Tech &tech) {
	groupConstraints.clear();

	for (int i = 0; i < (int)routes.size(); i++) {
		for (int type = 0; type < (int)base->stack.size(); type++) {
			for (int j = 0; j < (int)base->stack[type].pins.size(); j++) {
				auto pos = lower_bound(routes[i].pins.begin(), routes[i].pins.end(), Index(type, j), CompareIndex(base));
				if (pos != routes[i].pins.end() and pos != routes[i].pins.begin() and pos->idx != Index(type, j)) {
					pos--;
					int level = routes[i].getLevel(pos-routes[i].pins.begin());
					if (level == base->stack[type].pins[j].layer) {
						groupConstraints.push_back(RouteGroupConstraint(i, Index(type, j)));
					}
				}
			}
		}
	}
}

set<int> Router::propagateRouteConstraint(int idx) {
	set<int> result;
	if (routeConstraints[idx].select < 0) {
		return result;
	}

	// DESIGN(edward.bingham) If a route constraint implies some ordering that
	// involves a route that participates in a group constraint, then it also
	// implies the same ordering with all other routes that participate in that
	// group constraint.
	//
	//  ----- ^ ^
	//        | |
	//  --O-- | v route constraint
	//    |  <-   group constraint
	//  --O--
	for (int i = 0; i < (int)groupConstraints.size(); i++) {
		int from[2] = {-1,-1};
		if (groupConstraints[i].wire == routeConstraints[idx].wires[0]) {
			from[0] = 0;
		} else if (groupConstraints[i].wire == routeConstraints[idx].wires[1]) {
			from[0] = 1;
		}

		if (from[0] >= 0 and routes[routeConstraints[idx].wires[1-from[0]]].hasPin(base, groupConstraints[i].pin)) {
			for (int j = 0; j < (int)routeConstraints.size(); j++) {
				if (j == idx or routeConstraints[j].select >= 0) {
					continue;
				}

				from[1] = -1;
				if (routeConstraints[j].wires[0] == groupConstraints[i].wire) {
					from[1] = 0;
				} else if (routeConstraints[j].wires[1] == groupConstraints[i].wire) {
					from[1] = 1;
				}

				if (from[1] >= 0 and routes[routeConstraints[j].wires[1-from[1]]].hasPin(base, groupConstraints[i].pin)) {
					routeConstraints[j].select = (from[0] == from[1] ? routeConstraints[idx].select : 1-routeConstraints[idx].select);
					result.insert(j);
				}
			}
		}
	}
	return result;
}

// make sure the graph is acyclic before running this
vector<int> Router::findTop() {
	for (int i = 0; i < (int)routeConstraints.size(); i++) {
		if (routeConstraints[i].select >= 0 and flip(routes[routeConstraints[i].wires[routeConstraints[i].select]].net) == Model::PMOS) {
			return vector<int>(1, routeConstraints[i].wires[routeConstraints[i].select]);
		}
	}

	// set up initial tokens for evaluating pin constraints
	vector<int> tokens;
	for (int i = 0; i < (int)routes.size(); i++) {
		bool found = false;
		for (int j = 0; not found and j < (int)pinConstraints.size(); j++) {
			found = found or routes[i].hasPin(base, Index(Model::NMOS, pinConstraints[j].to));
		}
		for (int j = 0; not found and j < (int)routeConstraints.size(); j++) {
			found = found or (routeConstraints[j].select >= 0 and routeConstraints[j].wires[1-routeConstraints[j].select] == i);
		}
		if (not found) {
			tokens.push_back(i);
		}
	}
	
	return tokens;
}

// make sure the graph is acyclic before running this
vector<int> Router::findBottom() {
	for (int i = 0; i < (int)routeConstraints.size(); i++) {
		if (routeConstraints[i].select >= 0 and flip(routes[routeConstraints[i].wires[routeConstraints[i].select]].net) == Model::NMOS) {
			return vector<int>(1, routeConstraints[i].wires[routeConstraints[i].select]);
		}
	}

	// set up initial tokens for evaluating pin constraints
	vector<int> tokens;
	for (int i = 0; i < (int)routes.size(); i++) {
		bool found = false;
		for (int j = 0; not found and j < (int)pinConstraints.size(); j++) {
			found = found or routes[i].hasPin(base, Index(Model::PMOS, pinConstraints[j].from));
		}
		for (int j = 0; not found and j < (int)routeConstraints.size(); j++) {
			found = found or (routeConstraints[j].select >= 0 and routeConstraints[j].wires[routeConstraints[j].select] == i);
		}
		if (not found) {
			tokens.push_back(i);
		}
	}
	
	return tokens;
}

void Router::zeroWeights() {
	cellHeight = 0;
	for (int i = 0; i < (int)routes.size(); i++) {
		routes[i].pOffset = 0;
		routes[i].nOffset = 0;
	}
}

void Router::clearPrev() {
	for (int i = 0; i < (int)routes.size(); i++) {
		routes[i].prevNodes.clear();
	}
}

void Router::buildPrevNodes(vector<int> start) {
	bool hasError = false;
	if (start.empty()) {
		for (int i = 0; i < (int)routes.size(); i++) {
			start.push_back(i);
		}
	}

	vector<vector<int> > tokens;
	for (int i = 0; i < (int)start.size(); i++) {
		tokens.push_back(vector<int>(1, start[i]));
	}
	while (not tokens.empty()) {
		vector<int> curr = tokens.back();
		tokens.pop_back();

		if (curr.back() >= 0) {
			for (auto pin = pinConstraints.begin(); pin != pinConstraints.end(); pin++) {
				if (routes[curr.back()].hasPin(base, Index(Model::PMOS, pin->from))) {
					for (int j = 0; j < (int)routes.size(); j++) {
						if (j != curr.back() and routes[j].hasPin(base, Index(Model::NMOS, pin->to))) {
							bool change = routes[j].prevNodes.insert(curr.back()).second;
							for (auto prev = routes[curr.back()].prevNodes.begin(); prev != routes[curr.back()].prevNodes.end(); prev++) {
								bool inserted = routes[j].prevNodes.insert(*prev).second;
								change = change or inserted;
							}

							if (change) {
								auto pos = find(curr.begin(), curr.end(), j);
								if (pos == curr.end()) {
									tokens.push_back(curr);
									tokens.back().push_back(j);
								} else {
									hasError = true;
									printf("error: buildPrevNodes found cycle {");
									for (int i = 0; i < (int)curr.size(); i++) {
										printf("%d ", curr[i]);
									}
									printf("%d}\n", j);
								}
							}
						}
					}
				}
			}
		}

		for (int i = 0; i < (int)routeConstraints.size(); i++) {
			if (routeConstraints[i].select >= 0 and curr.back() == routeConstraints[i].wires[routeConstraints[i].select]) {
				int out = routeConstraints[i].wires[1-routeConstraints[i].select];

				// keep track of anscestor nodes
				bool change = false;
				if (curr.back() >= 0) {
					change = routes[out].prevNodes.insert(curr.back()).second;
					for (auto prev = routes[curr.back()].prevNodes.begin(); prev != routes[curr.back()].prevNodes.end(); prev++) {
						bool inserted = routes[out].prevNodes.insert(*prev).second;
						change = change or inserted;
					}
				}

				if (change) {
					auto pos = find(curr.begin(), curr.end(), out);
					if (pos == curr.end()) {
						tokens.push_back(curr);
						tokens.back().push_back(out);
					} else {
						hasError = true;
						printf("error: buildPrevNodes found cycle {");
						for (int i = 0; i < (int)curr.size(); i++) {
							printf("%d ", curr[i]);
						}
						printf("%d}\n", out);
					}
				}
			}
		}
	}

	if (hasError) {
		print();
	}
}

void Router::buildPOffsets(const Tech &tech, vector<int> start) {
	bool hasError = false;
	if (start.empty()) {
		start.push_back(base->stack[Model::PMOS].route);
	}

	vector<vector<int> > tokens;
	sort(start.begin(), start.end());
	start.erase(unique(start.begin(), start.end()), start.end());
	for (int i = 0; i < (int)start.size(); i++) {
		tokens.push_back(vector<int>(1, start[i]));
	}
	while (not tokens.empty()) {
		vector<int> curr = tokens.back();
		tokens.pop_back();

		if (curr.back() >= 0) {
			for (int i = 0; i < (int)pinConstraints.size(); i++) {
				if (routes[curr.back()].hasPin(base, Index(Model::PMOS, pinConstraints[i].from))) {
					for (int j = 0; j < (int)routes.size(); j++) {
						if (j != curr.back() and routes[j].hasPin(base, Index(Model::NMOS, pinConstraints[i].to))) {
							bool change = routes[j].prevNodes.insert(curr.back()).second;
							for (auto prev = routes[curr.back()].prevNodes.begin(); prev != routes[curr.back()].prevNodes.end(); prev++) {
								bool inserted = routes[j].prevNodes.insert(*prev).second;
								change = change or inserted;
							}

							if (change) {
								auto pos = find(curr.begin(), curr.end(), j);
								if (pos == curr.end()) {
									tokens.push_back(curr);
									tokens.back().push_back(j);
								} else {
									printf("error: buildPOffset found cycle {");
									for (int i = 0; i < (int)curr.size(); i++) {
										printf("%d ", curr[i]);
									}
									printf("%d}\n", j);
									hasError = true;
								}
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < (int)routeConstraints.size(); i++) {
			if (routeConstraints[i].select >= 0 and curr.back() == routeConstraints[i].wires[routeConstraints[i].select]) {
				int weight = routes[curr.back()].pOffset + routeConstraints[i].off[routeConstraints[i].select];
				int out = routeConstraints[i].wires[1-routeConstraints[i].select];

				// keep track of anscestor nodes
				bool change = false;
				if (curr.back() >= 0) {
					change = routes[out].prevNodes.insert(curr.back()).second;
					for (auto prev = routes[curr.back()].prevNodes.begin(); prev != routes[curr.back()].prevNodes.end(); prev++) {
						bool inserted = routes[out].prevNodes.insert(*prev).second;
						change = change or inserted;
					}
				}

				// keep track of weight
				if (routes[out].pOffset < weight) {
					routes[out].pOffset = weight;
					change = true;
				}

				if (change) {
					auto pos = find(curr.begin(), curr.end(), out);
					if (pos == curr.end()) {
						tokens.push_back(curr);
						tokens.back().push_back(out);
					} else {
						printf("error: buildPOffset found cycle {");
						for (int i = 0; i < (int)curr.size(); i++) {
							printf("%d ", curr[i]);
						}
						printf("%d}\n", out);
						hasError = true;
					}
				}
			}
		}
	}

	if (hasError) {
		print();
	}
}

void Router::buildNOffsets(const Tech &tech, vector<int> start) {
	bool hasError = false;
	if (start.empty()) {
		start.push_back(base->stack[Model::NMOS].route);
	}

	vector<vector<int> > tokens;
	sort(start.begin(), start.end());
	start.erase(unique(start.begin(), start.end()), start.end());
	for (int i = 0; i < (int)start.size(); i++) {
		tokens.push_back(vector<int>(1, start[i]));
	}
	while (not tokens.empty()) {
		vector<int> curr = tokens.back();
		tokens.pop_back();

		for (int i = 0; i < (int)routeConstraints.size(); i++) {
			if (routeConstraints[i].select >= 0 and curr.back() == routeConstraints[i].wires[1-routeConstraints[i].select]) {
				int weight = routes[curr.back()].nOffset + routeConstraints[i].off[routeConstraints[i].select];
				int in = routeConstraints[i].wires[routeConstraints[i].select];
				if (in >= 0 and routes[in].nOffset < weight) {
					routes[in].nOffset = weight;

					auto pos = find(curr.begin(), curr.end(), in);
					if (pos == curr.end()) {
						tokens.push_back(curr);
						tokens.back().push_back(in);
					} else {
						hasError = true;
						printf("error: buildNOffset found cycle {");
						for (int i = 0; i < (int)curr.size(); i++) {
							printf("%d ", curr[i]);
						}
						printf("%d}\n", in);
					}
				}
			}
		}
	}

	if (hasError) {
		print();
	}
}

void Router::resetGraph(const Tech &tech) {
	zeroWeights();
	clearPrev();
	buildPrevNodes();
	buildPOffsets(tech);
	buildNOffsets(tech);
}

void Router::assignRouteConstraints(const Tech &tech) {
	vector<int> inTokens, outTokens;
	for (int i = 0; i < (int)routeConstraints.size(); i++) {
		if (routeConstraints[i].select >= 0) {
			set<int> prop = propagateRouteConstraint(i);
			for (auto j = prop.begin(); j != prop.end(); j++) {
				int from = routeConstraints[*j].select;
				inTokens.push_back(routeConstraints[*j].wires[from]);
				outTokens.push_back(routeConstraints[*j].wires[1-from]);
			}
		}
	}
	if (inTokens.size() + outTokens.size() > 0) {
		buildPOffsets(tech, inTokens);
		buildNOffsets(tech, outTokens);
	}

	vector<int> unassigned;
	unassigned.reserve(routeConstraints.size());
	for (int i = 0; i < (int)routeConstraints.size(); i++) {
		if (routeConstraints[i].select < 0) {
			unassigned.push_back(i);
		}
	}

	while (unassigned.size() > 0) {
		// handle critical constraints, that would create cycles if assigned the wrong direction.
		inTokens.clear();
		outTokens.clear();
		for (int u = (int)unassigned.size()-1; u >= 0; u--) {
			if (routeConstraints[unassigned[u]].select >= 0) {
				unassigned.erase(unassigned.begin()+u);
				continue;
			}
			// TODO(edward.bingham) check via constraints. If this is a critical
			// constraint that participates in a via constraint, and as a results
			// both directions create a cycle, then we need to expand the via
			// constraint. If this is not a critical constraint, but it participates
			// in a via constraint, and setting a particular direction would create a
			// cycle as a result of the via constraint, then either we need to set
			// the other direction or we need to expand the via constraint. The final
			// way we could deal with a via constraint cycle is to break up the route
			// to allow a horizontal path

			int i = unassigned[u];
			int a = routeConstraints[i].wires[0];
			int b = routeConstraints[i].wires[1];

			if (a >= 0 and routes[a].prevNodes.find(b) != routes[a].prevNodes.end()) {
				routeConstraints[i].select = 1;
				inTokens.push_back(b);
				outTokens.push_back(a);
				unassigned.erase(unassigned.begin()+u);
			} else if (b >= 0 and routes[b].prevNodes.find(a) != routes[b].prevNodes.end()) {
				routeConstraints[i].select = 0;
				inTokens.push_back(a);
				outTokens.push_back(b);
				unassigned.erase(unassigned.begin()+u);
			}

			// Propagate order decision through the group constraints
			if (routeConstraints[i].select >= 0) {
				set<int> prop = propagateRouteConstraint(i);
				for (auto j = prop.begin(); j != prop.end(); j++) {
					int from = routeConstraints[*j].select;
					inTokens.push_back(routeConstraints[*j].wires[from]);
					outTokens.push_back(routeConstraints[*j].wires[1-from]);
					//unassigned.erase(remove(unassigned.begin(), unassigned.end(), *j), unassigned.end());
				}
			}

			// TODO(edward.bingham) Did doing this create a cycle when we include violated via constraints? If so, we resolve that cycle by pushing the associated pins out as much as needed.
			// TODO(edward.bingham) In more advanced nodes, pushing the pin out may not be possible because poly routes are only allowed on a regular grid. So if we pushed the poly at all, we'd have to push it an entire grid unit. We may need to account for this in our placement algorithm. We may also be able to single out this pin from the route and route it separately. In that case, all of our previous understandings about the route direction assignments will change. So, this would have to be identified before running this algorithm.
		}
		if (inTokens.size() + outTokens.size() > 0) {
			buildPOffsets(tech, inTokens);
			buildNOffsets(tech, outTokens);
			continue;
		}

		// find the largest label
		int maxLabel = -1;
		int index = -1;
		int uindex = -1;
		for (int u = (int)unassigned.size()-1; u >= 0; u--) {
			// TODO(edward.bingham) If there is a direction that violates a via constraint and a direction that doesn't, then we pre-emptively chose the direction that doesn't. If both directions violate the via constraint, then we need to resolve that conflict by pushing the associated pins out to make space for the via.
			int i = unassigned[u];
			int label = max(
				routes[routeConstraints[i].wires[0]].pOffset + routes[routeConstraints[i].wires[1]].nOffset + routeConstraints[i].off[0], 
				routes[routeConstraints[i].wires[1]].pOffset + routes[routeConstraints[i].wires[0]].nOffset + routeConstraints[i].off[1]
			);

			if (label > maxLabel) {
				maxLabel = label;
				index = i;
				uindex = u;
			}
		}

		if (index >= 0) {
			int label0 = routes[routeConstraints[index].wires[0]].pOffset + routes[routeConstraints[index].wires[1]].nOffset + routeConstraints[index].off[0];
			int label1 = routes[routeConstraints[index].wires[1]].pOffset + routes[routeConstraints[index].wires[0]].nOffset + routeConstraints[index].off[1];

			// DESIGN(edward.bingham) this randomization implements gradient descent
			// for the route lowering algorithm
			if (label0 < label1 or (label0 == label1 and (rand()&1))) {
				routeConstraints[index].select = 0;
				inTokens.push_back(routeConstraints[index].wires[0]);
				outTokens.push_back(routeConstraints[index].wires[1]);
			} else {
				routeConstraints[index].select = 1;
				inTokens.push_back(routeConstraints[index].wires[1]);
				outTokens.push_back(routeConstraints[index].wires[0]);
			}
			unassigned.erase(unassigned.begin()+uindex);
	
			// Propagate order decision through the group constraints
			if (routeConstraints[index].select >= 0) {
				set<int> prop = propagateRouteConstraint(index);
				for (auto j = prop.begin(); j != prop.end(); j++) {
					int from = routeConstraints[*j].select;
					inTokens.push_back(routeConstraints[*j].wires[from]);
					outTokens.push_back(routeConstraints[*j].wires[1-from]);
					//unassigned.erase(remove(unassigned.begin(), unassigned.end(), *j), unassigned.end());
				}
			}

			buildPOffsets(tech, inTokens);
			buildNOffsets(tech, outTokens);
			continue;
		}
	}
}

void Router::findAndBreakPinCycles() {
	vector<vector<int> > cycles;
	findCycles(cycles);
	breakCycles(cycles);
}

void Router::lowerRoutes(const Tech &tech, int window) {
	// TODO(edward.bingham) There's still an interaction between route lowering
	// and via merging where it ends up creating a double route for two close
	// pins, causing DRC violations

	// TODO(edward.bingham) For routes that cross over one of the two transitor
	// stacks, I can do one of two things. 1. I could not lower that route,
	// keeping track of pin levels. 2. I could reduce the width of wide vias to
	// provide space for local interconnect and keep the higher layers open.
	// Manual layouts seem to prefer the second option.
	
	// int pinLevel = 1;

	// indexed by [route][pin]
	vector<vector<set<int> > > blockedLevels(routes.size(), vector<set<int> >());
	for (int i = 0; i < (int)routes.size(); i++) {
		for (int type = 0; type < (int)base->stack.size(); type++) {
			for (int j = 0; j < (int)base->stack[type].pins.size(); j++) {
				if (routes[i].pOffset >= base->stack[type].pins[j].lo and routes[i].pOffset <= base->stack[type].pins[j].hi and not routes[i].pins.empty()) {
					auto pos = lower_bound(routes[i].pins.begin(), routes[i].pins.end(), Index(type, j), CompareIndex(base));
					if (pos != routes[i].pins.begin() and pos != routes[i].pins.end() and pos->idx != Index(type, j)) {
						if (i >= (int)blockedLevels.size()) {
							blockedLevels.resize(i+1);
						}
						int index = (pos-routes[i].pins.begin())-1;
						if (index >= (int)blockedLevels[i].size()) {
							blockedLevels[i].resize(index+1);
						}
						blockedLevels[i][index].insert(base->stack[type].pins[j].layer);
					}
				}
			}
		}
	}

	for (int i = 0; i < (int)blockedLevels.size(); i++) {
		printf("blocked[%d] {", i);
		for (int j = 0; j < (int)blockedLevels[i].size(); j++) {
			printf("(");
			for (auto l = blockedLevels[i][j].begin(); l != blockedLevels[i][j].end(); l++) {
				printf("%d ", *l);
			}
			printf(") ");
		}
		printf("}\n");
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		vector<bool> types = routes[i].pinTypes();
		if ((not types[Model::PMOS] or not types[Model::NMOS]) and not routes[i].hasGate(base)) {
			continue;
		}
		for (int j = 0; j < (int)routes[i].pins.size()-1; j++) {
			int level = min(base->pin(routes[i].pins[j].idx).layer, base->pin(routes[i].pins[j+1].idx).layer);
			for (; level < (int)tech.wires.size(); level++) {
				bool found = false;
				for (int k = max(0, j-window); not found and k < min(j+window+1, (int)blockedLevels[i].size()); k++) {
					found = found or (blockedLevels[i][k].find(level) != blockedLevels[i][k].end());
				}
				if (not found) {
					break;
				}
			}
			if ((int)routes[i].level.size() < j+1) {
				routes[i].level.resize(j+1, 2);
			}
			routes[i].level[j] = level;
		}
	}
}

void Router::updateRouteConstraints(const Tech &tech) {
	// Compute route constraints
	for (auto con = routeConstraints.begin(); con != routeConstraints.end(); con++) {
		if (con->wires[0] >= 0 and con->wires[1] >= 0) {
			con->off[0] = 0;
			con->off[1] = 0;
			minOffset(con->off+0, tech, 1, routes[con->wires[0]].layout.layers, 0, routes[con->wires[1]].layout.layers, 0, Layout::DEFAULT, Layout::MERGENET);
			minOffset(con->off+1, tech, 1, routes[con->wires[1]].layout.layers, 0, routes[con->wires[0]].layout.layers, 0, Layout::DEFAULT, Layout::MERGENET);
		}
	}
}

int Router::computeCost() {
	// TODO(edward.bingham) This may be useful for a second placement round where
	// we use full cell area as the cost function. So, start with the simpler
	// cost function, do an initial layout, then use the layout size as the new
	// layout size as the cost function and each iteration does a full layout.
	// This may be like the detail step of the placement algorithm.
	int left = 1000000000;
	int right = -1000000000;
	for (int type = 0; type < 2; type++) {
		if (base->stack[type].pins.size() > 0 and base->stack[type].pins[0].pos < left) {
			left = base->stack[type].pins[0].pos;
		}
		if (base->stack[type].pins.size() > 0 and base->stack[type].pins.back().pos > right) {
			right = base->stack[type].pins.back().pos;
		}
	}

	//int cellHeightOverhead = 10;
	cost = cellHeight;//(cellHeightOverhead+right-left)*cellHeight*(int)(1+aStar.size());
	return cost;
}

int Router::solve(const Tech &tech) {
	buildPins(tech);
	buildHorizConstraints(tech);
	updatePinPos();
	alignPins(200);
	buildPinConstraints(tech, 0);
	print();
	//buildViaConstraints(tech);
	buildRoutes();
	buildContacts(tech);
	findAndBreakPinCycles();
	buildHorizConstraints(tech);
	updatePinPos();	
	drawRoutes(tech);

	buildRouteConstraints(tech);
	resetGraph(tech);
	assignRouteConstraints(tech);
	buildPinBounds();
	//alignPins(200);
	updatePinPos();
	drawRoutes(tech);

	for (int i = 0; i < 5; i++) {
		lowerRoutes(tech);
		buildContacts(tech);
		buildHorizConstraints(tech);
		updatePinPos();
		print();
		drawRoutes(tech);

		buildRouteConstraints(tech);
		buildGroupConstraints(tech);
		resetGraph(tech);
		assignRouteConstraints(tech);
		buildPinBounds();
		updatePinPos();
		drawRoutes(tech);
	}

	// TODO(edward.bingham) The route placement should start at the center and
	// work it's way toward the bottom and top of the cell instead of starting at
	// the bottom and working it's way to the top. This would make the cell more
	// dense overall, but give more space for overcell routing. I might want to
	// create directed routing constraints for power and ground that keeps them
	// at the bottom and top of the cell routing so that the two sources can be
	// easily routed in the larger context. Using pOffset and nOffset
	// alternatively didn't really work.
	/*
	int minOff = -1;
	int pOff = 0;
	int nOff = 0;
	for (int i = 0; i < (int)routes.size(); i++) {
		if (routes[i].pins.size() != 0) {
			int off = (routes[i].pOffset - routes[i].nOffset);
			off = off < 0 ? -off : off;
			if (minOff < 0 or off < minOff) {
				printf("route %d is center\n", i);
				minOff = off;
				pOff = routes[i].pOffset;
				nOff = routes[i].nOffset;
			}
		}
	}

	printf("off %d %d %d\n", minOff, pOff, nOff);
	for (int i = 0; i < (int)routes.size(); i++) {
		if (routes[i].pOffset < cellHeight/2) {
			routes[i].pos = cellHeight - routes[i].nOffset;
		} else {
			routes[i].pos = routes[i].pOffset;
		}
	}*/

	// TODO(edward.bingham) I may need to create the straps for power and ground
	// depending on the cell placement and global and local routing engine that
	// these cells are interfacing with.

	base->routes = routes;
	base->cellHeight = cellHeight;
	//print();
	return computeCost();
}

void Router::print() {
	printf("NMOS\n");
	for (int i = 0; i < (int)base->stack[0].pins.size(); i++) {
		const Pin &pin = base->stack[0].pins[i];
		printf("pin[%d] dev=%d nets=%d->%d->%d size=%dx%d pos=%d align=%d\n", i, pin.device, pin.leftNet, pin.outNet, pin.rightNet, pin.width, pin.height, pin.pos, pin.align);
	}

	printf("\nPMOS\n");
	for (int i = 0; i < (int)base->stack[1].pins.size(); i++) {
		const Pin &pin = base->stack[1].pins[i];
		printf("pin[%d] dev=%d nets=%d->%d->%d size=%dx%d pos=%d align=%d\n", i, pin.device, pin.leftNet, pin.outNet, pin.rightNet, pin.width, pin.height, pin.pos, pin.align);
	}

	printf("\nRoutes\n");
	for (int i = 0; i < (int)routes.size(); i++) {
		printf("wire[%d] %s(%d) %d->%d in:%d out:%d: ", i, (routes[i].net >= 0 and routes[i].net < (int)base->nets.size() ? base->nets[routes[i].net].name.c_str() : ""), routes[i].net, routes[i].left, routes[i].right, routes[i].pOffset, routes[i].nOffset);
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			printf("(%d,%d) ", routes[i].pins[j].idx.type, routes[i].pins[j].idx.pin);
		}
		printf("\n");
	}

	printf("\nStack Constraints\n");
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)base->stack[type].pins.size(); i++) {
			for (auto o = base->stack[type].pins[i].toPin.begin(); o != base->stack[type].pins[i].toPin.end(); o++) {
				printf("toPin (%d,%d) -> %d -> (%d,%d)\n", type, i, o->second, o->first.type, o->first.pin);
			}
		}
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		printf("route %d\n", i);
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			printf("\t(%d,%d) {", routes[i].pins[j].idx.type, routes[i].pins[j].idx.pin);
			for (auto k = routes[i].pins[j].fromPin.begin(); k != routes[i].pins[j].fromPin.end(); k++) {
				printf("(%d,%d)->%d ", k->first.type, k->first.pin, k->second);
			}
			for (auto k = routes[i].pins[j].toPin.begin(); k != routes[i].pins[j].toPin.end(); k++) {
				printf("%d->(%d,%d) ", k->second, k->first.type, k->first.pin);
			}
			printf("}\n");
		}
		printf("\n");
	}

	printf("\nRouting Constraints\n");
	for (int i = 0; i < (int)pinConstraints.size(); i++) {
		printf("pin[%d] %d -> %d\n", i, pinConstraints[i].from, pinConstraints[i].to);
	}
	for (int i = 0; i < (int)routeConstraints.size(); i++) {
		printf("route[%d] %d %s %d: %d,%d\n", i, routeConstraints[i].wires[0], (routeConstraints[i].select == 0 ? "->" : (routeConstraints[i].select == 1 ? "<-" : "--")), routeConstraints[i].wires[1], routeConstraints[i].off[0], routeConstraints[i].off[1]);
	}
	for (int i = 0; i < (int)viaConstraints.size(); i++) {
		printf("via[%d] {", i);
		for (auto j = viaConstraints[i].side[0].begin(); j != viaConstraints[i].side[0].end(); j++) {
			printf("(%d %d):%d ", j->idx.type, j->idx.pin, j->off);
		}
		printf("} -> (%d %d) -> {", viaConstraints[i].idx.type, viaConstraints[i].idx.pin);
		for (auto j = viaConstraints[i].side[1].begin(); j != viaConstraints[i].side[1].end(); j++) {
			printf("(%d %d):%d ", j->idx.type, j->idx.pin, j->off);
		}
		printf("}\n");
	}

	printf("\n");
}

