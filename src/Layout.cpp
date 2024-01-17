#include "Layout.h"

Rect::Rect() {
	layer = -1;
	net = -1;
	left = 0;
	bottom = 0;
	right = 0;
	top = 0;
}

Rect::Rect(int layer, int net, vec2i ll, vec2i ur) {
	this->layer = layer;
	this->left = ll[0];
	this->bottom = ll[1];
	this->right = ur[0];
	this->top = ur[1];

	if (this->top < this->bottom) {
		int tmp = this->top;
		this->top = this->bottom;
		this->bottom = tmp;
	}

	if (this->right < this->left) {
		int tmp = this->right;
		this->right = this->left;
		this->left = tmp;
	}
}

Rect::~Rect() {
}

bool Rect::merge(Rect r) {
	if (layer == r.layer and net == r.net and left == r.left and right == r.right and bottom <= r.top and top >= r.bottom) {
		if (r.bottom < bottom) {
			bottom = r.bottom;
		}
		if (r.top > top) {
			top = r.top;
		}
		return true;
	} else if (layer == r.layer and net == r.net and bottom == r.bottom and top == r.top and left <= r.right and right >= r.left) {
		if (r.left < left) {
			left = r.left;
		}
		if (r.right > right) {
			right = r.right;
		}
		return true;
	} else if (layer == r.layer and net == r.net and bottom <= r.bottom and top >= r.top and left <= r.left and right >= r.right) {
		return true;
	} else if (layer == r.layer and net == r.net and bottom >= r.bottom and top <= r.top and left >= r.left and right <= r.right) {
		left = r.left;
		right = r.right;
		bottom = r.bottom;
		top = r.top;
		return true;
	}
	return false;
}

gdstk::Polygon *Rect::emit(const Tech &tech) const {
	return new gdstk::Polygon(gdstk::rectangle(gdstk::Vec2{(double)left, (double)bottom}, gdstk::Vec2{(double)right, (double)top}, gdstk::make_tag(tech.layers[layer].major, tech.layers[layer].minor)));
}

void mergeRects(vector<Rect> &rects) {
	for (int i = (int)rects.size()-1; i >= 0; i--) {
		for (int j = (int)rects.size()-1; j > i; j--) {
			if (rects[i].merge(rects[j])) {
				rects.erase(rects.begin()+j);
			}
		}
	}
}

Layout::Layout() {
}

Layout::~Layout() {
}

void Layout::drawTransistor(const Tech &tech, const Mos &mos, vec2i pos, vec2i dir) {
	vec2i ll = pos;
	vec2i ur = pos + mos.size*dir;

	// draw poly
	vec2i polyOverhang = vec2i(0, tech.models[mos.model].polyOverhang)*dir;
	geometry.push_back(Rect(tech.wires[0].drawingLayer, mos.ports[Mos::GATE], ll - polyOverhang, ur + polyOverhang));
	
	// draw diffusion
	for (auto layer = tech.models[mos.model].layers.begin(); layer != tech.models[mos.model].layers.end(); layer++) {
		vec2i diffOverhang = vec2i(layer->overhangX, layer->overhangY)*dir;
		ll -= diffOverhang;
		ur += diffOverhang;
		geometry.push_back(Rect(layer->layer, -1, ll, ur));
	}
}

/*void Layout::drawDiffContact(const Tech &tech, const Pin &pin, vec2i pos, vec2i dir) {
	int via = tech.vias[0].drawingLayer;
	int viaWidth = tech.layers[via].minWidth;
	int viaSpacing = tech.layers[via].minSpacing;
	int diffEncloseLo = tech.vias[0].downLo;
	int diffEncloseHi = tech.vias[0].downHi;

	int numVias = 1 + (width-viaWidth - 2*diffEncloseLo) / (viaSpacing+viaWidth);
	int contactWidth = numVias*viaWidth + (numVias-1)*viaSpacing;
	int contactOffset = (width - contactWidth)/2;

	int diffEncloseH = diffEncloseHi;
	if (contactOffset >= diffEncloseHi) {
		diffEncloseH = diffEncloseLo;
	}

	// draw diffusion
	int type = tech.models[model].type;
	vec2i diffPos(pos[0]-diffEncloseH*dir[0], pos[1]);
	int diffLength = (viaWidth + 2*diffEncloseH)*dir[0];
	int diffWidth = width*dir[1];
	for (auto layer = tech.models[model].layers.begin(); layer != tech.models[model].layers.end(); layer++) {
		if (layer != tech.models[model].layers.begin()) {
			diffPos[0] -= layer->overhangX*dir[0];
			diffPos[1] -= layer->overhangY*dir[1];
			diffLength += 2*layer->overhangX*dir[0];
			diffWidth += 2*layer->overhangY*dir[1];
		}
		diff[type].push_back(Rect(layer->layer, diffPos, diffLength, diffWidth));
	}

	pos[1] += dir[1]*contactOffset;
	if (dir[1] < 0) {
		pos[1] += dir[1]*contactWidth;
	}

	nets[net].contacts[type].push_back(Rect(tech.wires[1].drawingLayer, pos, viaWidth, contactWidth));

	for (int i = 0; i < numVias; i++) {
		nets[net].routes.push_back(Rect(via, pos, viaWidth, viaWidth));

		pos[1] += viaWidth+viaSpacing;
	}
}*/

