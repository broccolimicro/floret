#include "Solution.h"

Solution::Solution() {
}

Solution::Solution(const Circuit *ckt) {
	base = ckt;
	dangling[0].reserve(base->mos.size());
	dangling[1].reserve(base->mos.size());
	for (int i = 0; i < (int)base->mos.size(); i++) {
		dangling[base->mos[i].type].push_back(i);
	}
	contacts = 0;
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
		printf("error parsing spice circuit, mos should have four ports\n")
		exit(1);
	}

	// Get information about the previous transistor on the stack. First if
	// statement in the funtion guarantees that there is at least one transistor
	// already on the stack.
	int prev = stack[type].back().device;
	int prevNet = base->mos[prev].ports[Mos::DRAIN];
	if (stack[type].back().flipped) {
		prevNet = base->mos[prev].ports[Mos::SOURCE];
	}

	// This does two things:
	// 1. Determine if we can link this transistor to the previous one on the stack
	// 2. Determine whether we need to flip this transistor to do so
	int fromPort = Mos::SOURCE;
	int toPort = Mos::DRAIN;
	int from = base->mos[device].ports[fromPort];
	int to = base->mos[device].ports[toPort];
	if (to == prevNet) {
		to = from;
		from = prevNet;
		fromPort = Mos::DRAIN;
		toPort = Mos::SOURCE;
	} else if (from != prevNet) {
		return false;
	}

	// duplicate solution
	Solution *next = new Solution(*this);

	// look for serial transistor stacks
	do {
		// insert new wires
		if (next->stack[type].size() == 0) {
			// add a contact for the first net
			next->contacts++;
			next->stack[type].push_back(Wire(from, Index(-next->contacts), Index(device, fromPort)));
		} else if (base->nets[from].ports.size() > 2) {
			// add a contact between the two transistors
			next->contacts++;
			next->stack[type].back().to = Index(-next->contacts);
			next->stack[type].push_back(Wire(from, Index(-next->contacts), Index(device, fromPort)));
		} else {
			next->stack[type].back().to = Index(device, fromPort);
			// no contact between
		}
		next->stack[type].push_back(Wire(to, Index(device, toPort), Index()));
	} while (...);

	// remove item from dangling
	next->dangling[type].erase(next->dangling[type].begin()+index);

	dst.push_back(next);
	return true;
}

// index into Solution::dangling
bool Solution::push(vector<Solution*> &dst, int type, int index) {
	int device = dangling[type][index];
	if (base->mos[device].ports.size() < 4) {
		printf("error parsing spice circuit, mos should have four ports\n")
		exit(1);
	}

	int fromPort = Mos::SOURCE;
	int toPort = Mos::DRAIN;
	int from = base->mos[device].ports[fromPort];
	int to = base->mos[device].ports[toPort];	

	// duplicate solution
	Solution *next = new Solution(*this);

	// insert new wires
	if (next->stack[type].size() == 0) {
		// add a contact for the first net
		// DESIGN(edward.bingham) There's no need to insert both the flipped and
		// unflipped version of the solution for the first transistor on the stack
		// because having the same transistor at the end of the stack already
		// implements the dual.
		next->contacts++;
		next->stack[type].push_back(Wire(from, Index(-next->contacts), Index(device, fromPort)));
	} else {
		next->stack[type].back().to = Index(device, fromPort);
		// no contact between
	}
	next->stack[type].push_back(Wire(to, Index(device, toPort), Index()));

	// remove item from dangling
	next->dangling[type].erase(next->dangling[type].begin()+index);

	dst.push_back(next);
	return true;
}


void Solution::build() {
}

void Solution::solve(const Tech &tech, int minCost) {
}

void draw(const Tech &tech) {
}
