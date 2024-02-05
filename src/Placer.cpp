#include "Placer.h"
#include "Draw.h"
#include "Timer.h"

#include <list>
#include <set>
#include <unordered_set>
#include <vector>

Edge::Edge() {
	gate = -1;
	size = vec2i(0, 0);
}

Edge::Edge(int mos, int gate, vector<int> verts, vec2i size) {
	this->mos.push_back(mos);
	this->gate = gate;
	this->verts = verts;
	this->size = size;
}

Edge::~Edge() {
}

Vertex::Vertex() {
	ports.push_back(vector<int>());
}

Vertex::~Vertex() {
}

void Vertex::addPort(int e) {
	auto pos = lower_bound(ports[0].begin(), ports[0].end(), e);
	ports[0].insert(pos, e);
}

void Vertex::addGate(int e) {
	gates.push_back(e);
}

Token::Token() {
	edge = -1;
	port = -1;
}

Token::Token(int edge, int port) {
	this->edge = edge;
	this->port = port;
}

Token::~Token() {
}

Network::Network() {
}

Network::~Network() {
}

vector<Token> Network::next(Token t) {
	vector<Token> result;

	if (t.edge < 0) {
		return result;
		/*for (int i = 0; i < (int)edges.size(); i++) {
			result.push_back(Token(i, -1));
		}
		return result;*/
	}

	for (int i = 0; i < (int)edges[t.edge].verts.size(); i++) {
		int port = edges[t.edge].verts[i];
		if (port != t.port) {
			for (int j = 0; j < (int)verts[port].ports.size(); j++) {
				auto pos = lower_bound(verts[port].ports[j].begin(), verts[port].ports[j].end(), t.edge);
				if (pos != verts[port].ports[j].end() and *pos == t.edge) {
					for (int k = 0; k < (int)verts[port].ports[j].size(); k++) {
						int next = verts[port].ports[j][k];
						if (next != t.edge) {
							result.push_back(Token(next, port));
						}
					}
				}
			}
		}
	}
	return result;
}

void Network::buildSupernodes() {
	// Create super nodes
	int brk = -1;
	for (int i = 0; i < (int)verts.size(); i++) {
		if (verts[i].ports[0].size()&1) {
			if (brk < 0) {
				brk = (int)verts.size();
				verts.push_back(Vertex());
			}
			int edge = (int)edges.size();
			edges.push_back(Edge());
			verts[i].ports[0].push_back(edge);
			edges[edge].verts.push_back(i);
			verts[brk].ports[0].push_back(edge);
			edges[edge].verts.push_back(brk);
		}
	}
}

vector<vector<int> > Network::findCycles() {
	vector<vector<int> > cycles;

	// index into Network::edges
	unordered_set<int> seen;
	// index into Network::edges
	unordered_set<int> staged;

	// index into Network::edges, Network::verts
	// pair<sequence of edges, last vert>
	list<pair<vector<int>, Token> > tokens;
	tokens.push_back(pair<vector<int>, Token>(vector<int>(1, 0), Token(0,0)));
	staged.insert(0);
	while (not tokens.empty()) {
		// breadth first search from current starting point to identify
		// any connected cycles.
		while (not tokens.empty()) {
			printf("%d\r", (int)tokens.size());
			fflush(stdout);
			pair<vector<int>, Token> curr = tokens.front();
			tokens.pop_front();
		
			vector<Token> n = next(curr.second);
			for (int i = 0; i < (int)n.size(); i++) {

				// Check to see if this next edge would create a loop
				auto loop = find(curr.first.begin(), curr.first.end(), n[i].edge);
				if (loop != curr.first.end()) {
					cycles.push_back(curr.first);
					cycles.back().erase(cycles.back().begin(), cycles.back().begin()+(loop-curr.first.begin()));
					vector<int>::iterator minElem = min_element(cycles.back().begin(), cycles.back().end());
					rotate(cycles.back().begin(), minElem, cycles.back().end());
					if (find(cycles.begin(), (cycles.end()-1), cycles.back()) != (cycles.end()-1)) {
						cycles.pop_back();
					}
				} else if (seen.find(n[i].edge) == seen.end()) {
					tokens.push_back(curr);
					tokens.back().first.push_back(n[i].edge);
					tokens.back().second = n[i];
					staged.insert(n[i].edge);
				}
			}
		}

		// The graph may not be fully connected, find the next
		// unexplored edge to start the next breadth-first-search
		seen.insert(staged.begin(), staged.end());
		staged.clear();
		for (int edge = 0; edge < (int)edges.size(); edge++) {
			if (seen.find(edge) == seen.end()) {
				tokens.push_back(pair<vector<int>, Token>(vector<int>(1, edge), Token(edge, 0)));
				staged.insert(edge);
				break;
			}
		}
	}

	return cycles;
}

