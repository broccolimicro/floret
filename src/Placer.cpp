#include "Placer.h"
#include "Draw.h"
#include "Timer.h"

#include <list>
#include <set>
#include <unordered_set>
#include <vector>
#include <random>

Placement::Placement() {
	this->base = nullptr;
	b = 0;
	l = 0;
	w = 0;
}

Placement::Placement(const Circuit *base, int b, int l, int w, int g) {
	this->base = base;
	this->b = b;
	this->l = l;
	this->w = w;
	this->g = g;

	static std::default_random_engine generator(std::random_device{}());	static std::bernoulli_distribution distribution(0.5);
	for (int i = 0; i < (int)base->mos.size(); i++) {
		stack[base->mos[i].type].push_back(Device{i, distribution(generator)});
	}
	this->d[0] = max(0, (int)stack[0].size()-(int)stack[1].size());
	this->d[1] = max(0, (int)stack[1].size()-(int)stack[0].size());

	array<int, 2> D;
	for (int type = 0; type < 2; type++) {
		D[type] = -2;
		for (int i = 0; i < (int)base->nets.size(); i++) {
			D[type] += (base->nets[i].ports[type]&1);
		}
		D[type] >>= 1;
	}
	this->Wmin = max((int)stack[0].size()+D[0], (int)stack[1].size()+D[1]) - max((int)stack[0].size(), (int)stack[1].size());

	bool shorter = stack[1].size() < stack[0].size();
	stack[shorter].resize(stack[not shorter].size(), Device{-1,false});

	for (int type = 0; type < 2; type++) {
		random_shuffle(stack[type].begin(), stack[type].end());
	}
}

Placement::~Placement() {
}

void Placement::move(vec4i choice) {
	for (int i = choice[0]; i < choice[1]; i++) {
		for (int j = choice[2], k = choice[3]; j < k; j++, k--) {
			swap(stack[i][j], stack[i][k]);
			stack[i][j].flip = not stack[i][j].flip;
			stack[i][k].flip = not stack[i][k].flip;
		}
	}
}

int Placement::score() {
	int B = 0, W = 0, L = 0, G = 0;
	array<int, 2> brk = {0,0};

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

	for (int i = 0; i < (int)ext.size(); i++) {
		L += ext[i][1] - ext[i][0];
	}
	B = brk[0]+brk[1];
	W = min(brk[0]+d[0]-Wmin,brk[1]+d[1]-Wmin);

	for (int i = 0; i < (int)min(stack[0].size(), stack[1].size()); i++) {
		G += (stack[0][i].device >= 0 and stack[1][i].device >= 0 and base->mos[stack[0][i].device].gate == base->mos[stack[1][i].device].gate);
	}

	return b*B*B + l*L + w*W*W - g*G;
}

void Placement::solve(const Tech &tech, Circuit *base, int starts, int b, int l, int w, int g, float step, float rate) {
	if (base->mos.size() == 0) {
		return;
	}

	Placement best(base, b, l, w, g);
	float bestScore = (float)best.score();

	vector<vec4i> choices;
	for (int i = 0; i < 3; i++) {
		int end = (i == 2 ? min((int)best.stack[0].size(), (int)best.stack[1].size()) : (int)best.stack[i].size());
		for (int j = 0; j < end; j++) {
			for (int k = j+1; k < end; k++) {
				choices.push_back(vec4i(i == 2 ? 0 : i, i == 2 ? 2 : i+1, j, k));
			}
		}
	}

	for (int i = 0; i < starts; i++) {
		Placement curr(base, b, l, w, g);
		int score = 0;
		int newScore = curr.score();
		float currStep = step;
		int idx = 0;
		do {
			//printf("%d\r", idx);
			//fflush(stdout);
			score = newScore;
			for (auto choice = choices.begin(); choice != choices.end(); choice++) {
				curr.move(*choice);
				newScore = curr.score();
				if (newScore < score) {
					break;
				} else {
					curr.move(*choice);
				}
			}

			random_shuffle(choices.begin(), choices.end());
			currStep -= (currStep-1.0)*rate;
			idx++;
		} while (newScore < score*currStep);

		//printf("finished with score %d/%d with %d iterations\n", score, bestScore, idx);

		if (score < bestScore) {
			bestScore = score;
			best = curr;
		}
	}

	array<Stack, 2> result;
	for (int type = 0; type < 2; type++) {
		result[type].type = type;
		for (auto pin = best.stack[type].begin(); pin != best.stack[type].end(); pin++) {
			result[type].push(base, pin->device, pin->flip);
		}
		if (best.stack[type].back().device >= 0) {
			result[type].push(base, -1, false);
		}
	}
	base->stack = result;
}

