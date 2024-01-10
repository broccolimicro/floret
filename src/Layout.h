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
#include "Rect.h"
#include "Stack.h"

using namespace std;

struct Route
{
	vector<int> assign;
};

struct LayoutTask
{
	LayoutTask();
	~LayoutTask();

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

