#pragma once

#include "Circuit.h"
#include "Tech.h"
#include "Solution.h"

#include <iostream>
#include <gdstk/gdstk.hpp>
#include "vector.h"

using namespace std;

struct Layout;

struct Rect {
	Rect();
	Rect(int draw, vec2i ll, vec2i ur);
	Rect(const Routing &layer, int net, vec2i ll, vec2i ur);
	~Rect();

	int draw;
	int text;

	int net;

	int left;
	int bottom;
	int right;
	int top;

	bool merge(Rect r);
	bool hasLabel() const;

	gdstk::Polygon *emit(const Tech &tech) const;
	gdstk::Label *emitLabel(const Tech &tech, const Layout &layout) const;
};

void mergeRects(vector<Rect> &rects);

struct Layout {
	Layout();
	~Layout();

	string name;
	vec2i boxll, boxur;
	vector<string> nets;
	vector<Rect> geometry;

	void updateBox(vec2i ll, vec2i ur);

	void drawTransistor(const Tech &tech, const Mos &mos, vec2i pos=vec2i(0,0), vec2i dir=vec2i(1,1)); 
	void drawVia(const Tech &tech, int net, int downLevel, int upLevel, vec2i size=vec2i(0,0), vec2i pos=vec2i(0,0), vec2i dir=vec2i(1,1));
	void drawWire(const Tech &tech, const Solution *ckt, const Wire &wire, vec2i pos, vec2i dir);
	void drawCell(const Tech &tech, const Solution *ckt);

	void emit(const Tech &tech, gdstk::Library &lib) const;
};
