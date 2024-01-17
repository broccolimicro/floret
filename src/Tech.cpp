#include "Tech.h"
#include "Solution.h"

Layer::Layer() {
	this->name = "";
	this->major = 0;
	this->minor = 0;
	this->minSpacing = 0;
	this->minWidth = 0;
}

Layer::Layer(string name, int major, int minor) {
	this->name = name;
	this->major = major;
	this->minor = minor;
	this->minSpacing = 0;
	this->minWidth = 0;
}

Layer::~Layer() {
}

Diffusion::Diffusion() {
	layer = -1;
	overhangX = 0;
	overhangY = 0;
}

Diffusion::Diffusion(int layer, int overhangX, int overhangY) {
	this->layer = layer;
	this->overhangX = overhangX;
	this->overhangY = overhangY;
}

Diffusion::~Diffusion() {
}

Model::Model() {
	name = "";
	type = NMOS;
	viaPolySpacing = 0;
	polyOverhang = 0;
}

Model::Model(int type, string name, int viaPolySpacing, int polyOverhang) {
	this->name = name;
	this->type = type;
	this->viaPolySpacing = viaPolySpacing;
	this->polyOverhang = polyOverhang;
}

Model::~Model() {
}

Routing::Routing() {
	drawingLayer = -1;
	pinLayer = -1;
	labelLayer = -1;
}

Routing::Routing(int drawing, int pin, int label) {
	drawingLayer = drawing;
	pinLayer = pin;
	labelLayer = label;
}

Routing::~Routing() {
}

Via::Via() {
	upLevel = 0;
	downLevel = 0;
	drawingLayer = -1;
	downLo = 0;
	downHi = 0;
	upLo = 0;
	upHi = 0;
}

Via::Via(int downLevel, int upLevel, int layer, int downLo, int downHi, int upLo, int upHi) {
	this->downLevel = downLevel;
	this->upLevel = upLevel;
	this->drawingLayer = layer;
	this->downLo = downLo;
	this->downHi = downHi;
	this->upLo = upLo;
	this->upHi = upHi;
}

Via::~Via() {
}

