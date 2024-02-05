#include "Circuit.h"
#include "Common.h"
#include "spice.h"
#include "Timer.h"
#include "Draw.h"

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
	
	layer = 0;
	width = 0;
	height = 0;
	off = 0;
	pos = 0;
}

Pin::Pin(int outNet) {
	this->device = -1;
	this->outNet = outNet;
	this->leftNet = outNet;
	this->rightNet = outNet;

	layer = 1;
	width = 0;
	height = 0;
	off = 0;
	pos = 0;
}

Pin::Pin(int device, int outNet, int leftNet, int rightNet) {
	this->device = device;
	this->outNet = outNet;
	this->leftNet = leftNet;
	this->rightNet = rightNet;

	layer = 0;
	width = 0;
	height = 0;
	off = 0;
	pos = 0;
}

Pin::~Pin() {
}

CompareIndex::CompareIndex(const Circuit *s) {
	this->s = s;
}

CompareIndex::~CompareIndex() {
}

bool CompareIndex::operator()(const Index &i0, const Index &i1) {
	return s->stack[i0.type].pins[i0.pin].pos < s->stack[i1.type].pins[i1.pin].pos or
		(s->stack[i0.type].pins[i0.pin].pos == s->stack[i1.type].pins[i1.pin].pos and (i0.type > i1.type or
		(i0.type == i1.type and i0.pin < i1.pin)));
}

Wire::Wire() {
	net = -1;
	left = -1;
	right = -1;
	pOffset = 0;
	nOffset = 0;
}

Wire::Wire(int net) {
	this->net = net;
	this->left = -1;
	this->right = -1;
	this->pOffset = 0;
	this->nOffset = 0;
}

Wire::~Wire() {
}

void Wire::addPin(const Circuit *s, Index pin) {
	auto pos = lower_bound(pins.begin(), pins.end(), pin, CompareIndex(s));
	pins.insert(pos, pin);
	if (left < 0 or s->stack[pin.type].pins[pin.pin].pos < left) {
		left = s->stack[pin.type].pins[pin.pin].pos;
	}
	if (right < 0 or s->stack[pin.type].pins[pin.pin].pos+s->stack[pin.type].pins[pin.pin].width > right) {
		right = s->stack[pin.type].pins[pin.pin].pos + s->stack[pin.type].pins[pin.pin].width;
	}
}

bool Wire::hasPin(const Circuit *s, Index pin, vector<Index>::iterator *out) {
	auto pos = lower_bound(pins.begin(), pins.end(), pin, CompareIndex(s));
	if (out != nullptr) {
		*out = pos;
	}
	return pos != pins.end() and pos->type == pin.type and pos->pin == pin.pin;
}

int Wire::getLevel(int i) const {
	if (level.size() == 0) {
		return 2;
	}

	if (i < 0) {
		return level[0];
	}

	if (i >= (int)level.size()) {
		return level.back();
	}

	return level[i];
}

Stack::Stack() {
}

Stack::~Stack() {
}

void Stack::draw(const Tech &tech, int type) {
	// Draw the stacks
	for (int i = 0; i < (int)pins.size(); i++) {
		drawLayout(layout, pins[i].pinLayout, vec2i(pins[i].pos, 0), vec2i(1, type == Model::NMOS ? -1 : 1));
	}
}

Circuit::Circuit() {
	cellHeight = 0;
}

Circuit::~Circuit() {
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

Pin &Circuit::pin(Index i) {
	return stack[i.type].pins[i.pin];
}

const Pin &Circuit::pin(Index i) const {
	return stack[i.type].pins[i.pin];
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

	vector<int> ports;
	this->mos.push_back(Mos(modelIdx, tech.models[modelIdx].type));
	for (auto arg = args->tokens.begin(); arg != args->tokens.end(); arg++) {
		int port = (int)ports.size();
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
			ports.push_back(net);
		}
	}
	this->mos.back().gate = ports[1];
	this->mos.back().bulk = ports[3];
	this->mos.back().ports.push_back(ports[0]);
	this->mos.back().ports.push_back(ports[2]);

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

void Circuit::draw(const Tech &tech, Layout &dst) {
	vec2i dir(1,1);
	dst.name = name;

	dst.nets.reserve(nets.size());
	for (int i = 0; i < (int)nets.size(); i++) {
		dst.nets.push_back(nets[i].name);
	}

	for (int type = 0; type < 2; type++) {
		drawLayout(dst, stack[type].layout, vec2i(0, (type == Model::NMOS)*cellHeight)*dir, dir);
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		drawRoute(tech, dst, this, routes[i], vec2i(0,0), dir);
	}

	for (int i = 0; i < (int)dst.layers.size(); i++) {
		if (tech.paint[dst.layers[i].draw].fill) {
			Rect box = dst.layers[i].bbox();
			dst.layers[i].clear();
			dst.layers[i].push(box, true);
		}
	}

	dst.merge();
}
