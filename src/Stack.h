#pragma once

#include <vector>
#include <string>

#include "Term.h"
#include "ColorGraph.h"

using namespace std;

struct Cell;

struct Net
{
	Net();
	~Net();

	string name;

	// the total number of ports for this Net in the cell
	// a port is a connection of the Net to the source, drain, or gate of a transistor
	int ports;
};

struct TermIndex
{
	TermIndex();
	TermIndex(int idx, int flip);
	~TermIndex();

	// the device stack stack index
	int idx;

	// unflipped is source on left, drain on right. Flipped is drain on left,
	// source on right.
	bool flip;
};

struct Column
{
	Column();
	Column(int pos, int net);
	~Column();

	// position in layout of this column (columns aren't uniformly spaced)
	int pos;

	// the net connected to this port
	// this indexes into task->nets or task->stack[i].ovr
	int net;
};

struct OverRoute
{
	OverRoute();
	~OverRoute();

	// the number of ports this net has within this stack
	int gates;
	int links;

	// the latest port id that was routed
	int gateIdx;
	int linkIdx;
};

struct Stack
{
	Stack();
	~Stack();

	vector<Term> mos;
	vector<TermIndex> sel;
	vector<Column> col;
	vector<OverRoute> ovr;
	ColorGraph layer;

	int stage[2];
	int idx[2];
	int flip[2];

	void init(int nets);

	void stash();
	void commit();
	void clear();
	void reset();

	void print(const char *dev);
	void countPorts();
	void collect(Cell *task);
	void stageColumn(int net, bool is_gate);
	int stageStack(int sel, int flip);
};

