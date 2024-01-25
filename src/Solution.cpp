#include <algorithm>
#include <unordered_set>
#include <set>
#include <list>

#include "Solution.h"
#include "Timer.h"
#include "Draw.h"

Index::Index() {
	type = -1;
	pin = -1;
}

Index::Index(int type, int pin) {
	this->type = type;
	this->pin = pin;
}

Index::~Index() {
}

CompareIndex::CompareIndex(const Solution *s) {
	this->s = s;
}

CompareIndex::~CompareIndex() {
}

bool CompareIndex::operator()(const Index &i0, const Index &i1) {
	return s->stack[i0.type][i0.pin].pos < s->stack[i1.type][i1.pin].pos or
		(s->stack[i0.type][i0.pin].pos == s->stack[i1.type][i1.pin].pos and (i0.type > i1.type or
		(i0.type == i1.type and i0.pin < i1.pin)));
}

Pin::Pin() {
	device = -1;
	outNet = -1;
	leftNet = -1;
	rightNet = -1;
	
	layer = 0;
	width = 0;
	height = 0;
	off = 0;
	pos = 0;
}

Pin::Pin(int device, int outNet, int leftNet, int rightNet) {
	this->device = device;
	this->outNet = outNet;
	this->leftNet = leftNet;
	if (leftNet < 0) {
		this->leftNet = outNet;
	}
	this->rightNet = rightNet;
	if (rightNet < 0) {
		this->rightNet = outNet;
	}

	layer = 0;
	if (device < 0) {
		layer = 1;
	}
	width = 0;
	height = 0;
	off = 0;
	pos = 0;
}

Pin::~Pin() {
}

Wire::Wire() {
	net = -1;
	layer = -1;
	left = -1;
	right = -1;
	pOffset = 0;
	nOffset = 0;
}

Wire::Wire(int net, int layer) {
	this->net = net;
	this->layer = layer;
	this->left = -1;
	this->right = -1;
	this->pOffset = 0;
	this->nOffset = 0;
}

Wire::~Wire() {
}

void Wire::addPin(const Solution *s, Index pin) {
	auto pos = lower_bound(pins.begin(), pins.end(), pin, CompareIndex(s));
	pins.insert(pos, pin);
	if (left < 0 or s->stack[pin.type][pin.pin].pos < left) {
		left = s->stack[pin.type][pin.pin].pos;
	}
	if (right < 0 or s->stack[pin.type][pin.pin].pos+s->stack[pin.type][pin.pin].width > right) {
		right = s->stack[pin.type][pin.pin].pos + s->stack[pin.type][pin.pin].width;
	}
}

bool Wire::hasPin(const Solution *s, Index pin, vector<Index>::iterator *out) {
	auto pos = lower_bound(pins.begin(), pins.end(), pin, CompareIndex(s));
	if (out != nullptr) {
		*out = pos;
	}
	return pos != pins.end() and pos->type == pin.type and pos->pin == pin.pin;
}

PinConstraint::PinConstraint() {
	from = -1;
	to = -1;
	off = 0;
}

