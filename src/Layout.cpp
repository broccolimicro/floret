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
#include <stdio.h>
#include <string.h>
#include <set>
#include <map>
#include <utility>
#include <stdint.h>
#include "Layout.h"

Net::Net()
{
	ports = 0;
}

Net::~Net()
{
}

Gate::Gate()
{
}

Gate::Gate(int net, int width, int length)
{
	this->net = net;
	this->width = width;
	this->length = length;
}

Gate::~Gate()
{
}


Device::Device(int gate, int source, int drain, int bulk, int width, int length)
{
	this->gate.push_back(Gate(gate, width, length));
	this->source = source;
	this->drain = drain;
	this->bulk = bulk;

	this->selected = 0;
}

Device::~Device()
{
}

DeviceLayout::DeviceLayout()
{
	idx = 0;
	flip = 0;
}

DeviceLayout::DeviceLayout(int idx, int flip)
{
	this->idx = idx;
	this->flip = flip;
}

DeviceLayout::~DeviceLayout()
{
}

Column::Column()
{
	this->pos = 0;
	this->net = 0;
}

Column::Column(int pos, int net)
{
	this->pos = pos;
	this->net = net;
}

Column::~Column()
{
}


OverRoute::OverRoute()
{
	gates = 0;
	links = 0;
	gate_idx = -1;
	link_idx = -1;
}

OverRoute::~OverRoute()
{
}

Stack::Stack()
{
	stage[0] = 0;
	stage[1] = 0;
}

Stack::~Stack()
{
}

void Stack::init(int nets)
{
	ovr.resize(nets);
	layer.init(nets);
}

void Stack::stash()
{
	//printf("STASH\n");
	if (stage[1] < (int)col.size()) {
		ovr[mos[idx[1]].source].link_idx -= 1;
		ovr[mos[idx[1]].drain].link_idx -= 1;
		for (int i = 0; i < (int)mos[idx[1]].gate.size(); i++) {
			ovr[mos[idx[1]].gate[i].net].gate_idx -= 1;
		}
	}

	col.erase(col.begin()+stage[0], col.begin()+stage[1]);
	stage[1] = col.size();
	layer.stash();
	idx[0] = idx[1];
	flip[0] = flip[1];
}

void Stack::commit()
{
	//printf("COMMIT\n");
	if (stage[1] < (int)col.size()) {
		ovr[mos[idx[1]].source].link_idx -= 1;
		ovr[mos[idx[1]].drain].link_idx -= 1;
		for (int i = 0; i < (int)mos[idx[1]].gate.size(); i++) {
			ovr[mos[idx[1]].gate[i].net].gate_idx -= 1;
		}
	}

	if (stage[0] < stage[1]) {
		ovr[mos[idx[0]].source].link_idx += 1;
		ovr[mos[idx[0]].drain].link_idx += 1;
		for (int i = 0; i < (int)mos[idx[0]].gate.size(); i++) {
			ovr[mos[idx[0]].gate[i].net].gate_idx += 1;
		}
	}

	col.erase(col.begin()+stage[1], col.end());
	stage[0] = col.size();
	stage[1] = col.size();
	layer.commit();
	sel.push_back(DeviceLayout(idx[0], flip[0]));
	mos[idx[0]].selected = 1;
}

void Stack::clear()
{
	//printf("CLEAR\n");
	if (stage[1] < (int)col.size()) {
		ovr[mos[idx[1]].source].link_idx -= 1;
		ovr[mos[idx[1]].drain].link_idx -= 1;
		for (int i = 0; i < (int)mos[idx[1]].gate.size(); i++) {
			ovr[mos[idx[1]].gate[i].net].gate_idx -= 1;
		}
	}

	col.resize(stage[1]);
	layer.clear();
}

void Stack::reset()
{
	if (stage[1] < (int)col.size()) {
		ovr[mos[idx[1]].source].link_idx -= 1;
		ovr[mos[idx[1]].drain].link_idx -= 1;
		for (int i = 0; i < (int)mos[idx[1]].gate.size(); i++) {
			ovr[mos[idx[1]].gate[i].net].gate_idx -= 1;
		}
	}

	col.resize(stage[0]);
	stage[1] = stage[0];
	layer.reset();
}

