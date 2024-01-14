#include "Solution.h"

Wire::Wire() {
	net = -1;
}

Wire::Wire(int net, Index from, Index to) {
	this->net = net;
	this->from = from;
	this->to = to;
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
	int prevNet = stack[type].back().net;

	// This does two things:
	// 1. Determine if we can link this transistor to the previous one on the stack
	// 2. Determine whether we need to flip this transistor to do so
	int fromPort = Mos::SOURCE;
	int toPort = Mos::DRAIN;
	int fromNet = base->mos[device].ports[fromPort];
	int toNet = base->mos[device].ports[toPort];
	if (toNet == prevNet) {
		toNet = fromNet;
		fromNet = prevNet;
		fromPort = Mos::DRAIN;
		toPort = Mos::SOURCE;
	} else if (fromNet != prevNet) {
		return false;
	}

	// duplicate solution
	Solution *next = new Solution(*this);

	// TODO(edward.bingham) look for serial transistor stacks
	//do {
		// insert new wires
		if (next->stack[type].size() == 0) {
			// add a contact for the first net. This should never happen since it
			// should be handled by Solution::push().
			next->numContacts++;
			next->stack[type].push_back(Wire(fromNet, Index(-next->numContacts), Index(device, fromPort)));
		} else if (base->nets[fromNet].ports.size() > 2) {
			// add a contact between the two transistors
			next->numContacts++;
			next->stack[type].back().to = Index(-next->numContacts);
			next->stack[type].push_back(Wire(fromNet, Index(-next->numContacts), Index(device, fromPort)));
		} else {
			// no contact between
			next->stack[type].back().to = Index(device, fromPort);
		}

		next->stack[type].push_back(Wire(toNet, Index(device, toPort), Index()));

		// Check for a serial transistor stack
		//if (base->nets[toNet].ports) == 2) {
		//	Index dev0 = base->nets[toNet].ports[0].
		//}
	//} while (...);

	// remove item from dangling
	next->dangling[type].erase(next->dangling[type].begin()+index);
	if (next->dangling[type].size() == 0) {
		next->numContacts++;
		next->stack[type].back().to = Index(-next->numContacts);
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

	int fromPort = Mos::SOURCE;
	int toPort = Mos::DRAIN;
	int fromNet = base->mos[device].ports[fromPort];
	int toNet = base->mos[device].ports[toPort];	

	// We can't link this transistor to the previous one in the stack, so we
	// need to cap off the stack with a contact, start a new stack with a new
	// contact, then add this transistor. We need to test both the flipped and
	// unflipped orderings.

	// duplicate solution for the unflipped ordering
	Solution *next = new Solution(*this);

	if (next->stack[type].size() > 0) {
		next->numContacts++;
		next->stack[type].back().to = Index(-next->numContacts);
	}
	next->numContacts++;
	next->stack[type].push_back(Wire(fromNet, Index(-next->numContacts), Index(device, fromPort)));
	next->stack[type].push_back(Wire(toNet, Index(device, toPort), Index()));
	// remove item from dangling
	next->dangling[type].erase(next->dangling[type].begin()+index);
	if (next->dangling[type].size() == 0) {
		next->numContacts++;
		next->stack[type].back().to = Index(-next->numContacts);
	}
	dst.push_back(next);

	// duplicate solution for the flipped ordering
	next = new Solution(*this);

	if (next->stack[type].size() > 0) {
		next->numContacts++;
		next->stack[type].back().to = Index(-next->numContacts);
	}
	next->numContacts++;
	next->stack[type].push_back(Wire(toNet, Index(-next->numContacts), Index(device, toPort)));
	next->stack[type].push_back(Wire(fromNet, Index(device, fromPort), Index()));
	// remove item from dangling
	next->dangling[type].erase(next->dangling[type].begin()+index);
	if (next->dangling[type].size() == 0) {
		next->numContacts++;
		next->stack[type].back().to = Index(-next->numContacts);
	}
	dst.push_back(next);

	return true;
}

void Solution::build() {
	for (int type = 0; type < 2; type++) {
		vector<int> prevContact(base->nets.size(), -numContacts-1);
		for (int i = 0; i < (int)stack[type].size(); i++) {
			// if we happen upon a diffusion contact
			if (stack[type][i].from.device < 0) {
				int net = stack[type][i].net;
				int port = 0;
				if (prevContact[net] >= 0) {
					// the previous connection was the gate of a transistor
					port = Mos::GATE;
				}
				if (prevContact[net] >= -numContacts) {
					routes.push_back(Wire(net, Index(prevContact[net], port), Index(stack[type][i].from.device)));
				}	
				prevContact[net] = stack[type][i].from.device;
			}

			// if we happen upon a transistor
			if (stack[type][i].to.device >= 0) {
				int net = base->mos[stack[type][i].to.device].ports[Mos::GATE];
				int port = 0;
				if (prevContact[net] >= 0) {
					// the previous connection was the gate of a transistor
					port = Mos::GATE;
				}
				if (prevContact[net] >= -numContacts) {
					routes.push_back(Wire(net, Index(prevContact[net], port), Index(stack[type][i].to.device, Mos::GATE)));
				}
				prevContact[net] = stack[type][i].to.device;
			}
		}

		// handle the last diffusion contact in the stack
		if (stack[type].back().to.device < 0) {
			int net = stack[type].back().net;
			int port = 0;
			if (prevContact[net] >= 0) {
				// the previous connection was the gate of a transistor
				port = Mos::GATE;
			}
			if (prevContact[net] >= -numContacts) {
				routes.push_back(Wire(net, Index(prevContact[net], port), Index(stack[type].back().to.device)));
			}
			prevContact[net] = stack[type].back().to.device;
		}
	}

	/*printf("NMOS\n");
	for (int i = 0; i < (int)stack[0].size(); i++) {
		printf("wire %d from %d:%d to %d:%d\n", stack[0][i].net, stack[0][i].from.device, stack[0][i].from.port, stack[0][i].to.device, stack[0][i].to.port);
	}

	printf("\nPMOS\n");
	for (int i = 0; i < (int)stack[1].size(); i++) {
		printf("wire %d from %d:%d to %d:%d\n", stack[1][i].net, stack[1][i].from.device, stack[1][i].from.port, stack[1][i].to.device, stack[1][i].to.port);
	}

	printf("\nRoutes\n");
	for (int i = 0; i < (int)routes.size(); i++) {
		printf("wire %d from %d:%d to %d:%d\n", routes[i].net, routes[i].from.device, routes[i].from.port, routes[i].to.device, routes[i].to.port);
	}

	printf("\n\n");*/
	
}

void Solution::solve(const Tech &tech, int minCost) {

}

void draw(const Tech &tech) {
}
