#pragma once

#include "Cell.h"
#include "Tech.h"
#include <iostream>
#include <gdstk/gdstk.hpp>
#include "vector.h"

using namespace std;

struct Rect {
	Rect();
	Rect(int layer, int left, int bottom, int right, int top);
	Rect(int layer, vec2i pos, int width, int height);
	~Rect();

	int layer;

	int left;
	int bottom;
	int right;
	int top;

	bool merge(Rect r);

	gdstk::Polygon *emit(const Tech &tech) const;
};

void mergeRects(vector<Rect> &rects);

struct NetLayout {
	NetLayout();
	NetLayout(string name);
	~NetLayout();

	string name;
	// These are the vias, overcell, and channel routing geometry
	vector<Rect> routes;

	// this is where overcell and channel routing hook up to the nets
	vector<Rect> contacts[2];
};

struct Layout {
	Layout();
	~Layout();

	string name;

	vector<Rect> support;
	vector<Rect> diff[2];
	vector<NetLayout> nets;

	// offset from bottom of transistor
	vector<int> overRoutes[2];

	void drawTransistor(const Tech &tech, vec2i pos, vec2i dir, int model, const Gate &gate); 
	vec2i drawTerm(const Tech &tech, vec2i pos, vec2i dir, const Term &term, bool flip); 
	void drawDiffContact(const Tech &tech, int net, int model, vec2i pos, vec2i dir, int width);
	void drawVia(const Tech &tech, int net, int layer, vec2i pos, vec2i dir, int width, int length);
	void drawStack(const Tech &tech, vec2i pos, vec2i dir, const Stack &stack);
	void drawCell(const Tech &tech, vec2i pos, const Cell &cell);

	void cleanup();

	gdstk::Label *emitLabel(const Tech &tech, vec2i pos, int layer, string text) const;
	void emit(const Tech &tech, string libName) const;
};
