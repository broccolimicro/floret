#include "Circuit.h"
#include "Common.h"
#include "spice.h"
#include "Router.h"
#include "Timer.h"
#include "Placer.h"

Mos::Mos() {
	model = -1;
	type = -1;
	size = vec2i(0,0);
}

Mos::Mos(int model, int type) {
	this->model = model;
	this->type = type;
	this->size = vec2i(0,0);
}

Mos::~Mos() {
}

Net::Net() {
	ports = 0;
	isIO = false;
}

Net::Net(string name, bool isIO) {
	this->name = name;
	this->ports = 0;
	this->isIO = isIO;
}

Net::~Net() {
}

Circuit::Circuit() {
	layout = nullptr;
}

Circuit::~Circuit() {
	if (layout != nullptr) {
		delete layout;
		layout = nullptr;
	}
}

int Circuit::findNet(string name, bool create) {
	for (int i = 0; i < (int)nets.size(); i++) {
		if (nets[i].name == name) {
			return i;
		}
	}
	if (create) {
		int index = (int)nets.size();
		nets.push_back(Net(name));
		return index;
	}
	return -1;
}

bool Circuit::loadDevice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev) {
	// deviceType deviceName paramList
	if (dev.tokens.size() < 3) {
		printf("not a device\n");
		return false;
	}

	string devType = lower(lexer.read(dev.tokens[0].begin, dev.tokens[0].end));
	string devName = lexer.read(dev.tokens[1].begin, dev.tokens[1].end);

	// DESIGN(edward.bingham) Since we're focused on digital design, we'll only support transistor layout for now.
	if (string("mx").find(devType) == string::npos) {
		printf("not transistor or subckt %s%s\n", devType.c_str(), devName.c_str());
		return false;
	}

	auto args = dev.tokens.begin() + 2;
	// Moss must have the following args
	// drain gate source base modelName
	if (args->tokens.size() < 5) {
		printf("not enough args\n");
		return false;
	}

	// modelName cannot be a number or assignment
	if (args->tokens[4].type != lang.NAME) {
		printf("model name not present\n");
		return false;
	}

	string modelName = lexer.read(args->tokens[4].begin, args->tokens[4].end);
	int modelIdx = tech.findModel(modelName);
	// if the modelName isn't in the model list, then this is a non-transistor subckt
	if (modelIdx < 0) {
		printf("model not found %s\n", modelName.c_str());
		return false;
	}

	this->mos.push_back(Mos(modelIdx, tech.models[modelIdx].type));
	for (auto arg = args->tokens.begin(); arg != args->tokens.end(); arg++) {
		int port = (int)this->mos.back().ports.size();
		if (arg->type == lang.PARAM) {
			string paramName = lower(lexer.read(arg->tokens[0].begin, arg->tokens[0].end));
			vector<double> values;
			for (auto value = arg->tokens.begin()+1; value != arg->tokens.end(); value++) {
				values.push_back(loadValue(lang, lexer, *value));
			}
			if (paramName == "w") {
				this->mos.back().size[1] = int(values[0]/tech.dbunit);
			} else if (paramName == "l") {
				this->mos.back().size[0] = int(values[0]/tech.dbunit);
			} else {
				this->mos.back().params.insert(pair<string, vector<double> >(paramName, values));
			}
		} else if (port < 4) {
			string netName = lexer.read(arg->begin, arg->end);
			int net = this->findNet(netName, true);
			this->nets[net].ports++;
			this->mos.back().ports.push_back(net);
		}
	}

	return true;
}

void Circuit::loadSubckt(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt) {
	for (auto tok = subckt.tokens.begin(); tok != subckt.tokens.end(); tok++) {
		if (tok->type == lang.NAME) {
			this->name = lexer.read(tok->begin, tok->end);
		} else if (tok->type == lang.PORT_LIST) {
			for (auto port = tok->tokens.begin(); port != tok->tokens.end(); port++) {
				this->nets.push_back(Net(lexer.read(port->begin, port->end), true));
			}
		} else if (tok->type == lang.DEVICE) {
			if (not loadDevice(tech, lang, lexer, *tok)) {
				printf("unrecognized device\n");
			}
		}
	}
}

void Circuit::solve(const Tech &tech, float cycleCoeff) {
	Placer order;
	order.build(this);
	order.print(this);
	order.solve();
	order.print(this);

	vector<Router> stack;
	vector<array<Token, 2> > start = order.findStart();
	for (int i = 0; i < (int)start.size(); i++) {
		stack.push_back(Router(this));
		stack.back().curr = start[i];
	}

	int count = 0;
	Timer timer;

	// DESIGN(edward.bingham) Depth first search through all useful
	// orderings. These are orderings that try to keep the transistor
	// stacks fully connected. Prioritise orderings that better align
	// the nmos and pmos contacts. This will ensure that we test as few
	// orderings as possible while maximizing the chance of hitting the
	// global minimum for layout area.
	int maxAlignment = -1;
	stack.reserve(1000000);
	while (stack.size() > 0) {
		printf("\r%d %d      ", count, (int)stack.size());
		fflush(stdout);
		Router curr = stack.back();
		stack.pop_back();

		if (curr.dangling[Model::NMOS].size() == 0 and 
		    curr.dangling[Model::PMOS].size() == 0) {
			int alignment = curr.countAligned();
			if (alignment > maxAlignment) {
				if (layout != nullptr) {
					delete layout;
				}
				layout = new Router(curr);
				maxAlignment = alignment;
			}
			count++;
			continue;
		}

		// DESIGN(edward.bingham) The following search order makes sure
		// that we can't introduce redundant orderings of transistors:
		// NMOS linked, NMOS unlinked, PMOS linked, PMOS unlinked

		vector<Router> toadd;
		vector<Token> n;

		int type = 1;
		if (curr.stack[0].size() == 0 or (curr.stack[1].size() != 0 and curr.stack[0].back().pos < curr.stack[1].back().pos)) {
			type = 0;
		}

		bool found = false;
		n = order.stack[type].next(curr.curr[type]);
		for (int i = 0; i < (int)n.size(); i++) {
			curr.curr[type] = n[i];
			if (order.stack[type].edges[n[i].edge].mos.size() > 0) {
				auto pos = find(curr.dangling[type].begin(), curr.dangling[type].end(), order.stack[type].edges[n[i].edge].mos[0]);
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
		
		vector<Router> best;
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

		stack.insert(stack.end(), best.begin(), best.end());
		best.clear();
	}

	printf("\rCircuit::solve explored %d layouts for %s in %fms\n", count, name.c_str(), timer.since()*1e3);
}