PinConstraint::PinConstraint(int from, int to, int off) {
	this->from = from;
	this->to = to;
	this->off = off;
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

Solution::Solution() {
	cycleCount = 0;
	cellHeight = 0;
	cost = 0;
	numContacts = 0;

	for (int i = 0; i < 2; i++) {
		dangling[i] = vector<int>();
		stack[i] = vector<Pin>();
		stackLayout[i] = Layout();
	}
}

Solution::Solution(const Circuit *ckt) {
	for (int i = 0; i < 2; i++) {
		dangling[i] = vector<int>();
		stack[i] = vector<Pin>();
		stackLayout[i] = Layout();
	}

	base = ckt;
	dangling[0].reserve(base->mos.size());
	dangling[1].reserve(base->mos.size());
	for (int i = 0; i < (int)base->mos.size(); i++) {
		dangling[base->mos[i].type].push_back(i);
	}
	numContacts = 0;
	cycleCount = 0;
	cellHeight = 0;
	cost = 0;
}

Solution::~Solution() {
}

// index into Solution::dangling
bool Solution::tryLink(vector<Solution*> &dst, int type, int index) {
	// Make sure that this transistor can be linked with the previous transistor
	// on the stack. We cannot link this transistor if there isn't another
	// transistor on the stack.
	if (stack[type].size() == 0) {
		return false;
	}

	// Sanity check to make sure this transistor actually has ports. This should
	// never fail as it should be guaranteed by the spice loader in Circuit.cpp
	int device = dangling[type][index];
	if (base->mos[device].ports.size() < 4) {
		printf("error parsing spice circuit, mos should have four ports\n");
		exit(1);
	}

	// Get information about the previous transistor on the stack. First if
	// statement in the funtion guarantees that there is at least one transistor
	// already on the stack.
	int prevNet = stack[type].back().rightNet;

	// This does two things:
	// 1. Determine if we can link this transistor to the previous one on the stack
	// 2. Determine whether we need to flip this transistor to do so
	int fromNet = base->mos[device].ports[Mos::SOURCE];
	int toNet = base->mos[device].ports[Mos::DRAIN];
	int gateNet = base->mos[device].ports[Mos::GATE];
	if (toNet == prevNet) {
		toNet = fromNet;
		fromNet = prevNet;
	} else if (fromNet != prevNet) {
		return false;
	}

	// duplicate solution
	Solution *next = new Solution(*this);
	if (next->stack[type].size() == 0 or base->nets[fromNet].ports > 2) {
		// Add a contact for the first net or between two transistors.
		next->numContacts++;
		next->stack[type].push_back(Pin(-next->numContacts, fromNet));
	}
	next->stack[type].push_back(Pin(device, gateNet, fromNet, toNet));

	// remove item from dangling
	next->dangling[type].erase(next->dangling[type].begin()+index);
	if (next->dangling[type].size() == 0) {
		next->numContacts++;
		next->stack[type].push_back(Pin(-next->numContacts, toNet));
	}

	dst.push_back(next);
	return true;
}

// index into Solution::dangling
bool Solution::push(vector<Solution*> &dst, int type, int index) {
	// Sanity check to make sure this transistor actually has ports. This should
	// never fail as it should be guaranteed by the spice loader in Circuit.cpp
	int device = dangling[type][index];
	if (base->mos[device].ports.size() < 4) {
		printf("error parsing spice circuit, mos should have four ports\n");
		exit(1);
	}

	int fromNet = base->mos[device].ports[Mos::SOURCE];
	int toNet = base->mos[device].ports[Mos::DRAIN];
	int gateNet = base->mos[device].ports[Mos::GATE];

	// We can't link this transistor to the previous one in the stack, so we
	// need to cap off the stack with a contact, start a new stack with a new
	// contact, then add this transistor. We need to test both the flipped and
	// unflipped orderings.

	// duplicate solution for the unflipped ordering
	for (int i = 0; i < 2; i++) {
		Solution *next = new Solution(*this);

		if (next->stack[type].size() > 0) {
			next->numContacts++;
			next->stack[type].push_back(Pin(-next->numContacts, stack[type].back().rightNet));
		}
		next->numContacts++;
		next->stack[type].push_back(Pin(-next->numContacts, fromNet));
		next->stack[type].push_back(Pin(device, gateNet, fromNet, toNet));
		// remove item from dangling
		next->dangling[type].erase(next->dangling[type].begin()+index);
		if (next->dangling[type].size() == 0) {
			next->numContacts++;
			next->stack[type].push_back(Pin(-next->numContacts, toNet));
		}
		dst.push_back(next);

		int tmp = fromNet;
		fromNet = toNet;
		toNet = tmp;
	}

	return true;
}

void Solution::delRoute(int route) {
	for (int i = (int)horiz.size()-1; i >= 0; i--) {
		if (horiz[i].wires[0] == route or horiz[i].wires[1] == route) {
			horiz.erase(horiz.begin()+i);
		} else {
			if (horiz[i].wires[0] > route) {
				horiz[i].wires[0]--;
			}
			if (horiz[i].wires[1] > route) {
				horiz[i].wires[1]--;
			}
		}
	}

	routes.erase(routes.begin()+route);
}

void Solution::build(const Tech &tech) {
	// Draw the pin contact
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)stack[type].size(); i++) {
			stack[type][i].width = pinWidth(Index(type, i));
			stack[type][i].height = pinHeight(Index(type, i));
			drawPin(tech, stack[type][i].pinLayout, this, type, i);
		}
	}

	// Determine location of each pin
	for (int type = 0; type < 2; type++) {
		int pos = 0;
		for (int i = 0; i < (int)stack[type].size(); i++) {
			stack[type][i].off = 0;
			if (i > 0) {
				minOffset(&stack[type][i].off, tech, 0, stack[type][i-1].pinLayout.layers, stack[type][i].pinLayout.layers, stack[type][i-1].device >= 0 or stack[type][i].device >= 0);
			}

			pos += stack[type][i].off;
			stack[type][i].pos = pos;
		}
	}

	// draw the pin via
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)stack[type].size(); i++) {
			drawVia(tech, stack[type][i].conLayout, stack[type][i].outNet, stack[type][i].layer, 2, vec2i(0,0), vec2i(stack[type][i].pos, 0));
		}
	}

	// Create initial routes
	routes.reserve(base->nets.size());
	for (int i = 0; i < (int)base->nets.size(); i++) {
		routes.push_back(Wire(i, 2));
	}
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)stack[type].size(); i++) {
			routes[stack[type][i].outNet].addPin(this, Index(type, i));
		}
	}

	// Compute the pin constraints
	// TODO(edward.bingham) this could be more efficiently done as a 1d rectangle overlap problem
	for (int p = 0; p < (int)stack[Model::PMOS].size(); p++) {
		if (routes[stack[Model::PMOS][p].outNet].pins.size() > 1) {
			for (int n = 0; n < (int)stack[Model::NMOS].size(); n++) {
				if (routes[stack[Model::NMOS][n].outNet].pins.size() > 1) {
					int off = 0;
					if (minOffset(&off, tech, 1, stack[Model::PMOS][p].conLayout.layers, stack[Model::NMOS][n].conLayout.layers)) {
						vert.push_back(PinConstraint(p, n, off));
					}
				}
			}
		}
	}

	for (int i = (int)routes.size()-1; i >= 0; i--) {
		if (routes[i].pins.size() < 2) {
			delRoute(i);
		}
	}
}

