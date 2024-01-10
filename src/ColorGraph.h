#pragma once

#include <vector>
#include <cstdint>

using namespace std;

struct ColorEdge {
	ColorEdge();
	ColorEdge(uint32_t x, uint32_t y);
	~ColorEdge();

	uint32_t x;
	uint32_t y;
};

struct ColorGraph {
	ColorGraph(uint32_t count = 0);
	~ColorGraph();

	vector<ColorEdge> edges;
	vector<int32_t> color;
	uint32_t stage[2];

	void init(uint32_t count = 0);

	bool hasEdge(uint32_t a, uint32_t b);
	void pushEdge(uint32_t a, uint32_t b);

	uint32_t stashCost();
	uint32_t stageCost();

	void stash();
	void commit();
	void clear();
	void reset();
};

