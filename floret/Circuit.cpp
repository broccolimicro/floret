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

bool operator<(const Index &i0, const Index &i1) {
	return i0.type < i1.type or (i0.type == i1.type and i0.pin < i1.pin);
}

bool operator==(const Index &i0, const Index &i1) {
	return i0.type == i1.type and i0.pin == i1.pin;
}

bool operator!=(const Index &i0, const Index &i1) {
	return i0.type != i1.type or i0.pin != i1.pin;
}

Pin::Pin(const Tech &tech) : layout(tech) {
	device = -1;
	outNet = -1;
	leftNet = -1;
	rightNet = -1;
	
	align = -1;

	layer = 0;
	width = 0;
	height = 0;
	pos = 0;
	lo = numeric_limits<int>::max();
	hi = numeric_limits<int>::min();
}

Pin::Pin(const Tech &tech, int outNet) : layout(tech) {
	this->device = -1;
	this->outNet = outNet;
	this->leftNet = outNet;
	this->rightNet = outNet;

	align = -1;

	layer = 1;
	width = 0;
	height = 0;
	pos = 0;
	lo = numeric_limits<int>::max();
	hi = numeric_limits<int>::min();
}

Pin::Pin(const Tech &tech, int device, int outNet, int leftNet, int rightNet) : layout(tech) {
	this->device = device;
	this->outNet = outNet;
	this->leftNet = leftNet;
	this->rightNet = rightNet;

	align = -1;

	layer = 0;
	width = 0;
	height = 0;
	pos = 0;
	lo = numeric_limits<int>::max();
	hi = numeric_limits<int>::min();
}

Pin::~Pin() {
}

void Pin::offsetToPin(Index pin, int value) {
	printf("offsetToPin %d\n", value);
	auto result = toPin.insert(pair<Index, int>(pin, value));
	if (not result.second) {
		result.first->second = max(result.first->second, value);
	}
	printf("Done inserting %d\n", result.first->second);
}

bool Pin::isGate() const {
	return device >= 0;
}

bool Pin::isContact() const {
	return device < 0;
}

Contact::Contact(const Tech &tech) : layout(tech) {
	left = numeric_limits<int>::min();
	right = numeric_limits<int>::max();
}

Contact::Contact(const Tech &tech, Index idx) : layout(tech) {
	this->idx = idx;
	this->left = numeric_limits<int>::min();
	this->right = numeric_limits<int>::max();
}

Contact::~Contact() {
}

void Contact::offsetFromPin(Index pin, int value) {
	auto result = fromPin.insert(pair<Index, int>(pin, value));
	if (not result.second) {
		result.first->second = max(result.first->second, value);
	}
}

void Contact::offsetToPin(Index pin, int value) {
	auto result = toPin.insert(pair<Index, int>(pin, value));
	if (not result.second) {
		result.first->second = max(result.first->second, value);
	}
}

bool operator<(const Contact &c0, const Contact &c1) {
	return c0.idx < c1.idx;
}

bool operator==(const Contact &c0, const Contact &c1) {
	return c0.idx == c1.idx;
}

bool operator!=(const Contact &c0, const Contact &c1) {
	return c0.idx != c1.idx;
}

CompareIndex::CompareIndex(const Circuit *s) {
	this->s = s;
}

CompareIndex::~CompareIndex() {
}

bool CompareIndex::operator()(const Index &i0, const Index &i1) {
	const Pin &p0 = s->pin(i0);
	const Pin &p1 = s->pin(i1);
	return p0.pos < p1.pos or (p0.pos == p1.pos and i0 < i1);
}

bool CompareIndex::operator()(const Contact &c0, const Index &i1) {
	const Pin &p0 = s->pin(c0.idx);
	const Pin &p1 = s->pin(i1);
	return p0.pos < p1.pos or (p0.pos == p1.pos and c0.idx < i1);
}

bool CompareIndex::operator()(const Contact &c0, const Contact &c1) {
	const Pin &p0 = s->pin(c0.idx);
	const Pin &p1 = s->pin(c1.idx);
	return p0.pos < p1.pos or (p0.pos == p1.pos and c0.idx < c1.idx);
}

Wire::Wire(const Tech &tech) : layout(tech) {
	net = -1;
	left = -1;
	right = -1;
	pOffset = 0;
	nOffset = 0;
}

