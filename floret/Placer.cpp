#include "Placer.h"
#include "Draw.h"
#include "Timer.h"

#include <list>
#include <set>
#include <unordered_set>
#include <vector>

Placement::Placement() {
	this->base = nullptr;
	b = 0;
	l = 0;
	w = 0;
}

Placement::Placement(const Circuit *base, int b, int l, int w, int g, std::default_random_engine &rand) {
	this->base = base;
	this->b = b;
	this->l = l;
	this->w = w;
	this->g = g;

	// fill stacks with devices that have random orientiations
	static std::bernoulli_distribution distribution(0.5);
	for (int i = 0; i < (int)base->mos.size(); i++) {
		stack[base->mos[i].type].push_back(Device{i, distribution(rand)});
	}
	// cache stack size differences
	this->d[0] = max(0, (int)stack[0].size()-(int)stack[1].size());
	this->d[1] = max(0, (int)stack[1].size()-(int)stack[0].size());

	// compute Wmin
	array<int, 2> D;
	for (int type = 0; type < 2; type++) {
		D[type] = -2;
		for (int i = 0; i < (int)base->nets.size(); i++) {
			D[type] += (base->nets[i].ports[type]&1);
		}
		D[type] >>= 1;
	}
	this->Wmin = max((int)stack[0].size()+D[0], (int)stack[1].size()+D[1]) - max((int)stack[0].size(), (int)stack[1].size());

	// add dummy transistors
	bool shorter = stack[1].size() < stack[0].size();
	stack[shorter].resize(stack[not shorter].size(), Device{-1,false});

	// generate a random initial placement
	for (int type = 0; type < 2; type++) {
		shuffle(stack[type].begin(), stack[type].end(), rand);
	}
}

Placement::~Placement() {
}

void Placement::move(vec4i choice) {
	for (int i = choice[0]; i < choice[1]; i++) {
		int j = choice[2], k = choice[3];
		for (; j < k; j++, k--) {
			swap(stack[i][j], stack[i][k]);
			stack[i][j].flip = not stack[i][j].flip;
			stack[i][k].flip = not stack[i][k].flip;
		}
		if (j == k) {
			stack[i][j].flip = not stack[i][j].flip;
		}
	}
}

// Compute the cost of this placement using the cost function documented in floret/floret/Placer.h
int Placement::score() {
	int B = 0, W = 0, L = 0, G = 0;
	array<int, 2> brk = {0,0};

	// compute intermediate values
	// brk[] counts the number of diffusion breaks in this placement for each stack
	// ext[] finds the first and last index of each net
	vector<vec2i> ext(base->nets.size(), vec2i(((int)stack[0].size()+1)*2, -1));
	for (int type = 0; type < 2; type++) {
		for (auto c = stack[type].begin(); c != stack[type].end(); c++) {
			brk[type] += (c+1 != stack[type].end() and c->device >= 0 and (c+1)->device >= 0 and base->mos[c->device].ports[not c->flip] != base->mos[(c+1)->device].ports[(c+1)->flip]);

			if (c->device >= 0) {
				int gate = base->mos[c->device].gate;
				int off = (c-stack[type].begin())<<1;

				ext[gate][0] = min(ext[gate][0], off+1);
				ext[gate][1] = max(ext[gate][1], off+1);
				for (auto port = base->mos[c->device].ports.begin(); port != base->mos[c->device].ports.end(); port++) {
					int i = port-base->mos[c->device].ports.begin();
					ext[*port][0] = min(ext[*port][0], off+2*((int)c->flip==i));
					ext[*port][1] = max(ext[*port][1], off+2*((int)c->flip==i));
				}
			}
		}
	}

	// compute B, L, and W
	for (int i = 0; i < (int)ext.size(); i++) {
		L += ext[i][1] - ext[i][0];
	}
	B = brk[0]+brk[1];
	W = min(brk[0]+d[0]-Wmin,brk[1]+d[1]-Wmin);

	// compute G, keeping track of diffusion breaks
	for (auto i = stack[0].begin(), j = stack[1].begin(); i != stack[0].end() and j != stack[1].end(); i++, j++) {
		bool ibrk = (i != stack[0].begin() and (i-1)->device >= 0 and i->device >= 0 and base->mos[(i-1)->device].ports[not (i-1)->flip] != base->mos[i->device].ports[i->flip]);
		bool jbrk = (j != stack[1].begin() and (j-1)->device >= 0 and j->device >= 0 and base->mos[(j-1)->device].ports[not (j-1)->flip] != base->mos[j->device].ports[j->flip]);

		if (ibrk and not jbrk) {
			j++;
		} else if (jbrk and not ibrk) {
			i++;
		}
		
		if (i == stack[0].end() or j == stack[1].end()) {
			break;
		}

		G += (i->device >= 0 and j->device >= 0 and base->mos[i->device].gate != base->mos[j->device].gate);
	}

	return b*B*B + l*L + w*W*W + g*G;
}

