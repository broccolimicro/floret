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
	Rect(int layer, int net, vec2i ll, vec2i ur);
	~Rect();

	int layer;
	int net;

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

	void drawTransistor(const Tech &tech, const Mos &mos, vec2i pos=vec2i(0,0), vec2i dir=vec2i(1,1)); 
	/*void drawDiffContact(const Tech &tech, const Pin &pin, vec2i pos, vec2i dir, int width);*/
	void drawVia(const Tech &tech, int net, int idx, vec2i size=vec2i(0,0), vec2i pos=vec2i(0,0), vec2i dir=vec2i(1,1));
	/*void drawRoute(const Tech &tech, const Solution *ckt, int route);*/
	void drawCell(const Tech &tech, const Solution *ckt);

	gdstk::Label *emitLabel(const Tech &tech, vec2i pos, int layer, string text) const;
	void emit(const Tech &tech, string libName) const;
};
