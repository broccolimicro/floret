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
}

Rect::Rect(int layer, Point pos, int width, int height) {
	this->layer = layer;
	this->left = pos.x;
	this->bottom = pos.y;
	this->right = pos.x+width;
	this->top = pos.y+height;
}

Rect::~Rect() {
}

gdstk::Polygon *Rect::emit(const Tech &tech) const {
	return new gdstk::Polygon(gdstk::rectangle(gdstk::Vec2{(double)left, (double)bottom}, gdstk::Vec2{(double)right, (double)top}, gdstk::make_tag(tech.layers[layer].major, tech.layers[layer].minor)));
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
	
	// draw poly
	nets[gate.net].rect.push_back(Rect(tech.wires[0].drawingLayer, pos, gate.length, gate.width*flip));
	
	// draw diffusion
	int length = gate.length;
	for (auto layer = tech.models[model].layers.begin(); layer != tech.models[model].layers.end(); layer++) {
		pos.x -= layer->overhang;
		length += 2*layer->overhang;
		diff[type].push_back(Rect(layer->layer, pos, length, gate.width*flip));
	}
}

Point Layout::drawTerm(const Tech &tech, Point pos, const Term &term, bool flip) {
	for (int i = flip ? (int)term.gate.size()-1 : 0; i != (flip ? -1 : (int)term.gate.size()); i += flip ? -1 : 1) {
		// draw transistor
		drawTransistor(tech, pos, term.model, term.gate[i]);
		 
		// draw li contacts
		
		
		// draw li
		
		
		pos.x += term.gate[i].length + tech.layers[tech.wires[0].drawingLayer].minSpacing;
	}
	return pos;
}

void Layout::drawStack(const Tech &tech, Point pos, const Stack &stack) {
	// draw transistors
	for (auto i = stack.sel.begin(); i != stack.sel.end(); i++) {
		pos = drawTerm(tech, pos, stack.mos[i->idx], i->flip);
	}

	// draw overcell routing
}

void Layout::drawCell(const Tech &tech, Point pos, const Cell &cell) {
	name = cell.name;

	for (int i = 0; i < (int)cell.nets.size(); i++) {
		nets.push_back(NetLayout(cell.nets[i].name));
	}

	// draw pull-up
	drawStack(tech, Point(0, 30), cell.stack[Model::PMOS]);

	// draw pull-down
	drawStack(tech, Point(0, 30), cell.stack[Model::NMOS]);

	// draw channel routing

	// draw boundary
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
		}
	}

	lib.cell_array.append(cell);
	lib.write_gds((libName+".gds").c_str(), 0, NULL);
	lib.free_all();
}
