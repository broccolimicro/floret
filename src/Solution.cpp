#include "Solution.h"
#include <algorithm>

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

Wire::Wire(int net, int layer, int height) {
	this->net = net;
	this->layer = layer;
	this->height = height;
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

VerticalConstraint::VerticalConstraint(int from, int to, int off) {
	this->from = from;
	this->to = to;
	this->off = off;
}

VerticalConstraint::~VerticalConstraint() {
}

HorizontalConstraint::HorizontalConstraint() {
	wires[0] = -1;
	wires[1] = -1;
	select = -1;
	off = 0;
}

HorizontalConstraint::HorizontalConstraint(int a, int b, int off) {
	this->wires[0] = a;
	this->wires[1] = b;
	this->select = -1;
	this->off = off;
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
		routes.push_back(Wire(i, tech.wires[2].drawingLayer, tech.layers[tech.vias[2].drawingLayer].minWidth+max(tech.vias[2].upLo, tech.vias[2].downLo)));
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

		if (routes[pNet].pins.size() > 1) {
			for (int n = 0; n < (int)stack[Model::NMOS].size(); n++) {
				int nLeft = stack[Model::NMOS][n].pos;
				int nRight = nLeft + stack[Model::NMOS][n].width;
				int nNet = stack[Model::NMOS][n].outNet;

				if (pNet != nNet and pLeft < nRight and nLeft < pRight and routes[nNet].pins.size() > 1) {
					vert.push_back(VerticalConstraint(p, n, tech.layers[tech.wires[2].drawingLayer].minSpacing));
				}
			}
		}
	}

	// Compute horizontal constraints
	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = i+1; j < (int)routes.size(); j++) {
			if (routes[i].left < routes[j].right and routes[j].left < routes[i].right) {
				horiz.push_back(HorizontalConstraint(i, j, tech.layers[tech.wires[2].drawingLayer].minSpacing));
			}
		}
	}

	for (int i = (int)routes.size()-1; i >= 0; i--) {
		if (routes[i].pins.size() < 2) {
			delRoute(i);
		}
	}
}

vector<int> Solution::outVert(int r) {
	vector<int> result;
	for (int i = 0; i < (int)vert.size(); i++) {
		for (int j = 0; j < (int)routes[r].pins.size(); j++) {
			if (routes[r].pins[j].type == Model::PMOS and vert[i].from == routes[r].pins[j].pin) {
				result.push_back(i);
				break;
			}
		}
	}
	return result;
}

vector<int> Solution::inVert(int r) {
	vector<int> result;
	for (int i = 0; i < (int)vert.size(); i++) {
		for (int j = 0; j < (int)routes[r].pins.size(); j++) {
			if (routes[r].pins[j].type == Model::NMOS and vert[i].to == routes[r].pins[j].pin) {
				result.push_back(i);
				break;
			}
		}
	}
	return result;
}

vector<int> Solution::vertOut(int v) {
	vector<int> result;
	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			if (routes[i].pins[j].type == Model::NMOS and vert[v].to == routes[i].pins[j].pin) {
				result.push_back(i);
				break;
			}
		}
	}
	return result;
}

vector<int> Solution::vertIn(int v) {
	vector<int> result;
	for (int i = 0; i < (int)routes.size(); i++) {
		for (int j = 0; j < (int)routes[i].pins.size(); j++) {
			if (routes[i].pins[j].type == Model::PMOS and vert[v].from == routes[i].pins[j].pin) {
				result.push_back(i);
				break;
			}
		}
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
	return result;
}


vector<vector<int> > Solution::findCycles(bool searchHoriz) {
	vector<vector<int> > tokens;
	vector<vector<int> > cycles;
	for (int i = 0; i < (int)routes.size(); i++) {
		tokens.push_back(vector<int>(1, i));
	}

	while (tokens.size() > 0) {
		vector<int> curr = tokens.back();
		tokens.pop_back();
		// check to make sure this token hasn't entered a loop we've already found
		bool found = false;
		for (int i = 0; not found and i < (int)cycles.size(); i++) {
			found = find(cycles[i].begin(), cycles[i].end(), curr.back()) != cycles[i].end();
		}
		if (found) {
			continue;
		}

		vector<int> n = next(curr.back());
		for (int j = 0; j < (int)n.size(); j++) {
			tokens.push_back(curr);

			// check to see if we've eaten our tail
			auto token = find(tokens.back().begin(), tokens.back().end(), n[j]);
			if (token != tokens.back().end()) {
				// extract the cycle from our trace
				tokens.back().erase(tokens.back().begin(), token);
				vector<int>::iterator minElem = min_element(tokens.back().begin(), tokens.back().end());
				rotate(tokens.back().begin(), minElem, tokens.back().end());
				//if (find(cycles.begin(), cycles.end(), tokens.back()) == cycles.end()) {
					cycles.push_back(tokens.back());
				//}
				tokens.pop_back();
			} else {
				tokens.back().push_back(n[j]);
			}
		}
	}
	return cycles;
}

// make sure the graph is acyclic before running this
vector<int> Solution::initialTokens(bool searchHoriz) {
	// set up initial tokens for evaluating vertical constraints
	vector<int> tokens;
	for (int i = 0; i < (int)routes.size(); i++) {
		bool found = false;
		for (int j = 0; not found and j < (int)vert.size(); j++) {
			for (int k = 0; not found and k < (int)routes[i].pins.size(); k++) {
				found = found or (vert[j].to == routes[i].pins[k].pin);
			}
		}
		if (searchHoriz) {
			for (int j = 0; not found and j < (int)horiz.size(); j++) {
				found = found or (horiz[j].select >= 0 and horiz[j].wires[1-horiz[j].select] == i);
			}
		}
		if (not found) {
			tokens.push_back(i);
		}
	}
	
	return tokens;
}

void Solution::solve(const Tech &tech, int minCost) {
	// Cycles can show up in the vertical constraints without ever looking at the horizontal constraints

	// TODO handle cycles with doglegs
	// TODO search constraint graph

	vector<vector<int> > cycles = findCycles();

	/*vector<int> tokens = initialTokens();
	printf("Initial Tokens\n");
	for (int i = 0; i < (int)tokens.size(); i++) {
		printf("token %d\n", tokens[i]);
	}*/

	printf("Cycles\n");
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
		printf("wire %d %d->%d: %d\n", routes[i].net, routes[i].left, routes[i].right, routes[i].height);
	}

	printf("\nConstraints\n");
	for (int i = 0; i < (int)vert.size(); i++) {
		printf("vert %d -> %d: %d\n", vert[i].from, vert[i].to, vert[i].off);
	}
	for (int i = 0; i < (int)horiz.size(); i++) {
		printf("horiz %d -- %d: %d\n", horiz[i].wires[0], horiz[i].wires[1], horiz[i].off);
	}

	printf("\n\n");
}

void Solution::draw(const Tech &tech) {
	// TODO draw result
}
