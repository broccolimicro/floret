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
	ports[0] = 0;
	ports[1] = 0;
	gates[0] = 0;
	gates[1] = 0;
	isIO = false;
}

Net::Net(string name, bool isIO) {
	this->name = name;
	this->ports[0] = 0;
	this->ports[1] = 0;
	this->gates[0] = 0;
	this->gates[1] = 0;
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
	
	alignPin = -1;
	alignOff = 0;

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

	alignPin = -1;
	alignOff = 0;

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

	alignPin = -1;
	alignOff = 0;

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

bool Wire::hasPrev(int r) const {
	return prevNodes.find(r) != prevNodes.end();
}

Stack::Stack() {
	type = -1;
}

Stack::Stack(int type) {
	this->type = type;
}

Stack::~Stack() {
}

// index into Placement::dangling
void Stack::push(const Circuit *ckt, int device, bool flip) {
	int fromNet = -1;
	int toNet = -1;
	int gateNet = -1;

	if (device >= 0) {
		fromNet = ckt->mos[device].ports[flip];
		toNet = ckt->mos[device].ports[not flip];
		gateNet = ckt->mos[device].gate;
	}

	// Get information about the previous transistor on the stack. First if
	// statement in the funtion guarantees that there is at least one transistor
	// already on the stack.
	bool link = (pins.size() > 0 and gateNet >= 0 and fromNet == pins.back().rightNet);

	// We can't link this transistor to the previous one in the stack, so we
	// need to cap off the stack with a contact, start a new stack with a new
	// contact, then add this transistor. We need to test both the flipped and
	// unflipped orderings.

	if (not link and not pins.empty()) {
		pins.push_back(Pin(pins.back().rightNet));
	}

	bool hasContact = (ckt->nets[fromNet].ports[type] > 2 or ckt->nets[fromNet].ports[1-type] > 0 or ckt->nets[fromNet].gates[0] > 0 or ckt->nets[fromNet].gates[1] > 0);
	if (fromNet >= 0 and (not link or pins.empty() or hasContact or ckt->nets[fromNet].isIO)) {
		// Add a contact for the first net or between two transistors.
		pins.push_back(Pin(fromNet));
	}

	if (device >= 0) {
		pins.push_back(Pin(device, gateNet, fromNet, toNet));
	}
}

void Stack::draw(const Tech &tech) {
	layout.clear();
	// Draw the stacks
	for (int i = 0; i < (int)pins.size(); i++) {
		drawLayout(layout, pins[i].pinLayout, vec2i(pins[i].pos, 0), vec2i(1, type == Model::NMOS ? -1 : 1));
	}
}

Circuit::Circuit() {
	cellHeight = 0;
	for (int type = 0; type < 3; type++) {
		stack[type].type = type;
	}
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

	int port = 0;
	int type = tech.models[modelIdx].type;
	this->mos.push_back(Mos(modelIdx, type));
	for (auto arg = args->tokens.begin(); arg != args->tokens.end(); arg++) {
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
			if (port == 1) {
				this->nets[net].gates[type]++;
				this->mos.back().gate = net;
			} else if (port == 3) {
				this->mos.back().bulk = net;
			} else {
				this->nets[net].ports[type]++;
				this->mos.back().ports.push_back(net);
			}
			port++;
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

void Circuit::buildPins(const Tech &tech) {
	// Draw the pin contact and via
	for (int type = 0; type < 2; type++) {
		int pos = 0;
		for (int i = 0; i < (int)stack[type].pins.size(); i++) {
			stack[type].pins[i].width = pinWidth(tech, Index(type, i));
			stack[type].pins[i].height = pinHeight(Index(type, i));
			stack[type].pins[i].pinLayout.clear();
			drawPin(tech, stack[type].pins[i].pinLayout, this, stack[type], i);
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

void Circuit::updatePinPos(int p, int n) {
	bool done = false;
	do {
		done = true;
		for (int type = 0; type < 2; type++) {
			for (int i = (type == Model::PMOS ? p : n); i < (int)stack[type].pins.size(); i++) {
				Pin &curr = stack[type].pins[i];
				Pin &prev = stack[type].pins[i-1];

				int pos = max(curr.pos, prev.pos + curr.off);
				if (curr.alignPin >= 0) {
					pos = max(pos, stack[1-type].pins[curr.alignPin].pos);
				}
				if (pos != curr.pos) {
					done = false;
					curr.pos = pos;
				}
			}
		}
	} while (not done);
}

// horizontal size of pin
int Circuit::pinWidth(const Tech &tech, Index p) const {
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
int Circuit::pinHeight(Index p) const {
	int device = pin(p).device;
	if (device >= 0) {
		// this pin is a transistor, use width of transistor
		return mos[device].size[1];
	}
	// this is a contact, height should be min of transistor widths on either side.
	int result = -1;
	if (p.pin > 0) {
		int leftDevice = stack[p.type].pins[p.pin-1].device;
		if (leftDevice >= 0 and (result < 0 or mos[leftDevice].size[1] < result)) {
			result = mos[leftDevice].size[1];
		}
	}
	if (p.pin+1 < (int)stack[p.type].pins.size()) {
		int rightDevice = stack[p.type].pins[p.pin+1].device;
		if (rightDevice >= 0 and (result < 0 or mos[rightDevice].size[1] < result)) {
			result = mos[rightDevice].size[1];
		}
	}
	if (result < 0) {
		return 0;
	}
	return result;
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
