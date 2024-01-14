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
	
	layer = -1;
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

	layer = -1;
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
	height = 0;
	pos = 0;
	left = -1;
	right = -1;
}

Wire::Wire(int net) {
	this->net = net;
	this->layer = -1;
	this->height = 0;
	this->pos = 0;
	this->left = -1;
	this->right = -1;
}

Wire::~Wire() {
}

VerticalConstraint::VerticalConstraint() {
	from = -1;
	to = -1;
	off = 0;
}

VerticalConstraint::VerticalConstraint(int from, int to) {
	this->from = from;
	this->to = to;
	this->off = 0;
}

VerticalConstraint::~VerticalConstraint() {
}

HorizontalConstraint::HorizontalConstraint() {
	wires[0] = -1;
	wires[1] = -1;
	select = -1;
	off = 0;
}

HorizontalConstraint::HorizontalConstraint(int a, int b) {
	this->wires[0] = a;
	this->wires[1] = b;
	this->select = -1;
	this->off = 0;
}

HorizontalConstraint::~HorizontalConstraint() {
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
	// Determine location of each pin
	for (int type = 0; type < 2; type++) {
		int pos = 0;
		int lastModel = -1;
		for (int i = 0; i < (int)stack[type].size(); i++) {
			if (stack[type][i].device < 0) {
				stack[type][i].layer = tech.wires[1].drawingLayer;
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

				stack[type][i].layer = tech.wires[0].drawingLayer;
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

	// Create initial routes and constraints
	routes.reserve(base->nets.size());
	for (int i = 0; i < (int)base->nets.size(); i++) {
		routes.push_back(Wire(i));
	}
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < (int)stack[type].size(); i++) {
			routes[stack[type][i].outNet].pins.push_back(Index(type, i));
			if (routes[stack[type][i].outNet].left < 0 or
			    stack[type][i].pos < routes[stack[type][i].outNet].left) {
				routes[stack[type][i].outNet].left = stack[type][i].pos;
			}
			if (routes[stack[type][i].outNet].right < 0 or
			    stack[type][i].pos+stack[type][i].width > routes[stack[type][i].outNet].right) {
				routes[stack[type][i].outNet].right = stack[type][i].pos + stack[type][i].width;
			}
		}
	}

	// Compute the vertical constraints
	// TODO(edward.bingham) this could be more efficiently done as a 1d rectangle overlap problem
	for (int p = 0; p < (int)stack[Model::PMOS].size(); p++) {
		int pLeft = stack[Model::PMOS][p].pos;
		int pRight = pLeft + stack[Model::PMOS][p].width;
		int pNet = stack[Model::PMOS][p].outNet;

		for (int n = 0; n < (int)stack[Model::NMOS].size(); n++) {
			int nLeft = stack[Model::NMOS][n].pos;
			int nRight = nLeft + stack[Model::NMOS][n].width;
			int nNet = stack[Model::NMOS][n].outNet;

			if (pNet != nNet and pLeft < nRight and nLeft < pRight) {
				vert.push_back(VerticalConstraint(p, n));
			}
		}
	}

	// Compute horizontal constraints
	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = i+1; j < (int)routes.size(); j++) {
			if (routes[i].left < routes[j].right and routes[j].left < routes[i].right) {
				horiz.push_back(HorizontalConstraint(i, j));
			}
		}
	}

	for (int i = (int)routes.size()-1; i >= 0; i--) {
		if (routes[i].pins.size() < 2) {
			delRoute(i);
		}
	}

	// TODO moving indexes, spacing rules
	
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
		printf("wire %d %d->%d\n", routes[i].net, routes[i].left, routes[i].right);
	}

	printf("\nConstraints\n");
	for (int i = 0; i < (int)vert.size(); i++) {
		printf("vert %d -> %d\n", vert[i].from, vert[i].to);
	}
	for (int i = 0; i < (int)horiz.size(); i++) {
		printf("horiz %d -- %d\n", horiz[i].wires[0], horiz[i].wires[1]);
	}


	printf("\n\n");
}

void Solution::solve(const Tech &tech, int minCost) {
	// TODO handle cycles with doglegs
	// TODO search constraint graph
}

void draw(const Tech &tech) {
	// TODO draw result
}