void Placement::solve(const Tech &tech, Circuit *base, int starts, int b, int l, int w, int g, float step, float rate) {
	if (base->mos.size() == 0) {
		return;
	}
	std::default_random_engine rand(0/*std::random_device{}()*/);

	// TODO(edward.bingham) It might speed things up to scale the number of
	// starts based upon the cell complexity. Though, given that it would really
	// only reduce computation time for smaller cells and the larger cells
	// account for the majority of the computation time, I'm not sure that this
	// would really do that much to help.
	//starts = 50*(int)base->mos.size();

	Placement best(base, b, l, w, g, rand);
	int bestScore = best.score();

	// Precache the list of all possible moves. These will get reshuffled each time.
	vector<vec4i> choices;
	for (int i = 0; i < 3; i++) {
		int end = (i == 2 ? min((int)best.stack[0].size(), (int)best.stack[1].size()) : (int)best.stack[i].size());
		for (int j = 0; j < end; j++) {
			for (int k = j+1; k < end; k++) {
				choices.push_back(vec4i(i == 2 ? 0 : i, i == 2 ? 2 : i+1, j, k));
			}
		}
	}

	// Check multiple possible initial placements to avoid local minima
	for (int i = 0; i < starts; i++) {
		printf("start %d/%d\r", i, starts);
		fflush(stdout);

		// Run simulated annealing to find closest minimum
		Placement curr(base, b, l, w, g, rand);
		int score = 0;
		int newScore = curr.score();
		float currStep = step;
		do {
			// Test all of the possible moves and pick the best one.
			score = newScore;
			for (auto choice = choices.begin(); choice != choices.end(); choice++) {
				curr.move(*choice);
				
				// Check if this move makes any improvement within the constraints of
				// the annealing temperature
				newScore = curr.score();
				if (newScore < score*currStep) {
					break;
				} else {
					// undo the previous move
					curr.move(*choice);
				}
			}

			// Reshuffle the list of possible moves
			shuffle(choices.begin(), choices.end(), rand);

			// cool the annealing temperature
			currStep -= (currStep - 1.0)*rate;
			//printf("%f %f %d<%d\n", currStep, rate, newScore, score);
		} while ((float)score*currStep - (float)newScore > 0.01);

		if (score < bestScore) {
			bestScore = score;
			best = curr;
		}
	}
	printf("Placement complete after %d iterations\n", starts);

	// Save the resulting placement to the Circuit
	array<Stack, 2> result;
	for (int type = 0; type < 2; type++) {
		result[type].type = type;
		for (auto pin = best.stack[type].begin(); pin != best.stack[type].end(); pin++) {
			result[type].push(base, pin->device, pin->flip);
		}
		if (best.stack[type].back().device >= 0) {
			result[type].push(base, -1, false);
		}
		base->stack[type] = result[type];
	}
}

