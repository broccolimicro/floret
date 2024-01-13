#include "Layout.h"

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

Rect::Rect(int layer, vec2i pos, int width, int height) {
	this->layer = layer;
	this->left = pos[0];
	this->bottom = pos[1];
	this->right = pos[0]+width;
	this->top = pos[1]+height;

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

void Layout::drawTransistor(const Tech &tech, vec2i pos, vec2i dir, int model, const Gate &gate) {
	int type = tech.models[model].type;
	int polyOverhang = tech.models[model].polyOverhang;
	
	// draw poly
	nets[gate.net].contacts[type].push_back(Rect(tech.wires[0].drawingLayer, vec2i(pos[0], pos[1] - polyOverhang*dir[1]), gate.length*dir[0], (gate.width + 2*polyOverhang)*dir[1]));
	
	// draw diffusion
	int length = gate.length*dir[0];
	int width = gate.width*dir[1];
	for (auto layer = tech.models[model].layers.begin(); layer != tech.models[model].layers.end(); layer++) {
		pos[0] -= layer->overhangX*dir[0];
		pos[1] -= layer->overhangY*dir[1];
		length += 2*layer->overhangX*dir[0];
		width += 2*layer->overhangY*dir[1];
		diff[type].push_back(Rect(layer->layer, pos, length, width));
	}
}

vec2i Layout::drawTerm(const Tech &tech, vec2i pos, vec2i dir, const Term &term, bool flip) {
	int start = flip ? (int)term.gate.size()-1 : 0;
	int end = (flip ? -1 : (int)term.gate.size());
	int step = flip ? -1 : 1;

	for (int i = start; i != end; i += step) {
		// draw transistor
		drawTransistor(tech, pos, dir, term.model, term.gate[i]);
		
		if (i+step != end) {		
			pos[0] += (term.gate[i].length + tech.layers[tech.wires[0].drawingLayer].minSpacing)*dir[0];
		}
	}

	return pos;
}

void Layout::drawDiffContact(const Tech &tech, int net, int model, vec2i pos, vec2i dir, int width) {
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
}

void Layout::drawVia(const Tech &tech, int net, int layer, vec2i pos, vec2i dir, int sizeX, int sizeY) {
	int via = tech.vias[layer].drawingLayer;
	int down = tech.vias[layer].from;
	int up = tech.vias[layer].to;
	int viaWidth = tech.layers[via].minWidth;
	int viaSpacing = tech.layers[via].minSpacing;
	int dnLo = tech.vias[layer].downLo;
	int dnHi = tech.vias[layer].downHi;
	int upLo = tech.vias[layer].upLo;
	int upHi = tech.vias[layer].upHi;

	int numX = 1 + (sizeX-viaWidth - 2*dnLo) / (viaSpacing + viaWidth);
	int numY = 1 + (sizeY-viaWidth - 2*dnLo) / (viaSpacing+viaWidth);

	int widthX = numX*viaWidth + (numX-1)*viaSpacing;
	int widthY = numY*viaWidth + (numY-1)*viaSpacing;

	int offX = 0;
	int offY = 0;
	if (sizeX > widthX) {
		offX = (sizeX - widthX)/2;
	}
	if (sizeY > widthY) {
		offY = (sizeY - widthY)/2;
	}

	int dnH = dnHi;
	int dnV = dnLo;
	if (offX >= dnHi) {
		dnH = dnLo;
		dnV = dnHi;
	}

	int upH = upHi;
	int upV = upLo;
	if (offX >= upHi) {
		upH = upLo;
		upV = upHi;
	}

	if (offY < dnV) {
		offY = dnV;
	}
	if (offY < upV) {
		offY = upV;
	}

	// draw down
	nets[net].routes.push_back(Rect(down, vec2i(pos[0]+(offX-dnH)*dir[0], pos[1]+(offY-dnV)*dir[1]), (offX+widthX+dnH)*dir[0], (offY+widthY+dnV)*dir[1]));

	// draw via
	for (int x = 0; x < numX; x++) {
		for (int y = 0; y < numY; y++) {
			nets[net].routes.push_back(Rect(via, vec2i(pos[0]+(offX+x*(viaWidth+viaSpacing))*dir[0], pos[1]+(offY+y*(viaWidth+viaSpacing))*dir[1]), viaWidth*dir[0], viaWidth*dir[1]));
		}
	}

	// draw up
	nets[net].routes.push_back(Rect(up, vec2i(pos[0]+(offX-upH)*dir[0], pos[1]+(offY-upV)*dir[1]), (upH*2+widthX)*dir[0], (upV*2+widthY)*dir[1]));
}

void Layout::drawStack(const Tech &tech, vec2i pos, vec2i dir, const Stack &stack) {
	if (stack.sel.size() == 0) {
		return;
	}

	int via = tech.vias[0].drawingLayer; 
	int viaWidth = tech.layers[via].minWidth;

	// draw li contacts
		
	// draw li

	int model = stack.mos[stack.sel[0].idx].model;
	int type = tech.models[model].type;
	int viaPolySpacing = tech.models[model].viaPolySpacing;

	int net = stack.mos[stack.sel[0].idx].source;
	int width = stack.mos[stack.sel[0].idx].gate[0].width;
	int length = stack.mos[stack.sel[0].idx].gate[0].length;
	if (stack.sel[0].flip) {
		net = stack.mos[stack.sel[0].idx].drain;
		width = stack.mos[stack.sel[0].idx].gate.back().width;
		length = stack.mos[stack.sel[0].idx].gate.back().length;
	}

	drawDiffContact(tech, net, model, vec2i(pos[0] - (viaWidth+viaPolySpacing)*dir[0], pos[1]), dir, width);

	// draw transistors
	for (auto i = stack.sel.begin(); i != stack.sel.end(); i++) {
		model = stack.mos[i->idx].model;
		type = tech.models[model].type;

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

		pos = drawTerm(tech, pos, dir, stack.mos[i->idx], i->flip);

		drawDiffContact(tech, net, model, vec2i(pos[0] + (viaPolySpacing+length)*dir[0], pos[1]), dir, contactWidth);

		pos[0] += (2*viaPolySpacing + viaWidth + length)*dir[0];
	}

	// draw overcell routing
	int poly = tech.wires[0].drawingLayer;
	int polyWidth = tech.layers[poly].minWidth;
	int polySpacing = tech.layers[poly].minSpacing;
	int polyvia = tech.vias[1].drawingLayer;
	int polyviaWidth = tech.layers[polyvia].minWidth;
	int m1 = tech.wires[1].drawingLayer;	
	int m1Width = tech.layers[m1].minWidth;
	int m1Spacing = tech.layers[m1].minSpacing;
	int m1via = tech.vias[2].drawingLayer;
	int m1viaWidth = tech.layers[m1via].minWidth;
	int m2 = tech.wires[2].drawingLayer;
	int m2Width = tech.layers[m2].minWidth;
	int m2Spacing = tech.layers[m2].minSpacing;

	vector<int> nonPolyColors;
	vector<int> polyColors;
	int maxColor = 0;
	for (int net = 0; net < (int)stack.layer.color.size(); net++) {
		if (stack.layer.color[net] > maxColor) {
			maxColor = stack.layer.color[net];
		}
	}

	vector<int> colors;
	int maxOffset = pos[1];
	for (int color = 0; color <= maxColor; color++) {
		for (int net = 0; net < (int)nets.size(); net++) {
			if (stack.layer.color[net] == color) {
				while (color >= (int)colors.size()) {
					colors.push_back(maxOffset);
				}

				int left = 0;
				int right = 0;
				for (auto c = nets[net].contacts[type].begin(); c != nets[net].contacts[type].end(); c++) {
					if (left == right) {
						left = c->left;
						right = c->right;
					} else {
						if (c->left < left) {
							left = c->left;
						}
						if (c->right > right) {
							right = c->right;
						}
					}

					int n1 = colors[color];
					int n2 = n1+m2Width*dir[1];
					int bottom = c->bottom;
					int top = c->top;
					if (n1 < bottom) {
						bottom = n1;
					}
					if (n2 < bottom) {
						bottom = n2;
					}
					if (n1 > top) {
						top = n1;
					}
					if (n2 > top) {
						top = n2;
					}
					
					nets[net].routes.push_back(Rect(c->layer, c->left, bottom, c->right, top));
					drawVia(tech, net, 2, vec2i(c->left, colors[color]), dir, c->right-c->left, 0);
				}	

				nets[net].routes.push_back(Rect(m2, left, colors[color], right, colors[color]+m2Width*dir[1]));
				for (auto r = nets[net].routes.begin(); r != nets[net].routes.end(); r++) {
					if (r->layer == m2 and (r->top+m2Spacing*dir[1])*dir[1] > maxOffset*dir[1]) {
						maxOffset = r->top+m2Spacing*dir[1];
					}
					if (r->layer == m2 and (r->bottom+m2Spacing*dir[1])*dir[1] > maxOffset*dir[1]) {
						maxOffset = r->bottom+m2Spacing*dir[1];
					}
				} 
			}
		}
	}
}

void Layout::drawCell(const Tech &tech, vec2i pos, const Cell &cell) {
	name = cell.name;

	for (int i = 0; i < (int)cell.nets.size(); i++) {
		nets.push_back(NetLayout(cell.nets[i].name));
	}

	// draw pull-up
	drawStack(tech, vec2i(0, 100), vec2i(1, 1), cell.stack[Model::PMOS]);

	// draw pull-down
	drawStack(tech, vec2i(0, -100), vec2i(1, -1), cell.stack[Model::NMOS]);

	// draw channel routing

	// draw boundary
}

void Layout::cleanup() {
	mergeRects(support);
	mergeRects(diff[0]);
	mergeRects(diff[1]);
	for (int i = 0; i < (int)nets.size(); i++) {
		mergeRects(nets[i].routes);
		for (int j = 0; j < 2; j++) {
			mergeRects(nets[i].contacts[j]);
		}
	}
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
	for (auto r = support.begin(); r != support.end(); r++) {
		cell->polygon_array.append(r->emit(tech));
	}

	for (int i = 0; i < 2; i++) {
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
	}

	lib.cell_array.append(cell);
	lib.write_gds((libName+".gds").c_str(), 0, NULL);
	lib.free_all();
}
