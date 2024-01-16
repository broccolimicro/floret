#pragma once

#include "Circuit.h"
#include "Tech.h"
#include "Solution.h"

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

struct Layout {
	Layout();
	~Layout();

	string name;
	vector<Rect> geometry;

	void drawTransistor(const Tech &tech, vec2i pos, vec2i dir, int model, const Gate &gate); 
	void drawDiffContact(const Tech &tech, int net, int model, vec2i pos, vec2i dir, int width);
	void drawVia(const Tech &tech, int net, int layer, vec2i pos, vec2i dir, int width, int length);
	void drawRoute(const Tech &tech, const Solution *ckt, int route);
	void drawCell(const Tech &tech, const Solution *ckt);

	gdstk::Label *emitLabel(const Tech &tech, vec2i pos, int layer, string text) const;
	void emit(const Tech &tech, string libName) const;
};