Tech::Tech() {
	dbunit = 5e-3;

	// TODO(edward.bingham) hardcoding tech configuration values for now, but
	// this should be parsed in from python
	layers.push_back(Layer("diff.drawing", 65, 20));
	layers.back().minWidth = 30;
	layers.back().minSpacing = 54;
	layers.push_back(Layer("tap.drawing", 65, 44));
	layers.push_back(Layer("nwell.drawing", 64, 20));
	layers.push_back(Layer("dnwell.drawing", 64, 18));
	layers.push_back(Layer("pwbm.drawing", 19, 44));
	layers.push_back(Layer("pwde.drawing", 124, 20));
	layers.push_back(Layer("hvtr.drawing", 18, 20));
	layers.push_back(Layer("hvtp.drawing", 78, 44));
	layers.back().minWidth = 76;
	layers.back().minSpacing = 76;
	layers.push_back(Layer("ldntm.drawing", 11, 44));
	layers.push_back(Layer("hvi.drawing", 75, 20));
	layers.push_back(Layer("tunm.drawing", 80, 20));
	layers.push_back(Layer("lvtn.drawing", 125, 44));
	layers.push_back(Layer("poly.drawing", 66, 20));
	layers.back().minWidth = 30;
	layers.back().minSpacing = 42;
	layers.push_back(Layer("hvntm.drawing", 125, 20));
	layers.push_back(Layer("nsdm.drawing", 93, 44));
	layers.push_back(Layer("psdm.drawing", 94, 20));
	layers.push_back(Layer("rpm.drawing", 86, 20));
	layers.push_back(Layer("urpm.drawing", 79, 20));
	layers.push_back(Layer("npc.drawing", 95, 20));
	layers.push_back(Layer("licon1.drawing", 66, 44));
	layers.back().minWidth = 34;
	layers.back().minSpacing = 34;
	layers.push_back(Layer("li1.drawing", 67, 20));
	layers.back().minWidth = 34;
	layers.back().minSpacing = 34;
	layers.push_back(Layer("mcon.drawing", 67, 44));
	layers.back().minWidth = 34;
	layers.back().minSpacing = 38;
	layers.push_back(Layer("met1.drawing", 68, 20));
	layers.back().minWidth = 28;
	layers.back().minSpacing = 28;
	layers.push_back(Layer("via.drawing", 68, 44));
	layers.back().minWidth = 30;
	layers.back().minSpacing = 34;
	layers.push_back(Layer("met2.drawing", 69, 20));
	layers.back().minWidth = 28;
	layers.back().minSpacing = 28;
	layers.push_back(Layer("via2.drawing", 69, 44));
	layers.back().minWidth = 40;
	layers.back().minSpacing = 40;
	layers.push_back(Layer("met3.drawing", 70, 20));
	layers.back().minWidth = 60;
	layers.back().minSpacing = 60;
	layers.push_back(Layer("via3.drawing", 70, 44));
	layers.back().minWidth = 40;
	layers.back().minSpacing = 40;
	layers.push_back(Layer("met4.drawing", 71, 20));
	layers.back().minWidth = 60;
	layers.back().minSpacing = 60;
	layers.push_back(Layer("via4.drawing", 71, 44));
	layers.back().minWidth = 160;
	layers.back().minSpacing = 160;
	layers.push_back(Layer("met5.drawing", 72, 20));
	layers.back().minWidth = 320;
	layers.back().minSpacing = 320;
	layers.push_back(Layer("pad.drawing", 76, 20));
	layers.push_back(Layer("nsm.drawing", 61, 20));
	layers.push_back(Layer("capm.drawing", 89, 44));
	layers.push_back(Layer("cap2m.drawing", 97, 44));
	layers.push_back(Layer("vhvi.drawing", 74, 21));
	layers.push_back(Layer("uhvi.drawing", 74, 22));
	layers.push_back(Layer("npn.drawing", 82, 20));
	layers.push_back(Layer("inductor.drawing", 82, 24));
	layers.push_back(Layer("capacitor.drawing", 82, 64));
	layers.push_back(Layer("pnp.drawing", 82, 44));
	layers.push_back(Layer("lvsPrune.drawing", 84, 44));
	layers.push_back(Layer("ncm.drawing", 92, 44));
	layers.push_back(Layer("padcenter.drawing", 81, 20));
	layers.push_back(Layer("target.drawing", 76, 44));
	layers.push_back(Layer("areaid.sl.identifier", 81, 1));
	layers.push_back(Layer("areaid.ce.identifier", 81, 2));
	layers.push_back(Layer("areaid.fe.identifier", 81, 3));
	layers.push_back(Layer("areaid.sc.identifier", 81, 4));
	layers.push_back(Layer("areaid.sf.identifier", 81, 6));
	layers.push_back(Layer("areaid.sl.identifier1", 81, 7));
	layers.push_back(Layer("areaid.sr.identifier", 81, 8));
	layers.push_back(Layer("areaid.mt.identifier", 81, 10));
	layers.push_back(Layer("areaid.dt.identifier", 81, 11));
	layers.push_back(Layer("areaid.ft.identifier", 81, 12));
	layers.push_back(Layer("areaid.ww.identifier", 81, 13));
	layers.push_back(Layer("areaid.ld.identifier", 81, 14));
	layers.push_back(Layer("areaid.ns.identifier", 81, 15));
	layers.push_back(Layer("areaid.ij.identifier", 81, 17));
	layers.push_back(Layer("areaid.zr.identifier", 81, 18));
	layers.push_back(Layer("areaid.ed.identifier", 81, 19));
	layers.push_back(Layer("areaid.de.identifier", 81, 23));
	layers.push_back(Layer("areaid.rd.identifier", 81, 24));
	layers.push_back(Layer("areaid.dn.identifier", 81, 50));
	layers.push_back(Layer("areaid.cr.identifier", 81, 51));
	layers.push_back(Layer("areaid.cd.identifier", 81, 52));
	layers.push_back(Layer("areaid.st.identifier", 81, 53));
	layers.push_back(Layer("areaid.op.identifier", 81, 54));
	layers.push_back(Layer("areaid.en.identifier", 81, 57));
	layers.push_back(Layer("areaid.en20.identifier", 81, 58));
	layers.push_back(Layer("areaid.le.identifier", 81, 60));
	layers.push_back(Layer("areaid.hl.identifier", 81, 63));
	layers.push_back(Layer("areaid.sd.identifier", 81, 70));
	layers.push_back(Layer("areaid.po.identifier", 81, 81));
	layers.push_back(Layer("areaid.it.identifier", 81, 84));
	layers.push_back(Layer("areaid.et.identifier", 81, 101));
	layers.push_back(Layer("areaid.lvt.identifier", 81, 108));
	layers.push_back(Layer("areaid.re.identifier", 81, 125));
	layers.push_back(Layer("fom.dummy", 22, 23));
	layers.push_back(Layer("poly.gate", 66, 9));
	layers.push_back(Layer("poly.model", 66, 83));
	layers.push_back(Layer("poly.resistor", 66, 13));
	layers.push_back(Layer("diff.resistor", 65, 13));
	layers.push_back(Layer("pwell.resistor", 64, 13));
	layers.push_back(Layer("li1.resistor", 67, 13));
	layers.push_back(Layer("diff.highVoltage", 65, 8));
	layers.push_back(Layer("met4.fuse", 71, 17));
	layers.push_back(Layer("inductor.terminal1", 82, 26));
	layers.push_back(Layer("inductor.terminal2", 82, 27));
	layers.push_back(Layer("inductor.terminal3", 82, 28));
	layers.push_back(Layer("li1.block", 67, 10));
	layers.push_back(Layer("met1.block", 68, 10));
	layers.push_back(Layer("met2.block", 69, 10));
	layers.push_back(Layer("met3.block", 70, 10));
	layers.push_back(Layer("met4.block", 71, 10));
	layers.push_back(Layer("met5.block", 72, 10));
	layers.push_back(Layer("prbndry.boundary", 235, 4));
	layers.push_back(Layer("diff.boundary", 65, 4));
	layers.push_back(Layer("tap.boundary", 65, 60));
	layers.push_back(Layer("mcon.boundary", 67, 60));
	layers.push_back(Layer("poly.boundary", 66, 4));
	layers.push_back(Layer("via.boundary", 68, 60));
	layers.push_back(Layer("via2.boundary", 69, 60));
	layers.push_back(Layer("via3.boundary", 70, 60));
	layers.push_back(Layer("via4.boundary", 71, 60));
	layers.push_back(Layer("li1.label", 67, 5));
	layers.push_back(Layer("met1.label", 68, 5));
	layers.push_back(Layer("met2.label", 69, 5));
	layers.push_back(Layer("met3.label", 70, 5));
	layers.push_back(Layer("met4.label", 71, 5));
	layers.push_back(Layer("met5.label", 72, 5));
	layers.push_back(Layer("poly.label", 66, 5));
	layers.push_back(Layer("diff.label", 65, 6));
	layers.push_back(Layer("pwell.label", 64, 59));
	layers.push_back(Layer("pwelliso.label", 44, 5));
	layers.push_back(Layer("pad.label", 76, 5));
	layers.push_back(Layer("tap.label", 65, 5));
	layers.push_back(Layer("nwell.label", 64, 5));
	layers.push_back(Layer("inductor.label", 82, 25));
	layers.push_back(Layer("text.label", 83, 44));
	layers.push_back(Layer("li1.net", 67, 23));
	layers.push_back(Layer("met1.net", 68, 23));
	layers.push_back(Layer("met2.net", 69, 23));
	layers.push_back(Layer("met3.net", 70, 23));
	layers.push_back(Layer("met4.net", 71, 23));
	layers.push_back(Layer("met5.net", 72, 23));
	layers.push_back(Layer("poly.net", 66, 23));
	layers.push_back(Layer("diff.net", 65, 23));
	layers.push_back(Layer("li1.pin", 67, 16));
	layers.push_back(Layer("met1.pin", 68, 16));
	layers.push_back(Layer("met2.pin", 69, 16));
	layers.push_back(Layer("met3.pin", 70, 16));
	layers.push_back(Layer("met4.pin", 71, 16));
	layers.push_back(Layer("met5.pin", 72, 16));
	layers.push_back(Layer("poly.pin", 66, 16));
	layers.push_back(Layer("diff.pin", 65, 16));
	layers.push_back(Layer("nwell.pin", 64, 16));
	layers.push_back(Layer("pad.pin", 76, 16));
	layers.push_back(Layer("pwell.pin", 122, 16));
	layers.push_back(Layer("pwelliso.pin", 44, 16));
	layers.push_back(Layer("nwell.pin1", 64, 0));
	layers.push_back(Layer("poly.pin1", 66, 0));
	layers.push_back(Layer("li1.pin1", 67, 0));
	layers.push_back(Layer("met1.pin1", 68, 0));
	layers.push_back(Layer("met2.pin1", 69, 0));
	layers.push_back(Layer("met3.pin1", 70, 0));
	layers.push_back(Layer("met4.pin1", 71, 0));
	layers.push_back(Layer("met5.pin1", 72, 0));
	layers.push_back(Layer("pad.pin1", 76, 0));
	layers.push_back(Layer("pwell.pin1", 122, 0));
	layers.push_back(Layer("diff.cut", 65, 14));
	layers.push_back(Layer("poly.cut", 66, 14));
	layers.push_back(Layer("li1.cut", 67, 14));
	layers.push_back(Layer("met1.cut", 68, 14));
	layers.push_back(Layer("met2.cut", 69, 14));
	layers.push_back(Layer("met3.cut", 70, 14));
	layers.push_back(Layer("met4.cut", 71, 14));
	layers.push_back(Layer("met5.cut", 72, 14));
	layers.push_back(Layer("met5.probe", 72, 25));
	layers.push_back(Layer("met4.probe", 71, 25));
	layers.push_back(Layer("met3.probe", 70, 25));
	layers.push_back(Layer("met2.probe", 69, 25));
	layers.push_back(Layer("met1.probe", 68, 25));
	layers.push_back(Layer("li1.probe", 67, 25));
	layers.push_back(Layer("poly.probe", 66, 25));
	layers.push_back(Layer("poly.short", 66, 15));
	layers.push_back(Layer("li1.short", 67, 15));
	layers.push_back(Layer("met1.short", 68, 15));
	layers.push_back(Layer("met2.short", 69, 15));
	layers.push_back(Layer("met3.short", 70, 15));
	layers.push_back(Layer("met4.short", 71, 15));
	layers.push_back(Layer("met5.short", 72, 15));
	layers.push_back(Layer("cncm.mask", 17, 0));
	layers.push_back(Layer("crpm.mask", 96, 0));
	layers.push_back(Layer("cpdm.mask", 37, 0));
	layers.push_back(Layer("cnsm.mask", 22, 0));
	layers.push_back(Layer("cmm5.mask", 59, 0));
	layers.push_back(Layer("cviam4.mask", 58, 0));
	layers.push_back(Layer("cmm4.mask", 51, 0));
	layers.push_back(Layer("cviam3.mask", 50, 0));
	layers.push_back(Layer("cmm3.mask", 34, 0));
	layers.push_back(Layer("cviam2.mask", 44, 0));
	layers.push_back(Layer("cmm2.mask", 41, 0));
	layers.push_back(Layer("cviam.mask", 40, 0));
	layers.push_back(Layer("cmm1.mask", 36, 0));
	layers.push_back(Layer("ctm1.mask", 35, 0));
	layers.push_back(Layer("cli1m.mask", 56, 0));
	layers.push_back(Layer("clicm1.mask", 43, 0));
	layers.push_back(Layer("cpsdm.mask", 32, 0));
	layers.push_back(Layer("cnsdm.mask", 30, 0));
	layers.push_back(Layer("cldntm.mask", 11, 0));
	layers.push_back(Layer("cnpc.mask", 49, 0));
	layers.push_back(Layer("chvntm.mask", 39, 0));
	layers.push_back(Layer("cntm.mask", 27, 0));
	layers.push_back(Layer("cp1m.mask", 28, 0));
	layers.push_back(Layer("clvom.mask", 46, 0));
	layers.push_back(Layer("conom.mask", 88, 0));
	layers.push_back(Layer("ctunm.mask", 20, 0));
	layers.push_back(Layer("chvtrm.mask", 98, 0));
	layers.push_back(Layer("chvtpm.mask", 97, 0));
	layers.push_back(Layer("clvtnm.mask", 25, 0));
	layers.push_back(Layer("cnwm.mask", 21, 0));
	layers.push_back(Layer("cdnm.mask", 48, 0));
	layers.push_back(Layer("cfom.mask", 23, 0));
	layers.push_back(Layer("cfom.drawing", 22, 20));
	layers.push_back(Layer("clvtnm.drawing", 25, 44));
	layers.push_back(Layer("chvtpm.drawing", 88, 44));
	layers.push_back(Layer("conom.drawing", 87, 44));
	layers.push_back(Layer("clvom.drawing", 45, 20));
	layers.push_back(Layer("cntm.drawing", 26, 20));
	layers.push_back(Layer("chvntm.drawing", 38, 20));
	layers.push_back(Layer("cnpc.drawing", 44, 20));
	layers.push_back(Layer("cnsdm.drawing", 29, 20));
	layers.push_back(Layer("cpsdm.drawing", 31, 20));
	layers.push_back(Layer("cli1m.drawing", 115, 44));
	layers.push_back(Layer("cviam3.drawing", 112, 20));
	layers.push_back(Layer("cviam4.drawing", 117, 20));
	layers.push_back(Layer("cncm.drawing", 96, 44));
	layers.push_back(Layer("cntm.maskAdd", 26, 21));
	layers.push_back(Layer("clvtnm.maskAdd", 25, 43));
	layers.push_back(Layer("chvtpm.maskAdd", 97, 43));
	layers.push_back(Layer("cli1m.maskAdd", 115, 43));
	layers.push_back(Layer("clicm1.maskAdd", 106, 43));
	layers.push_back(Layer("cpsdm.maskAdd", 31, 21));
	layers.push_back(Layer("cnsdm.maskAdd", 29, 21));
	layers.push_back(Layer("cp1m.maskAdd", 33, 43));
	layers.push_back(Layer("cfom.maskAdd", 22, 21));
	layers.push_back(Layer("cntm.maskDrop", 26, 22));
	layers.push_back(Layer("clvtnm.maskDrop", 25, 42));
	layers.push_back(Layer("chvtpm.maskDrop", 97, 42));
	layers.push_back(Layer("cli1m.maskDrop", 115, 42));
	layers.push_back(Layer("clicm1.maskDrop", 106, 42));
	layers.push_back(Layer("cpsdm.maskDrop", 31, 22));
	layers.push_back(Layer("cnsdm.maskDrop", 29, 22));
	layers.push_back(Layer("cp1m.maskDrop", 33, 42));
	layers.push_back(Layer("cfom.maskDrop", 22, 22));
	layers.push_back(Layer("cmm4.waffleDrop", 112, 4));
	layers.push_back(Layer("cmm3.waffleDrop", 107, 24));
	layers.push_back(Layer("cmm2.waffleDrop", 105, 52));
	layers.push_back(Layer("cmm1.waffleDrop", 62, 24));
	layers.push_back(Layer("cp1m.waffleDrop", 33, 24));
	layers.push_back(Layer("cfom.waffleDrop", 22, 24));
	layers.push_back(Layer("cmm5.waffleDrop", 117, 4));

	models.push_back(Model(Model::NMOS, "sky130_fd_pr__nfet_01v8", 18, 26));
	models.back().layers.push_back(Diffusion(findLayer("diff.drawing"), 50, 0));
	models.back().layers.push_back(Diffusion(findLayer("nsdm.drawing"), 25, 25));
	models.push_back(Model(Model::PMOS, "sky130_fd_pr__pfet_01v8", 18, 26));
	models.back().layers.push_back(Diffusion(findLayer("diff.drawing"), 50, 0));
	models.back().layers.push_back(Diffusion(findLayer("psdm.drawing"), 25, 25));
	models.back().layers.push_back(Diffusion(findLayer("nwell.drawing"), 36, 36));
	models.push_back(Model(Model::PMOS, "sky130_fd_pr__pfet_01v8_hvt", 18, 26));
	models.back().layers.push_back(Diffusion(findLayer("diff.drawing"), 50, 0));
	models.back().layers.push_back(Diffusion(findLayer("psdm.drawing"), 25, 25));
	models.back().layers.push_back(Diffusion(findLayer("hvtp.drawing"), 11, 11));
	models.back().layers.push_back(Diffusion(findLayer("nwell.drawing"), 36, 36));


	vias.push_back(Via(-1, 1, findLayer("licon1.drawing"), 8, 12, 0, 16));
	vias.push_back(Via(-2, 1, findLayer("licon1.drawing"), 8, 12, 0, 16));
	vias.push_back(Via(-3, 1, findLayer("licon1.drawing"), 8, 12, 0, 16));

	vias.push_back(Via(0, 1, findLayer("licon1.drawing"), 10, 16, 0, 16));
	vias.push_back(Via(1, 2, findLayer("mcon.drawing"), 0, 0, 6, 12));
	vias.push_back(Via(2, 3, findLayer("via.drawing"), 11, 11, 11, 17));
	vias.push_back(Via(3, 4, findLayer("via2.drawing"), 8, 17, 13, 13));
	vias.push_back(Via(4, 5, findLayer("via3.drawing"), 12, 18, 13, 13));
	vias.push_back(Via(5, 6, findLayer("via4.drawing"), 38, 38, 62, 62));

	wires.push_back(Routing(findLayer("poly.drawing"), findLayer("poly.pin"), findLayer("poly.label")));
	wires.push_back(Routing(findLayer("li1.drawing"), findLayer("li1.pin"), findLayer("li1.label")));
	wires.push_back(Routing(findLayer("met1.drawing"), findLayer("met1.pin"), findLayer("met1.label")));
	wires.push_back(Routing(findLayer("met2.drawing"), findLayer("met2.pin"), findLayer("met2.label")));
	wires.push_back(Routing(findLayer("met3.drawing"), findLayer("met3.pin"), findLayer("met3.label")));
	wires.push_back(Routing(findLayer("met4.drawing"), findLayer("met4.pin"), findLayer("met4.label")));
	wires.push_back(Routing(findLayer("met5.drawing"), findLayer("met5.pin"), findLayer("met5.label")));

	boundary = findLayer("areaid.sc.identifier");
}

