#include "Stack.h"
#include "Layout.h"

Net::Net()
{
	ports = 0;
}

Net::~Net()
{
}

TermIndex::TermIndex()
{
	idx = 0;
	flip = 0;
}

TermIndex::TermIndex(int idx, int flip)
{
	this->idx = idx;
	this->flip = flip;
}

TermIndex::~TermIndex()
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
	gateIdx = -1;
	linkIdx = -1;
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
		ovr[mos[idx[1]].source].linkIdx -= 1;
		ovr[mos[idx[1]].drain].linkIdx -= 1;
		for (int i = 0; i < (int)mos[idx[1]].gate.size(); i++) {
			ovr[mos[idx[1]].gate[i].net].gateIdx -= 1;
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
		ovr[mos[idx[1]].source].linkIdx -= 1;
		ovr[mos[idx[1]].drain].linkIdx -= 1;
		for (int i = 0; i < (int)mos[idx[1]].gate.size(); i++) {
			ovr[mos[idx[1]].gate[i].net].gateIdx -= 1;
		}
	}

	if (stage[0] < stage[1]) {
		ovr[mos[idx[0]].source].linkIdx += 1;
		ovr[mos[idx[0]].drain].linkIdx += 1;
		for (int i = 0; i < (int)mos[idx[0]].gate.size(); i++) {
			ovr[mos[idx[0]].gate[i].net].gateIdx += 1;
		}
	}

	col.erase(col.begin()+stage[1], col.end());
	stage[0] = col.size();
	stage[1] = col.size();
	layer.commit();
	sel.push_back(TermIndex(idx[0], flip[0]));
	mos[idx[0]].selected = 1;
}

void Stack::clear()
{
	//printf("CLEAR\n");
	if (stage[1] < (int)col.size()) {
		ovr[mos[idx[1]].source].linkIdx -= 1;
		ovr[mos[idx[1]].drain].linkIdx -= 1;
		for (int i = 0; i < (int)mos[idx[1]].gate.size(); i++) {
			ovr[mos[idx[1]].gate[i].net].gateIdx -= 1;
		}
	}

	col.resize(stage[1]);
	layer.clear();
}

void Stack::reset()
{
	if (stage[1] < (int)col.size()) {
		ovr[mos[idx[1]].source].linkIdx -= 1;
		ovr[mos[idx[1]].drain].linkIdx -= 1;
		for (int i = 0; i < (int)mos[idx[1]].gate.size(); i++) {
			ovr[mos[idx[1]].gate[i].net].gateIdx -= 1;
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
		ovr[mos[i].source].links += 1;
		ovr[mos[i].drain].links += 1;
		for (int j = 0; j < (int)mos[i].gate.size(); j++) {
			ovr[mos[i].gate[j].net].gates += 1;
		}
	}
}

// merge all serial-only transistor stacks into terms
void Stack::collect(Layout *task)
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
	//printf("Staging MOS %d: %d,%d\n", net, ovr[net].linkIdx, ovr[net].gateIdx);
	if (ovr[net].linkIdx >= 0 or ovr[net].gateIdx >= 0) {
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
		ovr[net].gateIdx += 1;
	} else {
		ovr[net].linkIdx += 1;
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
			if (ovr[mos[sel].source].links - ovr[mos[sel].source].linkIdx != 1) {
				cost += 1;
			}
		} else {
			// TODO: I have to decrement this if it is unstaged
			ovr[mos[sel].source].linkIdx += 1;
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
			if (ovr[mos[sel].drain].links - ovr[mos[sel].drain].linkIdx != 1) {
				cost += 1;
			}
		} else {
			// TODO: I have to decrement this if it is unstaged
			ovr[mos[sel].drain].linkIdx += 1;
		}
		for (int g = mos[sel].gate.size()-1; g >= 0; g--) {
			stageColumn(mos[sel].gate[g].net, true);
		}
		stageColumn(mos[sel].source, false);
	}
	return cost;
}

