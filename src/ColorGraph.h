#pragma once

#include <vector>
#include <cstdint>

using namespace std;

struct ColorEdge {
	ColorEdge();
	ColorEdge(int x, int y);
	~ColorEdge();

	int x;
	int y;
};

struct ColorGraph {
	ColorGraph(int count = 0);
	~ColorGraph();

	vector<ColorEdge> edges;
	vector<int32_t> color;
	int stage[2];

	void init(int count = 0);

	bool hasEdge(int a, int b);
	void pushEdge(int a, int b);

	int stashCost();
	int stageCost();

	void stash();
	void commit();
	void clear();
	void reset();
};