void Stack::print(const char *dev)
{
	for (int i = 0; i < (int)sel.size(); i++) {
		printf("%s b:%d idx:%d flip:%d\n", dev, mos[sel[i].idx].bulk, sel[i].idx, sel[i].flip);
		if (not sel[i].flip) {
			printf("  s:%d\n", mos[sel[i].idx].source);
			for (int j = 0; j < (int)mos[sel[i].idx].gate.size(); j++)
				printf("  g:%d w:%d l:%d\n",
				  mos[sel[i].idx].gate[j].net,
				  mos[sel[i].idx].gate[j].width,
				  mos[sel[i].idx].gate[j].length);
			printf("  d:%d\n", mos[sel[i].idx].drain);
		} else {
			printf("  d:%d\n", mos[sel[i].idx].drain);
			for (int j = (int)mos[sel[i].idx].gate.size()-1; j >= 0; j--)
				printf("  g:%d w:%d l:%d\n",
				  mos[sel[i].idx].gate[j].net,
				  mos[sel[i].idx].gate[j].width,
				  mos[sel[i].idx].gate[j].length);
			printf("  s:%d\n", mos[sel[i].idx].source);
		}
	}

	for (int i = 0; i < (int)col.size(); i++) {
		printf("%d: net:%d layer:%d\n", i, col[i].net, layer.color[col[i].net]);
	}
}

void Stack::countPorts()
{
	for (int i = 0; i < (int)mos.size(); i++) {
		ovr[mos[i].source].links+= 1;
		ovr[mos[i].drain].links += 1;
		for (int j = 0; j < (int)mos[i].gate.size(); j++) {
			ovr[mos[i].gate[j].net].gates += 1;
		}
	}
}

void Stack::collect(LayoutTask *task)
{
	for (int i = 0; i < (int)mos.size(); i++) {
		for (int j = mos.size()-1; j > i; j--) {
			if (mos[i].drain == mos[j].source and task->nets[mos[i].drain].ports == 2) {
				mos[i].gate.insert(mos[i].gate.end(), mos[j].gate.begin(), mos[j].gate.end());
				mos[i].drain = mos[j].drain;
				mos.erase(mos.begin()+j);
			} else if (mos[i].source == mos[j].drain and task->nets[mos[i].source].ports == 2) {
				mos[j].gate.insert(mos[j].gate.end(), mos[i].gate.begin(), mos[i].gate.end());
				mos[i].gate = mos[j].gate;
				mos[i].source = mos[j].source;
				mos.erase(mos.begin()+j);
			}
		}
	}
}

void Stack::stageColumn(int net, bool is_gate)
{
	//printf("Staging MOS %d: %d,%d\n", net, ovr[net].link_idx, ovr[net].gate_idx);
	if (ovr[net].link_idx >= 0 or ovr[net].gate_idx >= 0) {
		//printf("col[%d] = %d\n", col.size()-1, col[col.size()-1].net);
		for (int r = col.size()-1; r >= 0 and col[r].net != net; r--) {
			// I should only push this edge if this signal needs to be routed over
			// the cell, and the edge is not already in the graph.
			// TODO: I need to account for merged source/drain
			//printf("check %d -> %d: ports:%d has:%d\n", col[r].net, net, ovr[col[r].net].gates + ovr[col[r].net].links, layer.hasEdge(col[r].net, net));
			if (ovr[col[r].net].gates + ovr[col[r].net].links > 1
				and not layer.hasEdge(col[r].net, net)) {
				layer.pushEdge(col[r].net, net);
			}
		}
	}

	if (is_gate) {
		ovr[net].gate_idx += 1;
	} else {
		ovr[net].link_idx += 1;
	}

	col.push_back(Column(0, net));
}

int Stack::stageStack(int sel, int flip)
{
	//printf("Staging Stack %d:%d\n", sel, flip);
	int cost = 0;
	this->idx[1] = sel;
	this->flip[1] = flip;
	if (not flip) {
		if (stage[0] == 0 or mos[sel].source != col[stage[0]-1].net) {
			stageColumn(mos[sel].source, false);
			if (stage[0] > 0) {
				cost += 1;
			}
			if (ovr[mos[sel].source].links - ovr[mos[sel].source].link_idx != 1) {
				cost += 1;
			}
		} else {
			// TODO: I have to decrement this if it is unstaged
			ovr[mos[sel].source].link_idx += 1;
		}
		for (int g = 0; g < (int)mos[sel].gate.size(); g++) {
			stageColumn(mos[sel].gate[g].net, true);
		}
		stageColumn(mos[sel].drain, false);
	} else {
		if (stage[0] == 0 or mos[sel].drain != col[stage[0]-1].net) {
			stageColumn(mos[sel].drain, false);
			if (stage[0] > 0) {
				cost += 1;
			}
			if (ovr[mos[sel].drain].links - ovr[mos[sel].drain].link_idx != 1) {
				cost += 1;
			}
		} else {
			// TODO: I have to decrement this if it is unstaged
			ovr[mos[sel].drain].link_idx += 1;
		}
		for (int g = mos[sel].gate.size()-1; g >= 0; g--) {
			stageColumn(mos[sel].gate[g].net, true);
		}
		stageColumn(mos[sel].source, false);
	}
	return cost;
}