Pin &Solution::pin(Index i) {
	return stack[i.type][i.pin];
}

const Pin &Solution::pin(Index i) const {
	return stack[i.type][i.pin];
}

// horizontal size of pin
int Solution::pinWidth(Index p) const {
	int device = pin(p).device;
	if (device >= 0) {
		// this pin is a transistor, use length of transistor
		return base->mos[device].size[0];
	}
	// this pin is a contact
	return 0;
}

// vertical size of pin
int Solution::pinHeight(Index p) const {
	int device = pin(p).device;
	if (device >= 0) {
		// this pin is a transistor, use width of transistor
		return base->mos[device].size[1];
	}
	// this is a contact, height should be min of transistor widths on either side.
	int result = -1;
	if (p.pin > 0) {
		int leftDevice = stack[p.type][p.pin-1].device;
		if (leftDevice >= 0 and (result < 0 or base->mos[leftDevice].size[1] < result)) {
			result = base->mos[leftDevice].size[1];
		}
	}
	if (p.pin+1 < (int)stack[p.type].size()) {
		int rightDevice = stack[p.type][p.pin+1].device;
		if (rightDevice >= 0 and (result < 0 or base->mos[rightDevice].size[1] < result)) {
			result = base->mos[rightDevice].size[1];
		}
	}
	if (result < 0) {
		return 0;
	}
	return result;
}

vector<int> Solution::next(int r) {
	vector<int> pins;
	for (int i = 0; i < (int)vert.size(); i++) {
		for (int j = 0; j < (int)routes[r].pins.size(); j++) {
			if (routes[r].pins[j].type == Model::PMOS and vert[i].from == routes[r].pins[j].pin) {
				pins.push_back(vert[i].to);
				break;
			}
		}
	}

	vector<int> result;
	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			if (routes[i].pins[j].type == Model::NMOS and find(pins.begin(), pins.end(), routes[i].pins[j].pin) != pins.end()) {
				result.push_back(i);
				break;
			}
		}
	}

	for (int i = 0; i < (int)horiz.size(); i++) {
		if (horiz[i].select >= 0 and horiz[i].wires[horiz[i].select] == r and horiz[i].wires[1-horiz[i].select] >= 0) {
			result.push_back(horiz[i].wires[1-horiz[i].select]);
		}
	}

	sort(result.begin(), result.end());
	result.erase(unique(result.begin(), result.end()), result.end());
	return result;
}