Wire::Wire(const Tech &tech, int net) : layout(tech) {
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
	pins.insert(pos, Contact(*(s->tech), pin));
	if (left < 0 or s->stack[pin.type].pins[pin.pin].pos < left) {
		left = s->stack[pin.type].pins[pin.pin].pos;
	}
	if (right < 0 or s->stack[pin.type].pins[pin.pin].pos+s->stack[pin.type].pins[pin.pin].width > right) {
		right = s->stack[pin.type].pins[pin.pin].pos + s->stack[pin.type].pins[pin.pin].width;
	}
}

bool Wire::hasPin(const Circuit *s, Index pin, vector<Contact>::iterator *out) {
	auto pos = lower_bound(pins.begin(), pins.end(), pin, CompareIndex(s));
	if (out != nullptr) {
		*out = pos;
	}
	return pos != pins.end() and pos->idx == pin;
}

void Wire::resortPins(const Circuit *s) {
	sort(pins.begin(), pins.end(), CompareIndex(s));
}

int Wire::getLevel(int i) const {
	if ((int)level.size() == 0) {
		return 2;
	}

	if (i < 0) {
		return level[0];
	}

	if (i >= (int)level.size()) {
		return level[level.size()-1];
	}

	return level[i];
}

bool Wire::hasPrev(int r) const {
	return prevNodes.find(r) != prevNodes.end();
}

bool Wire::hasGate(const Circuit *s) const {
	for (int i = 0; i < (int)pins.size(); i++) {
		if (s->pin(pins[i].idx).device >= 0) {
			return true;
		}
	}
	return false;
}

vector<bool> Wire::pinTypes() const {
	vector<bool> result(3,false);
	for (int i = 0; i < (int)pins.size(); i++) {
		result[pins[i].idx.type] = true;
	}
	return result;
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

	if (not link and not pins.empty() and pins.back().device >= 0) {
		pins.push_back(Pin(*(ckt->tech), pins.back().rightNet));
	}

	if (fromNet >= 0) {
		bool hasContact = (ckt->nets[fromNet].ports[type] > 2 or ckt->nets[fromNet].ports[1-type] > 0 or ckt->nets[fromNet].gates[0] > 0 or ckt->nets[fromNet].gates[1] > 0);
		if (fromNet >= 0 and (not link or pins.empty() or hasContact or ckt->nets[fromNet].isIO)) {
			// Add a contact for the first net or between two transistors.
			pins.push_back(Pin(*(ckt->tech), fromNet));
		}
	}

	if (device >= 0) {
		pins.push_back(Pin(*(ckt->tech), device, gateNet, fromNet, toNet));
	}
}

void Stack::draw(const Tech &tech, const Circuit *base, Layout &dst) {
	dst.clear();
	// Draw the stacks
	for (int i = 0; i < (int)pins.size(); i++) {
		vec2i dir = vec2i(1, type == Model::NMOS ? -1 : 1);
		drawLayout(dst, pins[i].layout, vec2i(pins[i].pos, 0), dir);
		if (i > 0 and (pins[i].device >= 0 or pins[i-1].device >= 0)) {
			int height = min(pins[i].height, pins[i-1].height);
			int model = -1;
			if (pins[i].device >= 0) {
				model = base->mos[pins[i].device].model;
			} else {
				model = base->mos[pins[i-1].device].model;
			}

			drawDiffusion(tech, dst, model, -1, vec2i(pins[i-1].pos, 0), vec2i(pins[i].pos, height)*dir, dir);
		}
	}
}