void Network::breakCycles(vector<vector<int> > cycles) {
	for (int i = 0; i < (int)cycles.size(); i++) {
		printf("cycle %d: {", i);
		for (int j = 0; j < (int)cycles[i].size(); j++) {
			printf("%d ", cycles[i][j]);
		}
		printf("}\n");
	}

	/*while (not cycles.empty()) {
		float maxScore = -1;
		int from = -1;
		int to = -1;
		for (int i = 0; i < (int)cycles.size(); i++) {
			
		}
	}*/
}

vector<Sequence> Network::buildSequences() {
	vector<Sequence> result;

	/*while (not edges.empty()) {
	}*/

	return result;
} 

void Network::print(const Circuit *base) {
	printf("Vertices\n");
	for (int i = 0; i < (int)verts.size(); i++) {
		if (base != nullptr and i < (int)base->nets.size()) {
			printf("vert %s(%d): gates={", base->nets[i].name.c_str(), i);
		} else {
			printf("vert %d: gates={", i);
		}
		for (int j = 0; j < (int)verts[i].gates.size(); j++) {
			printf("%d ", verts[i].gates[j]);
		}
		printf("} ports={");
		for (int j = 0; j < (int)verts[i].ports.size(); j++) {
			if (j != 0) {
				printf(", ");
			}
			printf("(");
			for (int k = 0; k < (int)verts[i].ports[j].size(); k++) {
				printf("%d ", verts[i].ports[j][k]);
			}
			printf(")");
		}
		printf("}\n");
	}

	printf("Edges\n");
	for (int i = 0; i < (int)edges.size(); i++) {
		if (edges[i].gate < 0) {
			printf("brk %d: %s(%d) - %d {", i, base->nets[edges[i].verts[0]].name.c_str(), edges[i].verts[0], edges[i].verts[1]);
		} else if (base != nullptr) {
			printf("edge %d: %s(%d)\t-\t%s(%d)\t-\t%s(%d) {", i, base->nets[edges[i].verts[0]].name.c_str(), edges[i].verts[0], base->nets[edges[i].gate].name.c_str(), edges[i].gate, base->nets[edges[i].verts[1]].name.c_str(), edges[i].verts[1]);
		} else {
			printf("edge %d: %d-%d-%d {", i, edges[i].verts[0], edges[i].gate, edges[i].verts[1]);
		}
		for (int j = 0; j < (int)edges[i].mos.size(); j++) {
			printf("%d ", edges[i].mos[j]);
		}
		printf("}\n");
	}
}

Placement::Placement() {
}

Placement::Placement(const Circuit *base) {
	this->base = base;
	dangling[0].reserve(base->mos.size());
	dangling[1].reserve(base->mos.size());
	for (int i = 0; i < (int)base->mos.size(); i++) {
		dangling[base->mos[i].type].push_back(i);
	}
}

Placement::~Placement() {
}

Pin &Placement::pin(Index i) {
	return stack[i.type].pins[i.pin];
}

const Pin &Placement::pin(Index i) const {
	return stack[i.type].pins[i.pin];
}

