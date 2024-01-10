#include "ColorGraph.h"

#include <vector>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

ColorEdge::ColorEdge() {
	x = 0;
	y = 0;
}

ColorEdge::ColorEdge(uint32_t x, uint32_t y) {
	this->x = x;
	this->y = y;
}

ColorEdge::~ColorEdge() {
}

ColorGraph::ColorGraph(uint32_t count) {
	if (count > 0) {
		color.resize(count, -1);
	}

	stage[0] = 0;
	stage[1] = 0;
}

ColorGraph::~ColorGraph() {
}

void ColorGraph::init(uint32_t count) {
	if (count > color.size()) {
		color.resize(count, -1);
	}
}

bool ColorGraph::hasEdge(uint32_t a, uint32_t b) {
	for (uint32_t i = 0; i < stage[0]; i++) {
		if ((edges[i].x == a and edges[i].y == b) or (edges[i].x == b and edges[i].y == a)) {
			return true;
		}
	}
	for (uint32_t i = stage[1]; i < edges.size(); i++) {
		if ((edges[i].x == a and edges[i].y == b) or (edges[i].x == b and edges[i].y == a)) {
			return true;
		}
	}

	return false;
}

void ColorGraph::pushEdge(uint32_t a, uint32_t b)
{
	edges.push_back(ColorEdge(a, b));
}

uint32_t ColorGraph::stashCost()
{
	return edges.size() - stage[1];
}

uint32_t ColorGraph::stageCost()
{
	return stage[1] - stage[0];
}

void ColorGraph::stash()
{
	edges.resize(stage[0]);
	stage[1] = edges.size();
}

void ColorGraph::commit()
{
	edges.resize(stage[1]);

	uint32_t len = color.size()-1;
	for (uint32_t i = stage[0]; i < edges.size(); i++) {
		if (edges[i].x > len) {
			len = edges[i].x;
		}
		if (edges[i].y > len) {
			len = edges[i].y;
		}
	}
	len = len + 1 - color.size();
	
	if (len > 0) {
		// this should never happen if this structure is properly initialized
		color.resize(color.size()+len, -1);
	}

	printf("Edges:\n");
	for (uint32_t j = 0; j < edges.size(); j++) {
		printf("%d -> %d\n", edges[j].x, edges[j].y);
	}

	for (uint32_t j = stage[0]; j < edges.size(); j++) {
		uint32_t a = edges[j].x;
		uint32_t b = edges[j].y;

		if (color[a] < 0) {
			color[a] = 0;
		}
		if (color[b] < 0) {
			color[b] = 0;
		}

		if (color[a] == color[b]) {
			// vector<bool> is specialized, implementing a dynamic bitset in c++.
			vector<bool> s(color.size(), false);

			//printf("checking colors for %d\n", b);
			for (uint32_t i = 0; i <= j; i++) {
				if (edges[i].x == b) {
					printf("%u/%lu: %d/%lu\n", edges[i].y, color.size(), color[edges[i].y], s.size());
					s[color[edges[i].y]] = true;
				} else if (edges[i].y == b) {
					printf("%u/%lu: %d/%lu\n", edges[i].x, color.size(), color[edges[i].x], s.size());
					s[color[edges[i].x]] = true;
				}
			}
			
			for (uint32_t i = 0; i < color.size(); i++) {
				if (not s[i]) {
					color[b] = i;
					break;
				}
			}
		}
	}

	stage[0] = edges.size();
}

void ColorGraph::clear()
{
	edges.resize(stage[1]);
}

void ColorGraph::reset()
{
	edges.resize(stage[0]);
	stage[1] = stage[0];
}

