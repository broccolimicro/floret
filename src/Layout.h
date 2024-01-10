#pragma once

#include <vector>
#include <string>
#include "Rect.h"
#include "Stack.h"

using namespace std;

struct Route
{
	vector<int> assign;
};

struct Layout
{
	Layout();
	~Layout();

	Stack stack[2];
	vector<Net> nets;

	vector<Route> cols;
	int stage[2];

	vector<Rect> geo;
	
	void stash();
	void commit();
	void clear();
	void reset();
	void stageChannel();

	void collectStacks();
	void orderStacks();
	void routeChannel();

	
};

void processCell();

