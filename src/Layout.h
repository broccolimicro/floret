/*************************************************************************
 *
 *  This file is part of the ACT library
 *
 *  Copyright (c) 2018-2019 Rajit Manohar
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *
 **************************************************************************
 */

#pragma once

#include <vector>
#include <string>
#include "ColorGraph.h"

using namespace std;

struct layout_task;

struct Net
{
	Net();
	~Net();

	string name;

	// the total number of ports for this Net in the cell
	// a port is a connection of the Net to the source, drain, or gate of a transistor
	int ports;
};

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

struct Device
{
	Device(int gate, int source, int drain, int bulk, int width, int length);
	~Device();

	vector<Gate> gate;
	// the id of the net for this stack
	// this indexes into task->nets or task->stack[i].ovr
	int source;
	int drain;
	int bulk;

	// whether or not this stack has been placed in the layout problem
	bool selected;
};

struct DeviceLayout
{
	DeviceLayout();
	DeviceLayout(int idx, int flip);
	~DeviceLayout();

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

struct Rect
{
	int left;
	int right;
	int bottom;
	int top;
	int layer;
};


struct OverRoute
{
	OverRoute();
	~OverRoute();

	// the number of ports this net has within this stack
	int gates;
	int links;

	// the latest port id that was routed
	int gate_idx;
	int link_idx;
};

struct Stack
{
	Stack();
	~Stack();

	vector<Device> mos;
	vector<DeviceLayout> sel;
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
	void count_ports();
	void collect(layout_task *task);
	void stage_col(int net, bool is_gate);
	int stage_stack(int sel, int flip);
};

struct Route
{
	vector<int> assign;
};

struct layout_task
{
	layout_task();
	~layout_task();

	Stack stack[2];
	vector<Net> nets;

	vector<Route> cols;
	int stage[2];

	vector<Rect> geo;
	
	void stash();
	void commit();
	void clear();
	void reset();
	void stage_channel();
};

void collect_stacks(layout_task *task);
void process_cell();