void Layout::drawVia(const Tech &tech, int net, int level, vec2i size, vec2i pos, vec2i dir) {
	// layers
	int viaLayer = tech.vias[level].drawingLayer;
	int downLayer = tech.vias[level].from;
	int upLayer = tech.vias[level].to;

	// spacing and width of a via
	int viaWidth = tech.layers[viaLayer].minWidth;
	int viaSpacing = tech.layers[viaLayer].minSpacing;

	// enclosure rules
	vec2i dn(tech.vias[level].downLo, tech.vias[level].downHi);
	vec2i up(tech.vias[level].upLo, tech.vias[level].upHi);

	vec2i num = 1 + (size-viaWidth - 2*dn[0]) / (viaSpacing + viaWidth);
	vec2i width = num * viaWidth + (num-1)*viaSpacing;

	vec2i off(0,0);
	if (size[0] > width[0]) {
		off[0] = (size[0] - width[0])/2;
	}
	if (size[1] > width[1]) {
		off[1] = (size[1] - width[1])/2;
	}

	if (off[0] < dn[1]) {
		swap(dn[0], dn[1]);
	}
	if (off[0] < up[1]) {
		swap(up[0], up[1]);
	}
	
	//if (off[1] < max(dn[1], up[1])) {
	//	off[1] = max(dn[1], up[1]);
	//}

	if (level == 0) {
		dn[1] = off[1];
	}

	// draw down
	geometry.push_back(Rect(downLayer, net, pos+(off-dn)*dir, pos+(off+width+dn)*dir));

	// draw via
	vec2i idx;
	int step = viaWidth+viaSpacing;
	for (idx[0] = 0; idx[0] < num[0]; idx[0]++) {
		for (idx[1] = 0; idx[1] < num[1]; idx[1]++) {
			geometry.push_back(Rect(viaLayer, net, pos+(off+idx*step)*dir, pos+(off+idx*step+viaWidth)*dir));
		}
	}

	// draw up
	geometry.push_back(Rect(upLayer, net, pos+(off-up)*dir, pos+(off+width+up)*dir));
}

void Layout::drawWire(const Tech &tech, const Solution *ckt, const Wire &wire, vec2i pos, vec2i dir) {
	vec2i ll = pos+vec2i(wire.left,wire.inWeight)*dir;
	vec2i ur = pos+vec2i(wire.right,wire.inWeight+wire.height)*dir;

	geometry.push_back(Rect(tech.wires[wire.layer].drawingLayer, wire.net, ll, ur));

	for (auto pin = wire.pins.begin(); pin != wire.pins.end(); pin++) {
		int level = ckt->stack[pin->type][pin->pin].layer;
		int layer = tech.wires[level].drawingLayer;
		vec2i pp(ckt->stack[pin->type][pin->pin].pos, 0);
		vec2i ps(tech.layers[layer].minWidth,ckt->stack[pin->type][pin->pin].height/2);
		if (pin->type == Model::NMOS) {
			pp[1] = -ckt->cost;
			ps *= dir;
		}

		for (int i = level; i < 2; i++) {
			drawVia(tech, wire.net, i+1, vec2i(0, wire.height), vec2i(pp[0], ll[1]), dir);
		}
		geometry.push_back(Rect(layer, wire.net, vec2i(pp[0], ll[1]), pp+ps));
	}
}

void Layout::drawCell(const Tech &tech, const Solution *ckt) {
	name = ckt->base->name;

	for (int type = 0; type < 2; type++) {
		for (auto pin = ckt->stack[type].begin(); pin != ckt->stack[type].end(); pin++) {
			vec2i pos(pin->pos, 0);
			vec2i dir(1,1);
			if (type == Model::NMOS) {
				pos[1] = -ckt->cost;
				dir[1] = -1;
			}

			if (pin->device < 0) {
				drawVia(tech, pin->outNet, 0, vec2i(pin->width, pin->height), pos, dir);
			} else {
				drawTransistor(tech, ckt->base->mos[pin->device], pos, dir);
			}
		}
	}

	vec2i pos(0,0);
	vec2i dir(1,-1);
	for (int i = 0; i < (int)ckt->routes.size(); i++) {
		drawWire(tech, ckt, ckt->routes[i], pos, dir);
	}

	mergeRects(geometry);
}

gdstk::Label *Layout::emitLabel(const Tech &tech, vec2i pos, int layer, string text) const {
	return new gdstk::Label{
		.tag = gdstk::make_tag(tech.layers[layer].major, tech.layers[layer].minor),
		.text = strdup(text.c_str()),
		.origin = gdstk::Vec2{(double)pos[0], (double)pos[1]},
		.magnification = 1,
	};
}

void Layout::emit(const Tech &tech, string libName) const {
	gdstk::Library lib = {};
	lib.init(libName.c_str(), tech.dbunit*1e-6, tech.dbunit*1e-6);

	gdstk::Cell *cell = new gdstk::Cell();
	cell->init(name.c_str());
	for (auto r = geometry.begin(); r != geometry.end(); r++) {
		cell->polygon_array.append(r->emit(tech));
	}

	/*for (int i = 0; i < 2; i++) {
		for (auto r = diff[i].begin(); r != diff[i].end(); r++) {
			cell->polygon_array.append(r->emit(tech));
		}
	}

	for (auto n = nets.begin(); n != nets.end(); n++) {
		for (int c = 0; c < 2; c++) {
			for (auto r = n->contacts[c].begin(); r != n->contacts[c].end(); r++) {
				cell->polygon_array.append(r->emit(tech));
				cell->label_array.append(emitLabel(tech, vec2i((r->left+r->right)/2, (r->bottom+r->top)/2), r->layer, n->name));
			}
		}

		for (auto r = n->routes.begin(); r != n->routes.end(); r++) {
			cell->polygon_array.append(r->emit(tech));
		}
	}*/

	lib.cell_array.append(cell);
	lib.write_gds((libName+".gds").c_str(), 0, NULL);
	lib.free_all();
}