bool Solution::findCycles(vector<vector<int> > &cycles, int maxCycles) {
	// DESIGN(edward.bingham) There can be multiple cycles with the same set of
	// nodes as a result of multiple pin constraints. This function does not
	// differentiate between those cycles. Doing so could introduce an
	// exponential blow up, and we can ensure that we split those cycles by
	// splitting on the node in the cycle that has the most pin constraints
	// (maximising min(in.size(), out.size()))
	unordered_set<int> seen;
	unordered_set<int> staged;
	
	list<vector<int> > tokens;
	tokens.push_back(vector<int>(1, 0));
	staged.insert(0);
	while (not tokens.empty()) {
		while (not tokens.empty()) {
			vector<int> curr = tokens.front();
			tokens.pop_front();

			vector<int> n = next(curr.back());
			for (int i = 0; i < (int)n.size(); i++) {
				auto loop = find(curr.begin(), curr.end(), n[i]);
				if (loop != curr.end()) {
					cycles.push_back(curr);
					cycles.back().erase(cycles.back().begin(), cycles.back().begin()+(loop-curr.begin()));
					vector<int>::iterator minElem = min_element(cycles.back().begin(), cycles.back().end());
					rotate(cycles.back().begin(), minElem, cycles.back().end());
					if (find(cycles.begin(), (cycles.end()-1), cycles.back()) != (cycles.end()-1)) {
						cycles.pop_back();
					}
				} else if (seen.find(n[i]) == seen.end()) {
					tokens.push_back(curr);
					tokens.back().push_back(n[i]);
					staged.insert(n[i]);
				}
			}

			if (maxCycles > 0 and (int)cycles.size() >= maxCycles) {
				return false;
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

	return true;
}

void Solution::breakRoute(int route, set<int> cycleRoutes) {
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

	int left = min(stack[0][0].pos, stack[1][0].pos);
	int right = max(stack[0].back().pos, stack[1].back().pos);
	int center = (left + right)/2;

	Wire wp(routes[route].net, routes[route].layer);
	Wire wn(routes[route].net, routes[route].layer);
	vector<int> count(routes[route].pins.size(), 0);
	bool wpHasGate = false;
	bool wnHasGate = false;

	// TODO(edward.bingham) is this correct?
	// Move all of the pins that participate in pin constraints associated
	// with the cycle.	
	for (int i = 0; i < (int)vert.size(); i++) {
		vector<Index>::iterator from = routes[route].pins.end();
		vector<Index>::iterator to = routes[route].pins.end();
		// A pin cannot have both a pin constraint in and a pin
		// constraint out because a pin is either PMOS or NMOS and pin
		// constraints always go out PMOS pins and in NMOS pins.

		// TODO(edward.bingham) This assumption may be invalidated by adding stack
		// constraints or via constraints because it is specific to pin
		// constraints.

		// First, check if this pin constraint is connected to any of the pins
		// in this route.
		bool hasFrom = routes[route].hasPin(this, Index(Model::PMOS, vert[i].from), &from);
		// error checking version
		//bool hasTo = routes[route].hasPin(this, Index(Model::NMOS, vert[i].to), &to);
		//if (not hasFrom and not hasTo) {
		//	continue;
		//} else if (hasFrom and hasTo) {
		//	printf("unitary cycle\n");
		//}

		// optimized version
		bool hasTo = false;
		if (not hasFrom) {
			hasTo = routes[route].hasPin(this, Index(Model::NMOS, vert[i].to), &to);
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
			found = ((hasFrom and routes[*other].hasPin(this, Index(Model::NMOS, vert[i].to))) or
							 (hasTo and routes[*other].hasPin(this, Index(Model::PMOS, vert[i].from))));
		}
		if (not found) {
			continue;
		}

		// Move the pin to wp or wn depending on hasFrom and hasTo
		if (hasFrom) {
			wp.addPin(this, *from);
			wpHasGate = wpHasGate or stack[from->type][from->pin].device >= 0;
			count.erase(count.begin()+fromIdx);
			routes[route].pins.erase(from);
		} else if (hasTo) {
			wn.addPin(this, *to);
			wnHasGate = wnHasGate or stack[to->type][to->pin].device >= 0;
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
	bool needAStar = false;
	int sharedPin = -1;
	int sharedCount = -1;
	bool sharedIsGate = true;
	int sharedDistanceFromCenter = 0;
	for (int i = 0; i < (int)routes[route].pins.size(); i++) {
		bool isGate = stack[routes[route].pins[i].type][routes[route].pins[i].pin].device >= 0;
		int pinPosition = stack[routes[route].pins[i].type][routes[route].pins[i].pin].pos;
		int distanceFromCenter = abs(pinPosition-center);

		if (sharedCount < 0 or (count[i] < sharedCount or
		    (count[i] == sharedCount and ((sharedIsGate and not isGate) or
		    (sharedIsGate == isGate and distanceFromCenter > sharedDistanceFromCenter))))) {
			sharedPin = i;
			sharedCount = count[i];
			sharedIsGate = isGate;
			sharedDistanceFromCenter = distanceFromCenter;
		}
	}

	if (sharedPin >= 0) {
		wp.addPin(this, routes[route].pins[sharedPin]);
		wn.addPin(this, routes[route].pins[sharedPin]);
		routes[route].pins.erase(routes[route].pins.begin()+sharedPin);
		count.erase(count.begin()+sharedPin);
		wpHasGate = wpHasGate or sharedIsGate;
		wnHasGate = wnHasGate or sharedIsGate;
	} else {
		needAStar = true;
		printf("Need A Star %d %d\n", route, (int)routes[route].pins.size());
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
			Index pin = routes[route].pins[i];
			bool isGate = stack[pin.type][pin.pin].device < 0;
			if (pin.type == Model::PMOS and not isGate) {
				wp.addPin(this, pin);
			} else {
				wn.addPin(this, pin);
				wnHasGate = wnHasGate or isGate;
			}
			routes[route].pins.pop_back();
			count.pop_back();
		}
	}	else if (not wnHasGate) {
		// Put all non-gate NMOS pins into wn and all remaining pins into wp
		for (int i = (int)routes[route].pins.size()-1; i >= 0; i--) {
			Index pin = routes[route].pins[i];
			bool isGate = stack[pin.type][pin.pin].device < 0;
			if (pin.type == Model::NMOS and not isGate) {
				wn.addPin(this, pin);
			} else {
				wp.addPin(this, pin);
				wpHasGate = wpHasGate or isGate;
			}
			routes[route].pins.pop_back();
			count.pop_back();
		}
	} else {
		for (int i = (int)routes[route].pins.size()-1; i >= 0; i--) {
			Index pin = routes[route].pins[i];
			// Determine horizontal location of wp and wn relative to sharedPin, then
			// place all pins on same side of sharedPin as wp or wn into that wp or wn
			// respectively.
			bool isGate = stack[pin.type][pin.pin].device < 0;
			int pinPos = stack[pin.type][pin.pin].pos;
			if (pinPos >= wn.left and pinPos >= wp.left and pinPos <= wn.right and pinPos <= wp.right) {
				if (pin.type == Model::PMOS) {
					wp.addPin(this, pin);
					wpHasGate = wpHasGate or isGate;
				} else {
					wn.addPin(this, pin);
					wnHasGate = wnHasGate or isGate;
				}
			} else if (pinPos >= wn.left and pinPos <= wn.right) {
				wn.addPin(this, pin);
				wnHasGate = wnHasGate or isGate;
			} else if (pinPos >= wp.left and pinPos <= wp.right) {
				wp.addPin(this, pin);
				wpHasGate = wpHasGate or isGate;
			} else if (min(abs(pinPos-wn.right),abs(pinPos-wn.left)) < min(abs(pinPos-wp.right),abs(pinPos-wp.left))) {
				wn.addPin(this, pin);
				wnHasGate = wnHasGate or isGate;
			} else {
				wp.addPin(this, pin);
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
	routes[route].layer = wp.layer;
	routes[route].left = wp.left;
	routes[route].right = wp.right;
	routes[route].pOffset = wp.pOffset;
	routes[route].nOffset = wp.nOffset;
	if (needAStar) {
		aStar.push_back(pair<int, int>(route, (int)routes.size()));
	}
	routes.push_back(wn);
}

void Solution::breakCycles(vector<vector<int> > cycles) {
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
	for (int i = 0; i < (int)vert.size(); i++) {
		for (int j = 0; j < (int)routes.size(); j++) {
			if (routes[j].hasPin(this, Index(Model::PMOS, vert[i].from))) {
				numOut[j]++;
			}
			if (routes[j].hasPin(this, Index(Model::NMOS, vert[i].to))) {
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
	//for (int i = 0; i < (int)stack[0].size(); i++) {
	//	printf("pin %d %d->%d->%d: %dx%d %d %d\n", stack[0][i].device, stack[0][i].leftNet, stack[0][i].outNet, stack[0][i].rightNet, stack[0][i].width, stack[0][i].height, stack[0][i].off, stack[0][i].pos);
	//}

	//printf("\nPMOS\n");
	//for (int i = 0; i < (int)stack[1].size(); i++) {
	//	printf("pin %d %d->%d->%d: %dx%d %d %d\n", stack[1][i].device, stack[1][i].leftNet, stack[1][i].outNet, stack[1][i].rightNet, stack[1][i].width, stack[1][i].height, stack[1][i].off, stack[1][i].pos);
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
	//for (int i = 0; i < (int)vert.size(); i++) {
	//	printf("vert %d -> %d: %d\n", vert[i].from, vert[i].to, vert[i].off);
	//}
	//for (int i = 0; i < (int)horiz.size(); i++) {
	//	printf("horiz %d -- %d: %d\n", horiz[i].wires[0], horiz[i].wires[1], horiz[i].off);
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
		//     a <--- d        a b a b a c b d
		//   ^^ |||   ^        | | | | | | | |
		//   || vvv   |        v v v v v v v v
		//     b ---> c        b a b a b d c a
		//
		//
		//  a1 -> a0 <--- d    a b a b a c b d   nodes
		//  \\\   ^^      ^    | | | | | | | |
		//   vvv //       |  o-o-|-o-|-o | | |     a1
		//      b ------> c  |   |   |   | | |    vvv\
		//                   | o-o-o-o-o-|-o |     b  \
		//                   | |   |   | |   |     v\\|
		//                   | |   |   | o-o |     c|||
		//                   | |   |   |   | |     v|||
		//                   | |   |   | o-|-o     d|||
		//                   | |   |   | | |       vvvv
		//                   o---o-|-o-|-|-|-o     a0
		//                     | | | | | | | |
		//                     b a b a b d c a
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
			if ((int)cycleCount[i].size() > maxCycleCount or
					((int)cycleCount[i].size() == maxCycleCount and density > maxDensity)) {
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

void Solution::buildRouteConstraints(const Tech &tech) {
	// Draw the stacks
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)stack[type].size(); i++) {
			drawLayout(stackLayout[type], stack[type][i].pinLayout, vec2i(stack[type][i].pos, 0), vec2i(1, type == Model::NMOS ? -1 : 1));
		}
	}

	// Draw the routes
	for (int i = 0; i < (int)routes.size(); i++) {
		drawWire(tech, routes[i].layout, this, routes[i]);
	}

	// Compute stack constraints
	for (int i = 0; i < (int)routes.size(); i++) {
		// PMOS stack to route
		int off = 0;
		if (minOffset(&off, tech, 1, stackLayout[Model::PMOS].layers, routes[i].layout.layers)) {
			horiz.push_back(RouteConstraint(PMOS_STACK, i, off, 0, 0));
		}

		// Route to NMOS stack
		off = 0;
		if (minOffset(&off, tech, 1, routes[i].layout.layers, stackLayout[Model::NMOS].layers)) {
			horiz.push_back(RouteConstraint(i, NMOS_STACK, off, 0, 0));
		}
	}

	// Compute route constraints
	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = i+1; j < (int)routes.size(); j++) {
			int off[2] = {0,0};
			bool fromto = minOffset(off+0, tech, 1, routes[i].layout.layers, routes[j].layout.layers);
			bool tofrom = minOffset(off+1, tech, 1, routes[j].layout.layers, routes[i].layout.layers);
			if (fromto or tofrom) {
				horiz.push_back(RouteConstraint(i, j, off[0], off[1]));
			}
		}
	}
}

// make sure the graph is acyclic before running this
vector<int> Solution::findTop() {
	for (int i = 0; i < (int)horiz.size(); i++) {
		if (horiz[i].select >= 0 and horiz[i].wires[horiz[i].select] == PMOS_STACK) {
			return vector<int>(1, PMOS_STACK);
		}
	}

	// set up initial tokens for evaluating pin constraints
	vector<int> tokens;
	for (int i = 0; i < (int)routes.size(); i++) {
		bool found = false;
		for (int j = 0; not found and j < (int)vert.size(); j++) {
			found = found or routes[i].hasPin(this, Index(Model::NMOS, vert[j].to));
		}
		for (int j = 0; not found and j < (int)horiz.size(); j++) {
			found = found or (horiz[j].select >= 0 and horiz[j].wires[1-horiz[j].select] == i);
		}
		if (not found) {
			tokens.push_back(i);
		}
	}
	
	return tokens;
}

// make sure the graph is acyclic before running this
vector<int> Solution::findBottom() {
	for (int i = 0; i < (int)horiz.size(); i++) {
		if (horiz[i].select >= 0 and horiz[i].wires[horiz[i].select] == NMOS_STACK) {
			return vector<int>(1, NMOS_STACK);
		}
	}

	// set up initial tokens for evaluating pin constraints
	vector<int> tokens;
	for (int i = 0; i < (int)routes.size(); i++) {
		bool found = false;
		for (int j = 0; not found and j < (int)vert.size(); j++) {
			found = found or routes[i].hasPin(this, Index(Model::PMOS, vert[j].from));
		}
		for (int j = 0; not found and j < (int)horiz.size(); j++) {
			found = found or (horiz[j].select >= 0 and horiz[j].wires[horiz[j].select] == i);
		}
		if (not found) {
			tokens.push_back(i);
		}
	}
	
	return tokens;
}

void Solution::zeroWeights() {
	for (int i = 0; i < (int)routes.size(); i++) {
		routes[i].pOffset = 0;
		routes[i].nOffset = 0;
		routes[i].prevNodes.clear();
	}
}

void Solution::buildPOffsets(const Tech &tech, vector<int> start) {
	vector<int> tokens = start;
	while (tokens.size() > 0) {
		int curr = tokens.back();
		tokens.pop_back();
		
		if (curr >= 0) {
			for (int i = 0; i < (int)vert.size(); i++) {
				if (routes[curr].hasPin(this, Index(Model::PMOS, vert[i].from))) {
					int weight = routes[curr].pOffset + vert[i].off;
					for (int j = 0; j < (int)routes.size(); j++) {
						if (j != curr and routes[j].hasPin(this, Index(Model::NMOS, vert[i].to))) {
							bool change = routes[j].prevNodes.insert(curr).second;
							for (auto prev = routes[curr].prevNodes.begin(); prev != routes[curr].prevNodes.end(); prev++) {
								bool inserted = routes[j].prevNodes.insert(*prev).second;
								change = change or inserted;
							}

							if (routes[j].pOffset < weight) {
								routes[j].pOffset = weight;
								change = true;
							}

							if (change) {
								tokens.push_back(j);
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < (int)horiz.size(); i++) {
			if (horiz[i].select >= 0 and curr == horiz[i].wires[horiz[i].select]) {
				int weight = (curr < 0 ? 0 : routes[curr].pOffset) + horiz[i].off[horiz[i].select];
				int out = horiz[i].wires[1-horiz[i].select];
				if (out == NMOS_STACK) {
					if (cellHeight < weight) {
						cellHeight = weight;
					}
					continue;
				} else if (out < 0) {
					printf("i=%d, sel=%d from=%d to=%d\n", i, horiz[i].select, horiz[i].wires[horiz[i].select], horiz[i].wires[1-horiz[i].select]);
					printf("out should never be PMOS_STACK %d==%d\n", out, PMOS_STACK);
					continue;
				}

				// keep track of anscestor nodes
				bool change = false;
				if (curr >= 0) {
					change = routes[out].prevNodes.insert(curr).second;
					for (auto prev = routes[curr].prevNodes.begin(); prev != routes[curr].prevNodes.end(); prev++) {
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
					tokens.push_back(out);
				}
			}
		}
	}
}

void Solution::buildNOffsets(const Tech &tech, vector<int> start) {
	vector<int> tokens = start;
	while (tokens.size() > 0) {
		int curr = tokens.back();
		tokens.pop_back();
	
		if (curr >= 0) {	
			for (int i = 0; i < (int)vert.size(); i++) {
				if (routes[curr].hasPin(this, Index(Model::NMOS, vert[i].to))) {
					int weight = routes[curr].nOffset + vert[i].off;
					for (int j = 0; j < (int)routes.size(); j++) {
						if (j != curr and routes[j].hasPin(this, Index(Model::PMOS, vert[i].from))) {
							if (routes[j].nOffset < weight) {
								routes[j].nOffset = weight;
								tokens.push_back(j);
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < (int)horiz.size(); i++) {
			if (horiz[i].select >= 0 and curr == horiz[i].wires[1-horiz[i].select]) {
				int weight = (curr < 0 ? 0 : routes[curr].nOffset) + horiz[i].off[horiz[i].select];
				int in = horiz[i].wires[horiz[i].select];
				if (in >= 0 and routes[in].nOffset < weight) {
					routes[in].nOffset = weight;
					tokens.push_back(in);
				}
			}
		}
	}
}

bool Solution::solve(const Tech &tech, int maxCost, int maxCycles) {
	//Timer timer;

	print();

	vector<vector<int> > cycles;
	if (not findCycles(cycles, maxCycles)) {
		return false;
	}
	cycleCount = (int)cycles.size();

	//printf("findcycles %d %fms\n", (int)cycles.size(), timer.since()*1e3);
	//timer.reset();

	breakCycles(cycles);

	//printf("breakcycles %fms\n", timer.since()*1e3);
	//timer.reset();

	buildRouteConstraints(tech);

	//printf("horiz %fms\n", timer.since()*1e3);
	//timer.reset();

	zeroWeights();
	buildPOffsets(tech);

	//printf("in %fms\n", timer.since()*1e3);
	//timer.reset();

	buildNOffsets(tech);

	//printf("out %fms\n", timer.since()*1e3);
	//timer.reset();

	vector<int> unassigned;
	unassigned.reserve(horiz.size());
	for (int i = 0; i < (int)horiz.size(); i++) {
		if (horiz[i].select < 0) {
			unassigned.push_back(i);
		}
	}

	//printf("setup %fms\n", timer.since()*1e3);
	//timer.reset();

	while (unassigned.size() > 0) {
		// handle critical constraints, that would create cycles if assigned the wrong direction.
		vector<int> inTokens, outTokens;
		for (int u = (int)unassigned.size()-1; u >= 0; u--) {
			int i = unassigned[u];
			if (horiz[i].wires[0] >= 0 and routes[horiz[i].wires[0]].prevNodes.find(horiz[i].wires[1]) != routes[horiz[i].wires[0]].prevNodes.end()) {
				horiz[i].select = 1;
				inTokens.push_back(horiz[i].wires[1]);
				outTokens.push_back(horiz[i].wires[0]);
				unassigned.erase(unassigned.begin()+u);
			} else if (horiz[i].wires[1] >= 0 and routes[horiz[i].wires[1]].prevNodes.find(horiz[i].wires[0]) != routes[horiz[i].wires[1]].prevNodes.end()) {
				horiz[i].select = 0;
				inTokens.push_back(horiz[i].wires[0]);
				outTokens.push_back(horiz[i].wires[1]);
				unassigned.erase(unassigned.begin()+u);
			}
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
			int i = unassigned[u];
			int label = max(
				routes[horiz[i].wires[0]].pOffset + routes[horiz[i].wires[1]].nOffset + horiz[i].off[0], 
				routes[horiz[i].wires[1]].pOffset + routes[horiz[i].wires[0]].nOffset + horiz[i].off[1]
			);

			if (label > maxLabel) {
				maxLabel = label;
				index = i;
				uindex = u;
			}
		}

		if (index >= 0) {
			int label0 = routes[horiz[index].wires[0]].pOffset + routes[horiz[index].wires[1]].nOffset + horiz[index].off[0];
			int label1 = routes[horiz[index].wires[1]].pOffset + routes[horiz[index].wires[0]].nOffset + horiz[index].off[1];

			if (label0 < label1) {
				horiz[index].select = 0;
				inTokens.push_back(horiz[index].wires[0]);
				outTokens.push_back(horiz[index].wires[1]);
			} else {
				horiz[index].select = 1;
				inTokens.push_back(horiz[index].wires[1]);
				outTokens.push_back(horiz[index].wires[0]);
			}

			unassigned.erase(unassigned.begin()+uindex);
			buildPOffsets(tech, inTokens);
			buildNOffsets(tech, outTokens);
			continue;
		}
	}

	// TODO(edward.bingham) handle A* TODOs
	//printf("solve %fms\n", timer.since()*1e3);

	int left = 1000000000;
	int right = -1000000000;
	for (int type = 0; type < 2; type++) {
		if (stack[type].size() > 0 and stack[type][0].pos < left) {
			left = stack[type][0].pos;
		}
		if (stack[type].size() > 0 and stack[type].back().pos > right) {
			right = stack[type].back().pos;
		}
	}

	cost = (right-left)*cellHeight*(int)(1+aStar.size());
	//printf("%d * %d = %d\n", (right-left), cellHeight, cost);

	if (maxCost > 0 and cost >= maxCost)
		return false;

	return true;
}

void Solution::print() {
	printf("NMOS\n");
	for (int i = 0; i < (int)stack[0].size(); i++) {
		printf("pin %d %d->%d->%d: %dx%d %d %d\n", stack[0][i].device, stack[0][i].leftNet, stack[0][i].outNet, stack[0][i].rightNet, stack[0][i].width, stack[0][i].height, stack[0][i].off, stack[0][i].pos);
	}

	printf("\nPMOS\n");
	for (int i = 0; i < (int)stack[1].size(); i++) {
		printf("pin %d %d->%d->%d: %dx%d %d %d\n", stack[1][i].device, stack[1][i].leftNet, stack[1][i].outNet, stack[1][i].rightNet, stack[1][i].width, stack[1][i].height, stack[1][i].off, stack[1][i].pos);
	}

	printf("\nRoutes\n");
	for (int i = 0; i < (int)routes.size(); i++) {
		printf("wire %d %d->%d in:%d out:%d: ", routes[i].net, routes[i].left, routes[i].right, routes[i].pOffset, routes[i].nOffset);
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			printf("(%d,%d) ", routes[i].pins[j].type, routes[i].pins[j].pin);
		}
		printf("\n");
	}

	printf("\nConstraints\n");
	for (int i = 0; i < (int)vert.size(); i++) {
		printf("vert %d -> %d: %d\n", vert[i].from, vert[i].to, vert[i].off);
	}
	for (int i = 0; i < (int)horiz.size(); i++) {
		printf("horiz %d %s %d: %d,%d\n", horiz[i].wires[0], (horiz[i].select == 0 ? "->" : (horiz[i].select == 1 ? "<-" : "--")), horiz[i].wires[1], horiz[i].off[0], horiz[i].off[1]);
	}

	printf("\nA Star TODOs\n");
	for (int i = 0; i < (int)aStar.size(); i++) {
		printf("astar %d %d\n", aStar[i].first, aStar[i].second);
	}

	printf("\n");
}

void Solution::draw(const Tech &tech, Layout &dst) {
	vec2i dir(1,1);
	dst.name = base->name;

	dst.nets.reserve(base->nets.size());
	for (int i = 0; i < (int)base->nets.size(); i++) {
		dst.nets.push_back(base->nets[i].name);
	}

	for (int type = 0; type < 2; type++) {
		drawLayout(dst, stackLayout[type], vec2i(0, (type == Model::NMOS)*cellHeight)*dir, dir);
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		drawRoute(tech, dst, this, routes[i], vec2i(0,0), dir);
	}	

	for (int i = 0; i < (int)dst.layers.size(); i++) {
		if (tech.mats[dst.layers[i].draw].fill) {
			Rect box = dst.layers[i].bbox();
			dst.layers[i].clear();
			dst.layers[i].push(box, true);
		}
	}

	dst.merge();
}
