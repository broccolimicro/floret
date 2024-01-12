#include "Layout.h"

Point::Point() {
	x = 0;
	y = 0;
}

Point::Point(int x, int y) {
	this->x = x;
	this->y = y;
}

Point::~Point() {
}

Rect::Rect() {
	layer = -1;
	left = 0;
	bottom = 0;
	right = 0;
	top = 0;
}

Rect::Rect(int layer, int left, int bottom, int right, int top) {
	this->layer = layer;
	this->left = left;
	this->bottom = bottom;
	this->right = right;
	this->top = top;

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

Rect::Rect(int layer, Point pos, int width, int height) {
	this->layer = layer;
	this->left = pos.x;
	this->bottom = pos.y;
	this->right = pos.x+width;
	this->top = pos.y+height;

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
	if (layer == r.layer and left == r.left and right == r.right and bottom <= r.top and top >= r.bottom) {
		if (r.bottom < bottom) {
			bottom = r.bottom;
		}
		if (r.top > top) {
			top = r.top;
		}
		return true;
	} else if (layer == r.layer and bottom == r.bottom and top == r.top and left <= r.right and right >= r.left) {
		if (r.left < left) {
			left = r.left;
		}
		if (r.right > right) {
			right = r.right;
		}
		return true;
	} else if (layer == r.layer and bottom <= r.bottom and top >= r.top and left <= r.left and right >= r.right) {
		return true;
	} else if (layer == r.layer and bottom >= r.bottom and top <= r.top and left >= r.left and right <= r.right) {
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

NetLayout::NetLayout() {
}

NetLayout::NetLayout(string name) {
	this->name = name;
}

NetLayout::~NetLayout() {
}

Layout::Layout() {
}

Layout::~Layout() {
}

void Layout::drawTransistor(const Tech &tech, Point pos, int model, const Gate &gate) {
	int flip = 1;
	int type = tech.models[model].type;
	if (type == Model::NMOS) {
		flip = -1;
	}

	pos.y *= flip;

	int polyOverhang = tech.models[model].polyOverhang;
	
	// draw poly
	nets[gate.net].rect.push_back(Rect(tech.wires[0].drawingLayer, Point(pos.x, pos.y - polyOverhang*flip), gate.length, (gate.width + 2*polyOverhang)*flip));
	
	// draw diffusion
	int length = gate.length;
	int width = gate.width*flip;
	for (auto layer = tech.models[model].layers.begin(); layer != tech.models[model].layers.end(); layer++) {
		pos.x -= layer->overhangX;
		pos.y -= layer->overhangY*flip;
		length += 2*layer->overhangX;
		width += 2*layer->overhangY*flip;
		diff[type].push_back(Rect(layer->layer, pos, length, width));
	}
}

Point Layout::drawTerm(const Tech &tech, Point pos, const Term &term, bool flip) {
	int start = flip ? (int)term.gate.size()-1 : 0;
	int end = (flip ? -1 : (int)term.gate.size());
	int step = flip ? -1 : 1;

	for (int i = start; i != end; i += step) {
		// draw transistor
		drawTransistor(tech, pos, term.model, term.gate[i]);
		
		if (i+step != end) {		
			pos.x += term.gate[i].length + tech.layers[tech.wires[0].drawingLayer].minSpacing;
		}
	}

	return pos;
}

void Layout::drawDiffContact(const Tech &tech, int net, int model, Point pos, int width, int flip) {
	pos.y *= flip;

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
	Point diffPos(pos.x-diffEncloseH, pos.y);
	int diffLength = viaWidth + 2*diffEncloseH;
	int diffWidth = width*flip;
	for (auto layer = tech.models[model].layers.begin(); layer != tech.models[model].layers.end(); layer++) {
		if (layer != tech.models[model].layers.begin()) {
			diffPos.x -= layer->overhangX;
			diffPos.y -= layer->overhangY*flip;
			diffLength += 2*layer->overhangX;
			diffWidth += 2*layer->overhangY*flip;
		}
		diff[type].push_back(Rect(layer->layer, diffPos, diffLength, diffWidth));
	}

	pos.y += flip*contactOffset;
	if (flip < 0) {
		pos.y += flip*contactWidth;
	}

	nets[net].rect.push_back(Rect(tech.wires[1].drawingLayer, pos, viaWidth, contactWidth));

	for (int i = 0; i < numVias; i++) {
		nets[net].rect.push_back(Rect(via, pos, viaWidth, viaWidth));

		pos.y += viaWidth+viaSpacing;
	}
}

void Layout::drawStack(const Tech &tech, Point pos, const Stack &stack) {
	if (stack.sel.size() == 0) {
		return;
	}

	int via = tech.vias[0].drawingLayer; 
	int viaWidth = tech.layers[via].minWidth;

	// draw li contacts
		
	// draw li

	int model = stack.mos[stack.sel[0].idx].model;
	int coeff = tech.models[model].type == Model::NMOS ? -1 : 1;
	int viaPolySpacing = tech.models[model].viaPolySpacing;

	int net = stack.mos[stack.sel[0].idx].source;
	int width = stack.mos[stack.sel[0].idx].gate[0].width;
	int length = stack.mos[stack.sel[0].idx].gate[0].length;
	if (stack.sel[0].flip) {
		net = stack.mos[stack.sel[0].idx].drain;
		width = stack.mos[stack.sel[0].idx].gate.back().width;
		length = stack.mos[stack.sel[0].idx].gate.back().length;
	}

	drawDiffContact(tech, net, model, Point(pos.x - viaWidth - viaPolySpacing, pos.y), width, coeff);

	// draw transistors
	for (auto i = stack.sel.begin(); i != stack.sel.end(); i++) {
		model = stack.mos[i->idx].model;
		coeff = tech.models[model].type == Model::NMOS ? -1 : 1;

		net = stack.mos[i->idx].drain;
		width = stack.mos[i->idx].gate.back().width;
		length = stack.mos[i->idx].gate.back().length;
		if (i->flip) {
			net = stack.mos[i->idx].source;
			width = stack.mos[i->idx].gate[0].width;
			length = stack.mos[i->idx].gate[0].length;
		}

		bool connect = false;

		int contactWidth = width;
		auto j = i+1;
		if (j != stack.sel.end()) {
			int net2 = stack.mos[j->idx].source;
			int width2 = stack.mos[j->idx].gate[0].width;
			if (j->flip) {
				net2 = stack.mos[j->idx].drain;
				width2 = stack.mos[j->idx].gate.back().width;
			}

			if (net2 == net) {
				connect = true;
				if (width2 < contactWidth) {
					contactWidth = width2;
				}
			}
		}

		pos = drawTerm(tech, pos, stack.mos[i->idx], i->flip);

		drawDiffContact(tech, net, model, Point(pos.x + viaPolySpacing + length, pos.y), contactWidth, coeff);

		pos.x += 2*viaPolySpacing + viaWidth + length;
	}

	// draw overcell routing
}

void Layout::drawCell(const Tech &tech, Point pos, const Cell &cell) {
	name = cell.name;

	for (int i = 0; i < (int)cell.nets.size(); i++) {
		nets.push_back(NetLayout(cell.nets[i].name));
	}

	// draw pull-up
	drawStack(tech, Point(0, 100), cell.stack[Model::PMOS]);

	// draw pull-down
	drawStack(tech, Point(0, 100), cell.stack[Model::NMOS]);

	// draw channel routing

	// draw boundary
}

void Layout::cleanup() {
	mergeRects(support);
	mergeRects(diff[0]);
	mergeRects(diff[1]);
	for (int i = 0; i < (int)nets.size(); i++) {
		mergeRects(nets[i].rect);
	}
}

gdstk::Label *Layout::emitLabel(const Tech &tech, Point pos, int layer, string text) const {
	return new gdstk::Label{
		.tag = gdstk::make_tag(tech.layers[layer].major, tech.layers[layer].minor),
		.text = strdup(text.c_str()),
		.origin = gdstk::Vec2{(double)pos.x, (double)pos.y},
		.magnification = 1,
	};
}

void Layout::emit(const Tech &tech, string libName) const {
	gdstk::Library lib = {};
	lib.init(libName.c_str(), tech.dbunit*1e-6, tech.dbunit*1e-6);

	gdstk::Cell *cell = new gdstk::Cell();
	cell->init(name.c_str());
	for (auto r = support.begin(); r != support.end(); r++) {
		cell->polygon_array.append(r->emit(tech));
	}

	for (int i = 0; i < 2; i++) {
		for (auto r = diff[i].begin(); r != diff[i].end(); r++) {
			cell->polygon_array.append(r->emit(tech));
		}
	}

	for (auto n = nets.begin(); n != nets.end(); n++) {
		for (auto r = n->rect.begin(); r != n->rect.end(); r++) {
			cell->polygon_array.append(r->emit(tech));
			cell->label_array.append(emitLabel(tech, Point((r->left+r->right)/2, (r->bottom+r->top)/2), r->layer, n->name));
		}
	}

	lib.cell_array.append(cell);
	lib.write_gds((libName+".gds").c_str(), 0, NULL);
	lib.free_all();
}