Circuit::Circuit(const Tech &tech) {
	this->tech = &tech;
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

bool Circuit::loadDevice(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev) {
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
	int modelIdx = tech->findModel(modelName);
	// if the modelName isn't in the model list, then this is a non-transistor subckt
	if (modelIdx < 0) {
		printf("model not found %s\n", modelName.c_str());
		return false;
	}

	int port = 0;
	int type = tech->models[modelIdx].type;
	this->mos.push_back(Mos(modelIdx, type));
	for (auto arg = args->tokens.begin(); arg != args->tokens.end(); arg++) {
		if (arg->type == lang.PARAM) {
			string paramName = lower(lexer.read(arg->tokens[0].begin, arg->tokens[0].end));
			vector<double> values;
			for (auto value = arg->tokens.begin()+1; value != arg->tokens.end(); value++) {
				values.push_back(loadValue(lang, lexer, *value));
			}
			if (paramName == "w") {
				this->mos.back().size[1] = int(values[0]/tech->dbunit);
			} else if (paramName == "l") {
				this->mos.back().size[0] = int(values[0]/tech->dbunit);
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

void Circuit::loadSubckt(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt) {
	for (auto tok = subckt.tokens.begin(); tok != subckt.tokens.end(); tok++) {
		if (tok->type == lang.NAME) {
			this->name = lexer.read(tok->begin, tok->end);
		} else if (tok->type == lang.PORT_LIST) {
			for (auto port = tok->tokens.begin(); port != tok->tokens.end(); port++) {
				this->nets.push_back(Net(lexer.read(port->begin, port->end), true));
			}
		} else if (tok->type == lang.DEVICE) {
			if (not loadDevice(lang, lexer, *tok)) {
				printf("unrecognized device\n");
			}
		}
	}
}

// horizontal size of pin
int Circuit::pinWidth(Index p) const {
	int device = pin(p).device;
	if (device >= 0) {
		// this pin is a transistor, use length of transistor
		return tech->paint[tech->wires[0].draw].minWidth;
		//return base->mos[device].size[0];
	}
	// this pin is a contact
	return tech->paint[tech->wires[1].draw].minWidth;
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
	if (p.pin-1 >= 0) {
		int leftDevice = stack[p.type].pins[p.pin-1].device;
		if (leftDevice >= 0 and (result < 0 or mos[leftDevice].size[1] < result)) {
			result = mos[leftDevice].size[1];
		}
	}
	for (int i = p.pin+1; i < (int)stack[p.type].pins.size(); i++) {
		int rightDevice = stack[p.type].pins[i].device;
		if (rightDevice >= 0 and (result < 0 or mos[rightDevice].size[1] < result)) {
			result = mos[rightDevice].size[1];
			break;
		} else if (result >= 0) {
			break;
		}
	}
	for (int i = p.pin-1; result < 0 and i >= 0; i--) {
		int leftDevice = stack[p.type].pins[i].device;
		if (leftDevice >= 0 and (result < 0 or mos[leftDevice].size[1] < result)) {
			result = mos[leftDevice].size[1];
		}
	}
	if (result < 0) {
		// This should never happen.
		printf("warning: failed to size the pin (%d,%d).\n", p.type, p.pin);
		return 0;
	}
	return result;
}

void Circuit::draw(Layout &dst) {
	vec2i dir(1,1);
	dst.name = name;

	dst.nets.reserve(nets.size());
	for (int i = 0; i < (int)nets.size(); i++) {
		dst.nets.push_back(nets[i].name);
	}

	for (int i = 0; i < (int)routes.size(); i++) {
		drawLayout(dst, routes[i].layout, vec2i(0, routes[i].pOffset)*dir, dir);
	}

	for (int type = 0; type < (int)stack.size(); type++) {
		for (int i = 0; i < (int)stack[type].pins.size(); i++) {
			const Pin &pin = stack[type].pins[i];
			bool first = true;
			int bottom = 0;
			int top = 0;
			
			int pinLevel = pin.layer;
			int pinLayer = tech->wires[pinLevel].draw;
			int width = tech->paint[pinLayer].minWidth;

			for (int j = 0; j < (int)routes.size(); j++) {
				if (routes[j].hasPin(this, Index(type, i))) {
					int v = routes[j].pOffset;
					if (routes[j].net >= 0) {
						top = first ? v+width : max(top, v+width);
					} else {
						top = first ? v : max(top, v);
					}
					bottom = first ? v : min(bottom, v);
					first = false;
				}
			}

 			dst.push(tech->wires[pinLevel], Rect(pin.outNet, vec2i(pin.pos, bottom), vec2i(pin.pos+width, top)));
		}
	}

	for (int i = 0; i < (int)dst.layers.size(); i++) {
		if (tech->paint[dst.layers[i].draw].fill) {
			Rect box = dst.layers[i].bbox();
			dst.layers[i].clear();
			dst.layers[i].push(box, true);
		}
	}

	dst.merge();
}
