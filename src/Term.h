#pragma once

#include <vector>

using namespace std;

struct Gate
{
	Gate();
	Gate(int net, int width, int length);
	~Gate();

	// the id of the net for this gate
	// this indexes into task->nets or task->stack[i].ovr
	int net;

	// transistor dimensions
	int width;
	int length;
};

struct Term
{
	Term(int gate, int source, int drain, int bulk, int width, int length);
	~Term();

	vector<Gate> gate;
	// the id of the net for this stack
	// this indexes into task->nets or task->stack[i].ovr
	int source;
	int drain;
	int bulk;

	// whether or not this stack has been placed in the layout problem
	bool selected;
};