Tech::~Tech() {
}

int Tech::findLayer(string name) const {
	for (int i = 0; i < (int)layers.size(); i++) {
		if (layers[i].name == name) {
			return i;
		}
	}

	return -1;
}

int Tech::findModel(string name) const {
	for (int i = 0; i < (int)models.size(); i++) {
		if (models[i].name == name) {
			return i;
		}
	}

	return -1;
}

vector<int> Tech::findVias(int downLevel, int upLevel) const {
	int curr = downLevel;
	
	vector<int> result;
	bool done = false;
	while (not done) {
		done = true;
		for (int i = 0; curr != upLevel and i < (int)vias.size(); i++) {
			if (vias[i].downLevel == curr) {
				result.push_back(i);
				curr = vias[i].upLevel;
				done = false;
			}
		}
	}
	return result;
}

// TODO(edward.bingham) instead of manually computing spacing for different situations, we should pre-layout each transistor, contact, and wire, and then use the DRC rules directly on the geometry to compute spacing. This would ensure that every cell is guaranteed to be DRC error free.

// horizontal size of pin
int Tech::hSize(const Solution *ckt, Index p) const {
	int device = ckt->pin(p).device;
	if (device >= 0) {
		// this pin is a transistor, use length of transistor
		return ckt->base->mos[device].size[0];
	}
	// this pin is a contact
	return layers[vias[0].drawingLayer].minWidth;
}