LayoutTask::LayoutTask()
{
}

LayoutTask::~LayoutTask()
{
}

void LayoutTask::stash()
{
	//printf("STASH\n");
	cols.erase(cols.begin()+stage[0], cols.begin()+stage[1]);
	stage[1] = cols.size();
}

void LayoutTask::commit()
{
	//printf("COMMIT\n");
	cols.erase(cols.begin()+stage[1], cols.end());
	stage[0] = cols.size();
	stage[1] = cols.size();
}

void LayoutTask::clear()
{
	//printf("CLEAR\n");
	cols.resize(stage[1]);
}

void LayoutTask::reset()
{
	cols.resize(stage[0]);
	stage[1] = stage[0];
}

void LayoutTask::stageChannel()
{
	//A_NEW(cols, Route);
	if (cols.size() <= 1) {
	} else {
		//A_NEWP(A_NEXT(cols).assign, int, )
	}
}

void collectStacks(LayoutTask *task)
{
	for (int m = 0; m < (int)2; m++) {
		task->stack[m].countPorts();
	}

	for (int i = 0; i < (int)task->nets.size(); i++) {
		for (int m = 0; m < (int)2; m++) {
			task->nets[i].ports += task->stack[m].ovr[i].gates + task->stack[m].ovr[i].links;
		}
	}

	for (int m = 0; m < (int)2; m++) {
		task->stack[m].collect(task);
	}
}

void compute_stack_order(LayoutTask *task)
{
	int j[2] = {0,0};
	while (j[0] < (int)task->stack[0].mos.size() or j[1] < (int)task->stack[1].mos.size()) {
		// Alternate picking PMOS/NMOS stacks
		// Pick the stack that minimizes:
		//   - The number of color assignments in the over-the-cell routing problems
		//   - The number of edges introduced into the over-the-cell routing problems
		//   - The total expected horizontal distance between nets connected between the nmos and pmos stacks
		
		// Pick whichever stack currently has fewer columns as long as there are transistors left to route in that stack
		int i = 0;
		if (j[0] >= (int)task->stack[0].mos.size() or (j[1] < (int)task->stack[1].mos.size() and task->stack[1].col.size() < task->stack[0].col.size())) {
			i = 1;
		}

		int chan_cost = 0;
		int col_cost = 0;
		int edge_cost = 0;

		//printf("\n%s\n", i == 0 ? "NMOS" : "PMOS");
		/*for (int n = 0; n < (int)task->stack[i].ovr.size(); n++) {
			printf("node %d: gate:%d/%d link:%d/%d\n", n, task->stack[i].ovr[n].gate_idx, task->stack[i].ovr[n].gates, task->stack[i].ovr[n].link_idx, task->stack[i].ovr[n].links);
		}*/
		for (int k = 0; k < (int)task->stack[i].mos.size(); k++) {
			if (not task->stack[i].mos[k].selected) {
				for (int f = 0; f < (int)2; f++) {
					int chan = 0;
					int col = 0;
					int edge = 0;
					
					// compute the cost of selecting this stack
					col += task->stack[i].stageStack(k, f);
				
					edge += task->stack[i].layer.stashCost();

					/*printf("chan:%d/%d col:%d/%d edge:%d/%d\n", chan, chan_cost, col, col_cost, edge, edge_cost);

					for (int n = 0; n < (int)task->stack[i].ovr.size(); n++) {
						printf("node %d: gate:%d/%d link:%d/%d\n", n, task->stack[i].ovr[n].gate_idx, task->stack[i].ovr[n].gates, task->stack[i].ovr[n].link_idx, task->stack[i].ovr[n].links);
					}*/

					//if (task->stack[i].stage[0] == task->stack[i].stage[1] or (edge < edge_cost or (edge == edge_cost and col < col_cost))) {
					if (task->stack[i].stage[0] == task->stack[i].stage[1] or (chan < chan_cost or (chan == chan_cost and (col < col_cost or (col == col_cost and edge < edge_cost))))) {
						chan_cost = chan;
						col_cost = col;
						edge_cost = edge;
						task->stack[i].stash();
					} else {
						task->stack[i].clear();
					}

					/*for (int n = 0; n < (int)task->stack[i].ovr.size(); n++) {
						printf("node %d: gate:%d/%d link:%d/%d\n", n, task->stack[i].ovr[n].gate_idx, task->stack[i].ovr[n].gates, task->stack[i].ovr[n].link_idx, task->stack[i].ovr[n].links);
					}*/
				}
			}
		}

		task->stack[i].commit();
		/*for (int n = 0; n < (int)task->stack[i].ovr.size(); n++) {
			printf("node %d: gate:%d/%d link:%d/%d\n", n, task->stack[i].ovr[n].gate_idx, task->stack[i].ovr[n].gates, task->stack[i].ovr[n].link_idx, task->stack[i].ovr[n].links);
		}*/

		j[i] += 1;
	}
}

