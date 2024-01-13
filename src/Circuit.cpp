#include "Circuit.h"
#include "Common.h"
#include "spice.h"
#include "Solution.h"

Transistor::Transistor() {
	model = -1;
}

Transistor::Transistor(int model) {
	this->model = model;
}

Transistor::~Transistor() {
}

Index::Index() {
	device = -1;
	port = 0;
}

Index::Index(int device, int port) {
	this->device = device;
	this->port = port;
}

Index::~Index() {
}

Net::Net() {
}

Net::Net(string name) {
	this->name = name;
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
		return false;
	}

	string devType = lexer.read(dev.tokens[0].begin, dev.tokens[0].end);
	string devName = lexer.read(dev.tokens[1].begin, dev.tokens[1].end);

	// DESIGN(edward.bingham) Since we're focused on digital design, we'll only support transistor layout for now.
	if (string("mMxX").find(devType) == string::npos) {
		return false;
	}

	auto args = dev.tokens.begin() + 2;
	// Transistors must have the following args
	// drain gate source base modelName
	if (args->tokens.size() < 5) {
		return false;
	}

	// modelName cannot be a number or assignment
	if (args->tokens[4].type != lang.NAME) {
		return false;
	}

	string modelName = lexer.read(args->tokens[4].begin, args->tokens[4].end);
	int modelIdx = tech.findModel(modelName);
	// if the modelName isn't in the model list, then this is a non-transistor subckt
	if (modelIdx < 0) {
		return false;
	}

	int index = (int)this->mos.size();
	this->mos.push_back(Transistor(modelIdx));

	int port = 0;
	for (auto arg = args->tokens.begin(); arg != args->tokens.end(); arg++) {
		if (arg->type == lang.PARAM) {
			string paramName = lower(lexer.read(arg->tokens[0].begin, arg->tokens[0].end));
			vector<double> values;
			for (auto value = arg->tokens.begin()+1; value != arg->tokens.end(); value++) {
				values.push_back(loadValue(lang, lexer, *value));
			}
			this->mos.back().params.insert(pair<string, vector<double> >(paramName, values));
		} else if (port < 4) {
			string netName = lexer.read(arg->begin, arg->end);
			this->nets[this->findNet(netName, true)].ports.push_back(Index(index, port++));
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
				this->nets.push_back(Net(lexer.read(port->begin, port->end)));
			}
		} else if (tok->type == lang.DEVICE) {
			if (not loadDevice(tech, lang, lexer, *tok)) {
				printf("unrecognized device\n");
			}
		}
	}
}

void Circuit::solve(const Tech &tech) {
	vector<Solution*> stack;
	stack.push_back(new Solution(this));
	
	// Depth first search through all useful orderings. These are
	// orderings that try to keep the transistor stacks fully
	// connected. Prioritise orderings that better align the nmos and
	// pmos contacts 
	while (stack.size() > 0) {
		Solution *curr = stack.back();
		stack.pop_back();
		
		
	}
}