// vertical size of pin
int Tech::vSize(const Solution *ckt, Index p) const {
	int device = ckt->pin(p).device;
	if (device >= 0) {
		// this pin is a transistor, use width of transistor
		return ckt->base->mos[device].size[1];
	}
	// this is a contact, height should be min of transistor widths on either side.
	int result = -1;
	if (p.pin > 0) {
		int leftDevice = ckt->stack[p.type][p.pin-1].device;
		if (leftDevice >= 0 and (result < 0 or ckt->base->mos[leftDevice].size[1] < result)) {
			result = ckt->base->mos[leftDevice].size[1];
		}
	}
	if (p.pin+1 < (int)ckt->stack[p.type].size()) {
		int rightDevice = ckt->stack[p.type][p.pin+1].device;
		if (rightDevice >= 0 and (result < 0 or ckt->base->mos[rightDevice].size[1] < result)) {
			result = ckt->base->mos[rightDevice].size[1];
		}
	}
	if (result < 0) {
		result = layers[vias[0].drawingLayer].minWidth;
	}
	
	return result;
}

// vertical size of wire
int Tech::vSize(const Solution *ckt, int w) const {
	return layers[vias[2].drawingLayer].minWidth +
	  max(vias[2].upLo, vias[2].downLo);
}

// horizontal spacing between two pins
int Tech::hSpacing(const Solution *ckt, Index p0, Index p1) const {
	int dev0 = ckt->pin(p0).device;
	int dev1 = ckt->pin(p1).device;
	if (dev0 >= 0 and dev1 >= 0) {
		// transistor to transistor
		return layers[wires[0].drawingLayer].minSpacing;
	} else if (dev0 >= 0 and dev1 < 0) {
		// transistor to contact
		return models[ckt->base->mos[dev0].model].viaPolySpacing;
	}	else if (dev0 < 0 and dev1 >= 0) {
		// contact to transistor
		return models[ckt->base->mos[dev1].model].viaPolySpacing;
	}

	int leftModel = -1;
	int rightModel = -1;
	for (Index i(p0.type, p0.pin-1); i.pin >= 0; i.pin--) {
		int device = ckt->pin(i).device;
		if (device >= 0) {
			leftModel = ckt->base->mos[device].model;
		}
	}
	for (Index i(p1.type, p1.pin+1); i.pin < (int)ckt->stack[p1.type].size(); i.pin++) {
		int device = ckt->pin(i).device;
		if (device >= 0) {
			rightModel = ckt->base->mos[device].model;
		}
	}

	vector<int> leftVias = findVias(-leftModel-1, 1);
	vector<int> rightVias = findVias(-rightModel-1, 1);

	if (leftVias.size() == 0 or rightVias.size() == 0) {
		return 0;
	}

	int diffLayer = models[leftModel].layers[0].layer;
	// contact to contact
	return vias[leftVias[0]].downLo +
	       layers[diffLayer].minSpacing + 
	       vias[rightVias[0]].downLo;
}

// horizontal spacing between two wires
// Assume that the wires are measured from the position of the first
// pin to the position of the last pin plus that pin's width. This
// means that the wires' left and right measurements don't include
// via enclosure or spacing between vias.
int Tech::hSpacing(const Solution *ckt, int w0, int w1) const {
	return layers[wires[2].drawingLayer].minSpacing;
}

int Tech::vSpacing(const Solution *ckt, Index p, int w) const {
	int device = ckt->pin(p).device;
	if (device >= 0) {
		// this is a transistor gate
		return ckt->pin(p).height+38;
	} else {
		// this is a contact
		return 0;
	}
}

int Tech::vSpacing(const Solution *ckt, int w0, int w1) const {
	return layers[wires[2].drawingLayer].minSpacing;
}

int Tech::vSpacing(const Solution *ckt, int w, Index p) const {
	int device = ckt->pin(p).device;
	if (device >= 0) {
		// this is a transistor gate
		return ckt->pin(p).height+38;
	} else {
		// this is a contact
		return 0;
	}
}

int Tech::vSpacing(const Solution *ckt, Index p0, Index p1) const {
	return layers[wires[2].drawingLayer].minSpacing;
}