void compute_channel_routes(LayoutTask *task)
{
}

void processCell()
{
	bool discrete_lengths = false;
	int n_fold = 10;
	int p_fold = 10;

	LayoutTask task;

	/*// allocate a new array with all of the nets in the cell
	int max_net_id = -1;
	for (node_t *x = n->hd; x; x = x->next) {
		if (x->i > max_net_id) {
			max_net_id = x->i;
		}
	}
	max_net_id += 1;

	task.nets.resize(max_net_id);

	// allocate the arrays for the pull up and pull down stacks
	task.stack[0].init(max_net_id);
	task.stack[1].init(max_net_id);

	printf("\n\nStarting Layout %d\n", max_net_id);
	// loop through all of the nets in the cell
	for (node_t *x = n->hd; x; x = x->next) {
		task.nets[x->i].node = x;

		listitem_t *li;

		for (li = list_first(x->e); li; li = list_next(li)) {
			edge_t *e = (edge_t *)list_value(li);
			
			int len_repeat, width_repeat;
      int width_last;
      int il, iw;
      int w, l;
      int fold;

			
			if (e->visited || e->pruned)
				continue;
			
			e->visited = 1;
	
			w = e->w;
			l = e->l;

			// discretize lengths
			len_repeat = e->nlen;
			if (discrete_len > 0) {
				l = discrete_len;
			}

			if (e->type == EDGE_NFET) {
				fold = n_fold;
			} else {
				Assert (e->type == EDGE_PFET, "Hmm");
				fold = p_fold;
			}

			width_repeat = e->nfolds;

			for (int il = 0; il < len_repeat; il++) {
				for (int iw = 0; iw < width_repeat; iw++) {
					if (width_repeat > 1) {
						w = EDGE_WIDTH (e, iw);
					} else {
						w = e->w;
					}

					A_NEW(task.stack[e->type].mos, Device);
					new (&A_NEXT(task.stack[e->type].mos)) Device(e->g->i, e->a->i, e->b->i, e->bulk->i, w, l);
					A_INC(task.stack[e->type].mos);
				}
			}
		}
	}

	for (node_t *x = n->hd; x; x = x->next) {
    listitem_t *li;
    for (li = list_first (x->e); li; li = list_next (li)) {
      edge_t *e = (edge_t *)list_value (li);
      e->visited = 0;
    }
  }

	// TODO: figure out the stack ordering
	// TODO: route above and below the stacks
	// TODO: route the channel
	// TODO: parse the DRC rule files
	// TODO: add the DRC constraints into the routers

	
	collectStacks(&task);
	compute_stack_order(&task);

	char buf[1000];
	for (int i = 0; i < (int)task.nets.size(); i++) {
		if (task.nets[i].node != NULL and task.nets[i].node->v != NULL) {
			ActId *id = task.nets[i].node->v->v->id->toid();
			id->sPrint(buf, 1000);
			printf("%d: \"%s\" pmos:%d,%d nmos:%d,%d\n", i, buf, task.stack[1].ovr[i].links, task.stack[1].ovr[i].gates, task.stack[0].ovr[i].links, task.stack[0].ovr[i].gates);
		} else {
			printf("%d: \"\" pmos:%d,%d nmos:%d,%d\n", i, task.stack[1].ovr[i].links, task.stack[1].ovr[i].gates, task.stack[0].ovr[i].links, task.stack[0].ovr[i].gates);
		}
	}

	task.stack[0].print("nmos");
	task.stack[1].print("pmos");*/
}
