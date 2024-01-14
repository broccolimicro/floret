#include "Solution.h"

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

Pin::Pin() {
	device = -1;
	outNet = -1;
	leftNet = -1;
	rightNet = -1;
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
}

Pin::~Pin() {
}

Wire::Wire() {
	net = -1;
	layer = -1;
}

Wire::Wire(int net) {
	this->net = net;
	this->layer = -1;
}

Wire::~Wire() {
}

Solution::Solution() {
}

Solution::Solution(const Circuit *ckt) {
	base = ckt;
	dangling[0].reserve(base->mos.size());
	dangling[1].reserve(base->mos.size());
	for (int i = 0; i < (int)base->mos.size(); i++) {
		dangling[base->mos[i].type].push_back(i);
	}
	numContacts = 0;
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

void Solution::build(const Tech &tech) {
	// Determine location of each pin
	for (int type = 0; type < 2; type++) {
		int pos = 0;
		int lastModel = -1;
		for (int i = 0; i < (int)stack[type].size(); i++) {
			if (stack[type][i].device < 0) {
				// this is a contact
				stack[type][i].width = tech.layers[tech.vias[0].drawingLayer].minWidth;

				// contact height is min of transistor widths on either side.
				stack[type][i].height = 0;
				if (i-1 >= 0 and stack[type][i-1].device >= 0 and (stack[type][i].height == 0 or base->mos[stack[type][i-1].device].width < stack[type][i].height)) {
					stack[type][i].height = base->mos[stack[type][i-1].device].width;
				}
				if (i+1 < (int)stack[type].size() and stack[type][i+1].device >= 0 and (stack[type][i].height == 0 or base->mos[stack[type][i+1].device].width < stack[type][i].height)) {
					stack[type][i].height = base->mos[stack[type][i+1].device].width;
				}

				stack[type][i].off = 0;
				if (i-1 >= 0 and lastModel >= 0) {
					if (stack[type][i-1].device >= 0) {
						// previous pin was a transistor
						stack[type][i].off = tech.models[lastModel].viaPolySpacing;
					} else {
						// previous pin was a contact and there was a transistor before it
						stack[type][i].off = tech.vias[0].downLo*2 + tech.layers[tech.models[lastModel].layers[0].layer].minSpacing;
					}
				}
			} else {
				// this is a transistor
				lastModel = base->mos[stack[type][i].device].model;

				stack[type][i].width = base->mos[stack[type][i].device].length;
				stack[type][i].height = base->mos[stack[type][i].device].width;

				stack[type][i].off = 0;
				if (i-1 >= 0 and lastModel >= 0) {
					if (stack[type][i-1].device >= 0) {
						// previous pin was a transistor
						stack[type][i].off = tech.layers[tech.wires[0].drawingLayer].minSpacing;
					} else {
						stack[type][i].off = tech.vias[0].downLo*2 + tech.layers[tech.models[lastModel].layers[0].layer].minSpacing;
					}
				}
			}

			stack[type][i].pos = pos + stack[type][i].off;
			pos = stack[type][i].pos + stack[type][i].width; 
		}
	}

	// Create initial routes
	routes.reserve(base->nets.size());
	for (int i = 0; i < (int)base->nets.size(); i++) {
		routes.push_back(Wire(i));
	}
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)stack[type].size(); i++) {
			routes[stack[type][i].outNet].pins.push_back(Index(type, i));
		}
	}

	// Add vertical and horizontal constraints
	//for (int i = 0; i < routes.size(); 

	printf("NMOS\n");
	for (int i = 0; i < (int)stack[0].size(); i++) {
		printf("pin %d %d->%d->%d: %dx%d %d %d\n", stack[0][i].device, stack[0][i].leftNet, stack[0][i].outNet, stack[0][i].rightNet, stack[0][i].width, stack[0][i].height, stack[0][i].off, stack[0][i].pos);
	}

	printf("\nPMOS\n");
	for (int i = 0; i < (int)stack[1].size(); i++) {
		printf("pin %d %d->%d->%d: %dx%d %d %d\n", stack[1][i].device, stack[1][i].leftNet, stack[1][i].outNet, stack[1][i].rightNet, stack[1][i].width, stack[1][i].height, stack[1][i].off, stack[1][i].pos);
	}

	/*printf("\nRoutes\n");
	for (int i = 0; i < (int)routes.size(); i++) {
		printf("wire %d from %d:%d to %d:%d\n", routes[i].net, routes[i].from.device, routes[i].from.port, routes[i].to.device, routes[i].to.port);
	}*/

	//printf("\n\n");
	
}

void Solution::solve(const Tech &tech, int minCost) {

}

void draw(const Tech &tech) {
}
