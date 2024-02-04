#include "Ordering.h"

#include <list>
#include <set>
#include <unordered_set>
#include <vector>

Edge::Edge() {
	gate = -1;
	size = vec2i(0, 0);
}

Edge::Edge(int mos, int gate, vector<int> ports, vec2i size) {
	this->mos.push_back(mos);
	this->gate = gate;
	this->ports = ports;
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

Eulerian::Eulerian() {
}

Eulerian::~Eulerian() {
}

vector<vector<int> > Eulerian::findCycles() {
	vector<vector<int> > cycles;

	// index into Eulerian::edges
	unordered_set<int> seen;
	// index into Eulerian::edges
	unordered_set<int> staged;

	// index into Eulerian::edges, Eulerian::verts
	// pair<sequence of edges, last vert>
	list<pair<vector<int>, int> > tokens;
	tokens.push_back(pair<vector<int>, int>(vector<int>(1, 0), 0));
	staged.insert(0);
	while (not tokens.empty()) {
		// breadth first search from current starting point to identify
		// any connected cycles.
		while (not tokens.empty()) {
			printf("%d\r", (int)tokens.size());
			fflush(stdout);
			pair<vector<int>, int> curr = tokens.front();
			tokens.pop_front();
			int edge = curr.first.back();
			
			for (int i = 0; i < (int)edges[edge].ports.size(); i++) {
				int port = edges[edge].ports[i];
				if (port == curr.second) {
					continue;
				}

				for (int j = 0; j < (int)verts[port].ports.size(); j++) {
					auto pos = lower_bound(verts[port].ports[j].begin(), verts[port].ports[j].end(), edge);
					if (pos == verts[port].ports[j].end() or *pos != edge) {
						continue;
					}

					// our transistor is connected to this particular pairing
					for (int k = 0; k < (int)verts[port].ports[j].size(); k++) {
						int next = verts[port].ports[j][k];
						if (next == edge) {
							continue;
						}

						// Check to see if this next edge would create a loop
						auto loop = find(curr.first.begin(), curr.first.end(), next);
						if (loop != curr.first.end()) {
							cycles.push_back(curr.first);
							cycles.back().erase(cycles.back().begin(), cycles.back().begin()+(loop-curr.first.begin()));
							vector<int>::iterator minElem = min_element(cycles.back().begin(), cycles.back().end());
							rotate(cycles.back().begin(), minElem, cycles.back().end());
							if (find(cycles.begin(), (cycles.end()-1), cycles.back()) != (cycles.end()-1)) {
								cycles.pop_back();
							}
						} else if (seen.find(next) == seen.end()) {
							tokens.push_back(curr);
							tokens.back().first.push_back(next);
							tokens.back().second = port;
							staged.insert(next);
						}
					}
				}
			}
		}

		// The graph may not be fully connected, find the next
		// unexplored edge to start the next breadth-first-search
		seen.insert(staged.begin(), staged.end());
		staged.clear();
		for (int edge = 0; edge < (int)edges.size(); edge++) {
			if (seen.find(edge) == seen.end()) {
				tokens.push_back(pair<vector<int>, int>(vector<int>(1, edge), 0));
				staged.insert(edge);
				break;
			}
		}
	}

	return cycles;
}

void Eulerian::breakCycles(vector<vector<int> > cycles) {
	for (int i = 0; i < (int)cycles.size(); i++) {
		printf("cycle %d: {", i);
		for (int j = 0; j < (int)cycles[i].size(); j++) {
			printf("%d ", cycles[i][j]);
		}
		printf("}\n");
	}

	
}

vector<Sequence> Eulerian::buildSequences() {
	vector<Sequence> result;

	/*while (not edges.empty()) {
	}*/

	return result;
} 

void Eulerian::print(const Circuit *base) {
	printf("Vertices\n");
	for (int i = 0; i < (int)verts.size(); i++) {
		if (base != nullptr) {
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
		if (base != nullptr) {
			printf("edge %d: %s(%d)\t-\t%s(%d)\t-\t%s(%d) {", i, base->nets[edges[i].ports[0]].name.c_str(), edges[i].ports[0], base->nets[edges[i].gate].name.c_str(), edges[i].gate, base->nets[edges[i].ports[1]].name.c_str(), edges[i].ports[1]);
		} else {
			printf("edge %d: %d-%d-%d {", i, edges[i].ports[0], edges[i].gate, edges[i].ports[1]);
		}
		for (int j = 0; j < (int)edges[i].mos.size(); j++) {
			printf("%d ", edges[i].mos[j]);
		}
		printf("}\n");
	}
}

Ordering::Ordering() {
}

Ordering::~Ordering() {
}

void Ordering::build(const Circuit *base) {
	// Create a Eulerian graph for the nmos stack and for the pmos stack
	// Nets represent vertices in that graph while transistors represent edges.
	for (int i = 0; i < (int)base->mos.size(); i++) {
		int type = base->mos[i].type;
		vec2i size = base->mos[i].size;
		int source = base->mos[i].ports[Mos::SOURCE];
		int drain = base->mos[i].ports[Mos::DRAIN];
		int gate = base->mos[i].ports[Mos::GATE];
		int maxNet = max(max(source, drain), gate);
		if ((int)mos[type].verts.size() <= maxNet) {
			mos[type].verts.resize(maxNet+1);
		}
		vector<int> ports;
		ports.push_back(min(source, drain));
		ports.push_back(max(source, drain));

		// See if this is a folding of another transistor
		int s = 0;
		while (s < (int)mos[type].edges.size() and (
			mos[type].edges[s].gate != gate or mos[type].edges[s].ports != ports or
			mos[type].edges[s].size != size)) {
			s++;
		}

		// if not a folding, then add a new edge
		if (s >= (int)mos[type].edges.size()) {
			mos[type].edges.push_back(Edge(i, gate, ports, base->mos[i].size));
		} else {
			mos[type].edges[s].mos.push_back(i);
		}

		mos[type].verts[source].addPort(s);
		mos[type].verts[drain].addPort(s);
		mos[type].verts[gate].addGate(s);
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

void Ordering::matchSequencing() {
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
		for (auto v0 = mos[type].verts.begin(); v0 != mos[type].verts.end(); v0++) {
			if (v0->ports.size() == 0) {
				continue;
			}
			if (v0->ports[0].size() > 2) {
				// DESIGN(edward.bingham) This ensures that adding items to v0->ports won't invalidate the iterator p0
				v0->ports.reserve(((int)v0->ports[0].size()+1)/2+1);
			}

			// At this point, we're looking at a single net in one of the two stacks
			int v0i = v0-mos[type].verts.begin();
			auto p0 = v0->ports.begin();
			if ((int)p0->size() > 2) {
				// This net needs to be broken up, look in the other stack for a
				// sequence that can help inform us how that should be done
				for (auto v1 = mos[1-type].verts.begin(); v1 != mos[1-type].verts.end(); v1++) {
					for (auto p1 = v1->ports.begin(); p1 != v1->ports.end(); p1++) {
						if ((int)p1->size() == 2) {
							// We found a strict sequencing, this is a net that is only
							// connected to two other source or drain nodes in the stack.
							// Does it have anything to do with the ports in our net?
							vector<PortPairing> n0;
							for (auto port = p0->begin(); port != p0->end(); port++) {
								// See if any of the ports in our net match the first of the two nodes
								if (mos[type].edges[*port].gate == mos[1-type].edges[(*p1)[0]].gate) {
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
								if (mos[type].edges[*port].gate == mos[1-type].edges[(*p1)[1]].gate) {
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
	// Eulerian graph that wouldn't exist if we were to draw out a Eulerian
	// Cycle.
}

void Ordering::breakCycles() {
	// Look for cycles in the Eulerian graph and cut them optimizing gate
	// distance and reducing number of connections to the other stack. The result
	// of this will be a set of gate strips which we need to align. We can create
	// de-bruijn graphs following the DNA sequencing strategies.

	// DESIGN(edward.bingham) Nets which can be placed over the transistor
	// stacks because they have fewer connections to the other stack are more
	// amenable to being broken up. Often, this will likely be Vdd or GND

	for (int type = 0; type < 2; type++) {
		vector<vector<int> > cycles = mos[type].findCycles();
		mos[type].breakCycles(cycles);
	}
}

void Ordering::buildSequences() {
	// Identify all of the stacks from the graph by exploring all of the nodes
	// (hopefully in linear time). Then, we can free the memory associated with
	// the Eulerian Graphs. While there are nodes left in the Eulerian graphs,
	// pick a node and walk the graph in both directions. Save the resulting
	// stack and record whether there is a cycle. Delete the nodes from the graph
	// as we explore them
	array<vector<Sequence>, 2> seq;
	for (int type = 0; type < 2; type++) {
		seq[type] = mos[type].buildSequences();
	}
}

void Ordering::buildConstraints() {
	// Identify the offsets for maximum alignment and the alignment score between
	// any pairing of the nmos and pmos stacks.
}

void Ordering::solveConstraints() {
	// Run a greedy algorithm, choosing the pairing and offset with the best
	// score, then checking to see which pairings it eliminates from our choice
	// set. We do this until we have either selected or eliminated all of the
	// pairings.
}

void Ordering::fixDangling() {
	// Check for dangling transistors which haven't been matched up. We can
	// either leave them where they are (hurting horizontal size due to unmatched
	// transistors in both stacks) or cut them off their stack and reposition
	// them somewhere with a hole (hurting vertical size due to increased routing
	// complexity). Since combinational logic is always fully matched, and
	// c-elements are often fully matched, this outcome should hopefully not
	// occur too often.
}

void Ordering::solve(int radix) {
	matchSequencing();
	breakCycles();
	buildSequences();
	buildConstraints();
	solveConstraints();
	fixDangling();


	// Old Code
	// Create super nodes
	/*for (int i = 0; i < 2; i++) {
		seq[i].push_back(Edge());
		auto s = prev(seq[i].end());
		sup[i].push_back(s);
		for (int j = 0; j < (int)nets.size(); j++) {
			if (nets[j].portCount[i]&1) {
				seq[i].back().nets.push_back(j);
				nets[j].ports.push_back(s);
			}
		}		
	}*/

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

void Ordering::print(const Circuit *base) {
	printf("NMOS Stack\n");
	mos[0].print(base);
	printf("\nPMOS Stack\n");
	mos[1].print(base);
	printf("\n");
}

