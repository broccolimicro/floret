#include "Ordering.h"

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
		if (s == (int)mos[type].edges.size()) {
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

void Ordering::solve(int radix) {
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

			int v0i = v0-mos[type].verts.begin();
			auto p0 = v0->ports.begin();
			if ((int)p0->size() > 2) {
				for (auto v1 = mos[1-type].verts.begin(); v1 != mos[1-type].verts.end(); v1++) {
					for (auto p1 = v1->ports.begin(); p1 != v1->ports.end(); p1++) {
						if ((int)p1->size() == 2) {
							vector<PortPairing> n0;
							for (auto port = p0->begin(); port != p0->end(); port++) {
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

							// Make a heap
							// greedily pick and apply pairings
							// arbitrarily determine remaining pairings
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

	// Our next task is to identify all of the stacks from the graph. This
	// involves exploring all of the nodes (hopefully in linear time) and then
	// recording them in stacks. At that point, we can free the Eulerian Graphs.
	// While there are nodes left in the Eulerian graphs, pick a node and walk
	// the graph in both directions. Save the resulting stack and record whether
	// there is a cycle. Delete the nodes from the graph as we explore them

	

	// Then, we need to to identify the offsets for maximum alignment and the
	// alignment score between any pairing of the nmos and pmos stacks.


	// Then we run a greedy algorithm, choosing the pairing and offset with the
	// best score, then checking to see which pairings it eliminates from our
	// choice set. We do this until we have either selected or eliminated all of
	// the pairings.


	// Then we need to check for dangling transistors which haven't been matched
	// up. We can either leave them where they are (hurting horizontal size due
	// to unmatched transistors in both stacks) or cut them off their stack and
	// reposition them somewhere with a hole (hurting vertical size due to
	// increased routing complexity). Since combinational logic is always fully
	// matched, and c-elements are often fully matched, this outcome should
	// hopefully not occur too often.





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
