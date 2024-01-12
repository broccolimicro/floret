#pragma once

#include "Cell.h"
#include "Tech.h"
#include <iostream>
#include <gdstk/gdstk.hpp>

using namespace std;

struct Point {
	Point();
	Point(int x, int y);
	~Point();

	int x;
	int y;
};

struct Rect {
	Rect();
	Rect(int layer, int left, int bottom, int right, int top);
	Rect(int layer, Point pos, int width, int height);
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
	vector<Rect> rect;
};

struct Layout {
	Layout();
	~Layout();

	string name;

	vector<Rect> support;
	vector<Rect> diff[2];
	vector<NetLayout> nets; 

	void drawTransistor(const Tech &tech, Point pos, int model, const Gate &gate); 
	Point drawTerm(const Tech &tech, Point pos, const Term &term, bool flip); 
	void drawDiffContact(const Tech &tech, int net, int model, Point pos, int width, int flip);
	void drawStack(const Tech &tech, Point pos, const Stack &stack);
	void drawCell(const Tech &tech, Point pos, const Cell &cell);

	void cleanup();

	gdstk::Label *emitLabel(const Tech &tech, Point pos, int layer, string text) const;
	void emit(const Tech &tech, string libName) const;
};