// index into Placement::dangling
bool Placement::tryLink(vector<Placement> &dst, int type, int index) {
	// Make sure that this transistor can be linked with the previous transistor
	// on the stack. We cannot link this transistor if there isn't another
	// transistor on the stack.
	if (stack[type].pins.size() == 0) {
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
	int prevNet = stack[type].pins.back().rightNet;

	// This does two things:
	// 1. Determine if we can link this transistor to the previous one on the stack
	// 2. Determine whether we need to flip this transistor to do so
	int fromNet = base->mos[device].ports[0];
	int toNet = base->mos[device].ports[1];
	int gateNet = base->mos[device].gate;
	if (toNet == prevNet) {
		toNet = fromNet;
		fromNet = prevNet;
	} else if (fromNet != prevNet) {
		return false;
	}

	// duplicate solution
	dst.push_back(Placement(*this));
	if (dst.back().stack[type].pins.size() == 0 or base->nets[fromNet].ports > 2 or base->nets[fromNet].isIO) {
		// Add a contact for the first net or between two transistors.
		dst.back().stack[type].pins.push_back(Pin(fromNet));
	}
	dst.back().stack[type].pins.push_back(Pin(device, gateNet, fromNet, toNet));

	// remove item from dangling
	dst.back().dangling[type].erase(dst.back().dangling[type].begin()+index);
	if (dst.back().dangling[type].size() == 0) {
		dst.back().stack[type].pins.push_back(Pin(toNet));
	}
	return true;
}

// index into Placement::dangling
bool Placement::push(vector<Placement> &dst, int type, int index) {
	// Sanity check to make sure this transistor actually has ports. This should
	// never fail as it should be guaranteed by the spice loader in Circuit.cpp
	int device = dangling[type][index];
	if (base->mos[device].ports.size() < 4) {
		printf("error parsing spice circuit, mos should have four ports\n");
		exit(1);
	}

	int fromNet = base->mos[device].ports[0];
	int toNet = base->mos[device].ports[1];
	int gateNet = base->mos[device].gate;

	// We can't link this transistor to the previous one in the stack, so we
	// need to cap off the stack with a contact, start a new stack with a new
	// contact, then add this transistor. We need to test both the flipped and
	// unflipped orderings.

	// duplicate solution for the unflipped ordering
	for (int i = 0; i < 2; i++) {
		dst.push_back(Placement(*this));
		if (dst.back().stack[type].pins.size() > 0) {
			dst.back().stack[type].pins.push_back(Pin(stack[type].pins.back().rightNet));
		}
		dst.back().stack[type].pins.push_back(Pin(fromNet));
		dst.back().stack[type].pins.push_back(Pin(device, gateNet, fromNet, toNet));
		// remove item from dangling
		dst.back().dangling[type].erase(dst.back().dangling[type].begin()+index);
		if (dst.back().dangling[type].size() == 0) {
			dst.back().stack[type].pins.push_back(Pin(toNet));
		}

		swap(fromNet, toNet);
	}

	return true;
}

void Placement::buildPins(const Tech &tech) {
	// Draw the pin contact and via
	for (int type = 0; type < 2; type++) {
		int pos = 0;
		for (int i = 0; i < (int)stack[type].pins.size(); i++) {
			stack[type].pins[i].width = pinWidth(tech, Index(type, i));
			stack[type].pins[i].height = pinHeight(Index(type, i));
			stack[type].pins[i].pinLayout.clear();
			drawPin(tech, stack[type].pins[i].pinLayout, base, stack[type], i);
			stack[type].pins[i].conLayout.clear();
			drawViaStack(tech, stack[type].pins[i].conLayout, stack[type].pins[i].outNet, stack[type].pins[i].layer, 2, vec2i(0,0), vec2i(0,0));
			//stack[type].pins[i].conLayout.push(tech.wires[stack[type].pins[i].layer], Rect(stack[type].pins[i].outNet, vec2i(0, 0), vec2i(stack[type].pins[i].width, 0)));
			
			stack[type].pins[i].off = 0;
			if (i > 0) {
				minOffset(&stack[type].pins[i].off, tech, 0, stack[type].pins[i-1].pinLayout.layers, 0, stack[type].pins[i].pinLayout.layers, 0, stack[type].pins[i-1].device >= 0 or stack[type].pins[i].device >= 0);
			}

			pos += stack[type].pins[i].off;
			stack[type].pins[i].pos = pos;
		}
	}
}

int Placement::countAligned() {
	int gateMatches = 0;
	vector<Pin>::iterator idx[2] = {stack[0].pins.begin(),stack[1].pins.begin()};
	int pos[2] = {0,0};
	while (idx[0] != stack[0].pins.end() and idx[1] != stack[1].pins.end()) {
		if (idx[0]->device < 0) {
			pos[0] += idx[0]->off;
			idx[0]++;
		} else if (idx[1]->device < 0) {
			pos[1] += idx[1]->off;
			idx[1]++;
		} else if (idx[0]->outNet == idx[1]->outNet and ((idx[0]->device < 0) == (idx[1]->device < 0))) {
			gateMatches++;
			pos[0] += idx[0]->off;
			pos[1] += idx[1]->off;
			idx[0]++;
			idx[1]++;
		} else if (pos[1]+idx[1]->off < pos[0]+idx[0]->off) {
			pos[1] += idx[1]->off;
			idx[1]++;
		} else {
			pos[0] += idx[0]->off;
			idx[0]++;
		}
	}

	int conMatches = 0;
	idx[0] = stack[0].pins.begin();
	idx[1] = stack[1].pins.begin();
	pos[0] = 0;
	pos[1] = 0;
	while (idx[0] != stack[0].pins.end() and idx[1] != stack[1].pins.end()) {
		if (idx[0]->device >= 0) {
			pos[0] += idx[0]->off;
			idx[0]++;
		} else if (idx[1]->device >= 0) {
			pos[1] += idx[1]->off;
			idx[1]++;
		} else if (idx[0]->outNet == idx[1]->outNet and ((idx[0]->device < 0) == (idx[1]->device < 0))) {
			conMatches++;
			pos[0] += idx[0]->off;
			pos[1] += idx[1]->off;
			idx[0]++;
			idx[1]++;
		} else if (pos[1]+idx[1]->off < pos[0]+idx[0]->off) {
			pos[1] += idx[1]->off;
			idx[1]++;
		} else {
			pos[0] += idx[0]->off;
			idx[0]++;
		}
	}

	return gateMatches*3 + conMatches*2;
}


int Placement::alignPins(int coeff) {
	int matches = 0;
	vector<Pin>::iterator idx[2] = {stack[0].pins.begin(),stack[1].pins.begin()};
	int pos[2] = {0,0};
	while (idx[0] != stack[0].pins.end() and idx[1] != stack[1].pins.end()) {
		int axis = 0;
		if (pos[1]+idx[1]->off < pos[0]+idx[0]->off) {
			axis = 1;
		}

		int p = pos[1-axis];
		int off = idx[axis]->off;
		for (auto other = idx[1-axis]; other != stack[1-axis].pins.end() and p + other->off - pos[axis] < coeff*idx[axis]->off; other++) {
			p += other->off;
			other->pos = p;
			if (other->outNet == idx[axis]->outNet and ((other->device < 0) == (idx[axis]->device < 0))) {
				off = p - pos[axis];
				//idx[1-axis] = other;
				//pos[1-axis] = p;
				matches++;
				break;
			}
		}

		pos[axis] += off;
		idx[axis]->pos = pos[axis];
		idx[axis]++;
	}

	for (int type = 0; type < 2; type++) {
		for (; idx[type] != stack[type].pins.end(); idx[type]++) {
			pos[type] += idx[type]->off;
			idx[type]->pos = pos[type];
		}
	}
	return matches;
}

void Placement::updatePinPos() {
	// Determine location of each pin
	for (int type = 0; type < 2; type++) {
		int pos = 0;
		for (int i = 0; i < (int)stack[type].pins.size(); i++) {
			pos += stack[type].pins[i].off;
			stack[type].pins[i].pos = pos;
		}
	}
}

// horizontal size of pin
int Placement::pinWidth(const Tech &tech, Index p) const {
	int device = pin(p).device;
	if (device >= 0) {
		// this pin is a transistor, use length of transistor
		return tech.paint[tech.wires[0].draw].minWidth;
		//return base->mos[device].size[0];
	}
	// this pin is a contact
	return tech.paint[tech.wires[1].draw].minWidth;
}

// vertical size of pin
int Placement::pinHeight(Index p) const {
	int device = pin(p).device;
	if (device >= 0) {
		// this pin is a transistor, use width of transistor
		return base->mos[device].size[1];
	}
	// this is a contact, height should be min of transistor widths on either side.
	int result = -1;
	if (p.pin > 0) {
		int leftDevice = stack[p.type].pins[p.pin-1].device;
		if (leftDevice >= 0 and (result < 0 or base->mos[leftDevice].size[1] < result)) {
			result = base->mos[leftDevice].size[1];
		}
	}
	if (p.pin+1 < (int)stack[p.type].pins.size()) {
		int rightDevice = stack[p.type].pins[p.pin+1].device;
		if (rightDevice >= 0 and (result < 0 or base->mos[rightDevice].size[1] < result)) {
			result = base->mos[rightDevice].size[1];
		}
	}
	if (result < 0) {
		return 0;
	}
	return result;
}


Placer::Placer() {
}

Placer::~Placer() {
}

void Placer::build(const Circuit *base) {
	// Create a Network graph for the nmos stack and for the pmos stack
	// Nets represent vertices in that graph while transistors represent edges.
	for (int i = 0; i < (int)base->mos.size(); i++) {
		int type = base->mos[i].type;
		vec2i size = base->mos[i].size;
		int source = base->mos[i].ports[0];
		int drain = base->mos[i].ports[1];
		int gate = base->mos[i].gate;
		int maxNet = max(max(source, drain), gate);
		if ((int)stack[type].verts.size() <= maxNet) {
			stack[type].verts.resize(maxNet+1);
		}
		vector<int> verts;
		verts.push_back(min(source, drain));
		verts.push_back(max(source, drain));

		// See if this is a folding of another transistor
		// int s = 0;
		// while (s < (int)stack[type].edges.size() and (
		// 	stack[type].edges[s].gate != gate or stack[type].edges[s].ports != ports or
		// 	stack[type].edges[s].size != size)) {
		// 	s++;
		// }

		int s = (int)stack[type].edges.size();
		// if not a folding, then add a new edge
		// if (s >= (int)stack[type].edges.size()) {
			stack[type].edges.push_back(Edge(i, gate, verts, base->mos[i].size));
		// } else {
		// 	mos[type].edges[s].mos.push_back(i);
		// }

		stack[type].verts[source].addPort(s);
		stack[type].verts[drain].addPort(s);
		stack[type].verts[gate].addGate(s);
	}

	int m = min((int)stack[0].verts.size(), (int)stack[1].verts.size());
	for (int type = 0; type < 2; type++) {
		for (int i = 0; i < m; i++) {
			stack[type].verts[i].score = (int)stack[type].verts[i].ports[0].size() - (int)stack[1-type].verts[i].ports[0].size();
		}
		for (int i = m; i < (int)stack[type].verts.size(); i++) {
			stack[type].verts[i].score = (int)stack[type].verts[i].ports[0].size();
		}
	}

	for (int type = 0; type < 2; type++) {
		stack[type].buildSupernodes();
	}
}

struct PortPairing {
	PortPairing() {
		vert = -1;
		ports[0] = -1;
		ports[1] = -1;
		count[0] = 0;
		count[1] = 0;
	}
	PortPairing(int vert) {
		this->vert = vert;
		this->ports[0] = -1;
		this->ports[1] = -1;
		this->count[0] = 0;
		this->count[1] = 0;
	}
	~PortPairing() {}

	int vert;
	array<int, 2> ports;
	array<int, 2> count;
};

bool operator<(PortPairing p0, PortPairing p1) {
	return min(p0.count[0], p0.count[1]) < min(p1.count[0], p1.count[1]);
}

vector<array<Token, 2> > Placer::findStart() {
	vector<array<Token, 2> > result;
	for (int i = 0; i < (int)stack[0].edges.size(); i++) {
		for (int j = 0; j < (int)stack[1].edges.size(); j++) {
			//if (stack[0].edges[i].gate == stack[1].edges[j].gate) {
				for (int k = 0; k < (int)stack[0].edges[i].verts.size(); k++) {
					for (int l = 0; l < (int)stack[1].edges[j].verts.size(); l++) {
						if (stack[0].edges[i].verts[k] == stack[1].edges[j].verts[l]) {
							result.push_back(array<Token, 2>({Token(i, stack[0].edges[i].verts[k]), Token(j, stack[1].edges[j].verts[l])}));
						}
					}
				}
			//}
		}
	}

	for (int i = 0; i < (int)result.size(); i++) {
		printf("start (%d %d) (%d %d)\n", result[i][0].edge, result[i][0].port, result[i][1].edge, result[i][1].port);
	}
	return result;
}

void Placer::matchSequencing() {
	// DESIGN(edward.bingham) Any time there is a net with more than 2 ports,
	// it's because of some parallel composition. That parallel composition
	// *must* either be standalone or paired with some sequential composition in
	// the other stack. If the other pair were also parallel, then it would
	// create interference.

	// So, our job is to loop through all of the nets with more than two ports,
	// then look for the associated sequential composition and choose the pairs
	// for that net which maximizes alignment with those sequences. This will
	// reduce the choices we have to just the ordering of the diffusion breaks.
	for (int type = 0; type < 2; type++) {
		for (auto v0 = stack[type].verts.begin(); v0 != stack[type].verts.end(); v0++) {
			if (v0->ports.size() == 0) {
				continue;
			}
			if (v0->ports[0].size() > 2) {
				// DESIGN(edward.bingham) This ensures that adding items to v0->ports won't invalidate the iterator p0
				v0->ports.reserve(((int)v0->ports[0].size()+1)/2+1);
			}

			// At this point, we're looking at a single net in one of the two stacks
			int v0i = v0-stack[type].verts.begin();
			auto p0 = v0->ports.begin();
			if ((int)p0->size() > 2) {
				// This net needs to be broken up, look in the other stack for a
				// sequence that can help inform us how that should be done
				for (auto v1 = stack[1-type].verts.begin(); v1 != stack[1-type].verts.end(); v1++) {
					for (auto p1 = v1->ports.begin(); p1 != v1->ports.end(); p1++) {
						if ((int)p1->size() == 2) {
							// We found a strict sequencing, this is a net that is only
							// connected to two other source or drain nodes in the stack.
							// Does it have anything to do with the ports in our net?
							vector<PortPairing> n0;
							for (auto port = p0->begin(); port != p0->end(); port++) {
								// See if any of the ports in our net match the first of the two nodes
								if (stack[type].edges[*port].gate == stack[1-type].edges[(*p1)[0]].gate) {
									int k = 0;
									while (k < (int)n0.size() and n0[k].ports[0] != *port) {
										k++;
									}
									if (k < (int)n0.size()) {
										n0[k].count[0]++;
									} else {
										n0.push_back(PortPairing(v0i));
										n0.back().ports[0] = *port;
										n0.back().count[0]++;
									}
								}
							}

							vector<PortPairing> n1;
							for (auto port = p0->begin(); port != p0->end(); port++) {
								// See if any of the ports in our net match the second of the two nodes
								if (stack[type].edges[*port].gate == stack[1-type].edges[(*p1)[1]].gate) {
									for (int k = 0; k < (int)n0.size(); k++) {
										if (n0[k].ports[0] != *port) {
											int l = 0;
											while (l < (int)n1.size() and n1[l].ports[1] != *port and n1[l].ports[0] != *port and n1[l].ports[1] != n0[k].ports[0]) {
												l++;
											}
											if (l >= (int)n1.size()) {
												n1.push_back(n0[k]);
												n1.back().ports[1] = *port;
												if (n1.back().ports[1] < n1.back().ports[0]) {
													swap(n1.back().ports[0], n1.back().ports[1]);
													swap(n1.back().count[0], n1.back().count[1]);
												}
											}
											n1[l].count[(n1[l].ports[1] == *port)]++;
										}
									}
								}
							}

							// If N ports in our net match the first node and M ports in our
							// net match the second, then we have N*M possible pairings.
							// We've already elaborated these possible pairings into n1. So,
							// we need to figure out which set of pairings is optimal. Make a
							// heap, greedily pick and apply pairings
							make_heap(n1.begin(), n1.end());
							while (n1.size() != 0) {
								pop_heap(n1.begin(), n1.end());
								PortPairing top = n1.back();
								n1.pop_back();
								
								auto pos0 = lower_bound(p0->begin(), p0->end(), top.ports[0]);
								if (pos0 == p0->end() or *pos0 != top.ports[0]) {
									continue;
								}
								auto pos1 = lower_bound(p0->begin(), p0->end(), top.ports[1]);
								if (pos1 == p0->end() or *pos1 != top.ports[1]) {
									continue;
								}

								pos1 = p0->erase(pos1);
								pos0 = p0->erase(pos0);
								v0->ports.push_back(vector<int>(begin(top.ports), end(top.ports)));

								for (int i = n1.size()-1; i >= 0; i--) {
									if (((n1[i].ports[0] == top.ports[0] or n1[i].ports[0] == top.ports[1]) and
									     (pos0 == p0->end() or *pos0 != top.ports[0])) or
									    ((n1[i].ports[1] == top.ports[0] or n1[i].ports[1] == top.ports[1]) and
									     (pos1 == p0->end() or *pos1 != top.ports[1]))) {
										n1.erase(n1.begin() + i);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// At this point we know two things. First, the above heuristic *does not*
	// minimize the number of diffusion breaks. It can create loops in the
	// Network graph that wouldn't exist if we were to draw out a Network
	// Cycle.
}

void Placer::breakCycles() {
	// Look for cycles in the Network graph and cut them optimizing gate
	// distance and reducing number of connections to the other stack. The result
	// of this will be a set of gate strips which we need to align. We can create
	// de-bruijn graphs following the DNA sequencing strategies.

	// DESIGN(edward.bingham) Nets which can be placed over the transistor
	// stacks because they have fewer connections to the other stack are more
	// amenable to being broken up. Often, this will likely be Vdd or GND

	for (int type = 0; type < 2; type++) {
		vector<vector<int> > cycles = stack[type].findCycles();
		stack[type].breakCycles(cycles);
	}
}

void Placer::buildSequences() {
	// Identify all of the stacks from the graph by exploring all of the nodes
	// (hopefully in linear time). Then, we can free the memory associated with
	// the Network Graphs. While there are nodes left in the Network graphs,
	// pick a node and walk the graph in both directions. Save the resulting
	// stack and record whether there is a cycle. Delete the nodes from the graph
	// as we explore them
	array<vector<Sequence>, 2> seq;
	for (int type = 0; type < 2; type++) {
		seq[type] = stack[type].buildSequences();
	}
}

void Placer::buildConstraints() {
	// Identify the offsets for maximum alignment and the alignment score between
	// any pairing of the nmos and pmos stacks.
}

void Placer::solveConstraints() {
	// Run a greedy algorithm, choosing the pairing and offset with the best
	// score, then checking to see which pairings it eliminates from our choice
	// set. We do this until we have either selected or eliminated all of the
	// pairings.
}

void Placer::fixDangling() {
	// Check for dangling transistors which haven't been matched up. We can
	// either leave them where they are (hurting horizontal size due to unmatched
	// transistors in both stacks) or cut them off their stack and reposition
	// them somewhere with a hole (hurting vertical size due to increased routing
	// complexity). Since combinational logic is always fully matched, and
	// c-elements are often fully matched, this outcome should hopefully not
	// occur too often.
}

void Placer::searchOrderings(const Tech &tech) {
	vector<Placement> orders;
	vector<array<Token, 2> > start = findStart();
	for (int i = 0; i < (int)start.size(); i++) {
		orders.push_back(Placement(base));
		orders.back().curr = start[i];
	}

	int count = 0;
	Timer timer;

	int maxAlignment = -1;
	Placement result;

	// DESIGN(edward.bingham) Depth first search through all useful
	// orderings. These are orderings that try to keep the transistor
	// stacks fully connected. Prioritise orderings that better align
	// the nmos and pmos contacts. This will ensure that we test as few
	// orderings as possible while maximizing the chance of hitting the
	// global minimum for layout area.
	orders.reserve(1000000);
	while (not orders.empty()) {
		printf("\r%d %d      ", count, (int)orders.size());
		fflush(stdout);
		Placement curr = orders.back();
		orders.pop_back();

		if (curr.dangling[Model::NMOS].size() == 0 and 
		    curr.dangling[Model::PMOS].size() == 0) {
			int alignment = curr.countAligned();
			if (alignment > maxAlignment) {
				result = curr;
				maxAlignment = alignment;
			}
			count++;
			continue;
		}

		// DESIGN(edward.bingham) The following search order makes sure
		// that we can't introduce redundant orderings of transistors:
		// NMOS linked, NMOS unlinked, PMOS linked, PMOS unlinked

		vector<Placement> toadd;
		vector<Token> n;

		int type = 1;
		if (curr.stack[0].pins.empty() or (not curr.stack[1].pins.empty() and curr.stack[0].pins.back().pos < curr.stack[1].pins.back().pos)) {
			type = 0;
		}

		bool found = false;
		n = stack[type].next(curr.curr[type]);
		for (int i = 0; i < (int)n.size(); i++) {
			curr.curr[type] = n[i];
			if (stack[type].edges[n[i].edge].mos.size() > 0) {
				auto pos = find(curr.dangling[type].begin(), curr.dangling[type].end(), stack[type].edges[n[i].edge].mos[0]);
				if (pos != curr.dangling[type].end()) {
					int d = pos-curr.dangling[type].begin(); 
					bool test = curr.tryLink(toadd, type, d) or
					            curr.push(toadd, type, d);
						found = found or test;
				}
			} else {
				toadd.push_back(curr);
			}
		}

		if (n.size() == 0) {
			for (int type = 0; type < 2 and not found; type++) {
				for (int link = 1; link >= 0 and not found; link--) {
					for (int i = 0; i < (int)curr.dangling[type].size(); i++) {
						bool test = (link ?
							curr.tryLink(toadd, type, i) :
							curr.push(toadd, type, i));
						found = found or test;
					}
				}
			}	
		}
		
		vector<Placement> best;
		int bestCost = -1;
		while (not toadd.empty()) {
			int cost = toadd.back().countAligned();
			if (bestCost < 0 or cost > bestCost) {
				bestCost = cost;
				best.clear();
			}

			if (cost == bestCost) {
				best.push_back(toadd.back());
			}

			toadd.pop_back();
		}

		orders.insert(orders.end(), best.begin(), best.end());
		best.clear();
	}

	printf("\rCircuit::solve explored %d layouts for %s in %fms\n", count, base->name.c_str(), timer.since()*1e3);
}

void Placer::solve(const Tech &tech) {
	matchSequencing();
	//breakCycles();
	//buildSequences();
	//buildConstraints();
	//solveConstraints();
	//fixDangling();
	searchOrderings(tech);
	
	// Old Code

	/*vector<int> dev;
	dev.reserve(radix);
	for (int i = 0; i < 2; i++) {
		// generate combinations
		for (auto j = choose(mos[i], radix); not j.done(); j.next()) {
			dev.clear();
			for (int k = 0; k < j.size() k++) {
				dev.push_back(*(j[k]));
			}
			// loop through orderings to generate non-duplicating permutations
			do {
				// only add this sequence if all of the source and drain
				// nodes are shared for this ordering
				bool valid = true;
				for (int k = 0; k < (int)dev.size(); k++) {
					if (base->mos[dev[k]].
					printf("%d ", dev[k]);
				}
				printf("\n");
			} while (next_permutation(dev.begin(), dev.end()));
		}
	}*/
}

void Placer::print() {
	printf("NMOS Network\n");
	stack[Model::NMOS].print(base);
	printf("\nPMOS Network\n");
	stack[Model::PMOS].print(base);
	printf("\n");
}


