#include <stdio.h>
#include <string.h>
#include <set>
#include <map>
#include <utility>
#include <stdint.h>
#include "Layout.h"

Layout::Layout()
{
}

Layout::~Layout()
{
}

void Layout::stash()
{
	//printf("STASH\n");
	cols.erase(cols.begin()+stage[0], cols.begin()+stage[1]);
	stage[1] = cols.size();
}

void Layout::commit()
{
	//printf("COMMIT\n");
	cols.erase(cols.begin()+stage[1], cols.end());
	stage[0] = cols.size();
	stage[1] = cols.size();
}

void Layout::clear()
{
	//printf("CLEAR\n");
	cols.resize(stage[1]);
}

void Layout::reset()
{
	cols.resize(stage[0]);
	stage[1] = stage[0];
}

void Layout::stageChannel()
{
	//A_NEW(cols, Route);
	if (cols.size() <= 1) {
	} else {
		//A_NEWP(A_NEXT(cols).assign, int, )
	}
}

void Layout::collectStacks()
{
	for (int m = 0; m < (int)2; m++) {
		stack[m].countPorts();
	}

	for (int i = 0; i < (int)nets.size(); i++) {
		for (int m = 0; m < (int)2; m++) {
			nets[i].ports += stack[m].ovr[i].gates + stack[m].ovr[i].links;
		}
	}

	for (int m = 0; m < (int)2; m++) {
		stack[m].collect(this);
	}
}

void Layout::orderStacks()
{
	int j[2] = {0,0};
	while (j[0] < (int)stack[0].mos.size() or j[1] < (int)stack[1].mos.size()) {
		// Alternate picking PMOS/NMOS stacks
		// Pick the stack that minimizes:
		//   - The number of color assignments in the over-the-cell routing problems
		//   - The number of edges introduced into the over-the-cell routing problems
		//   - The total expected horizontal distance between nets connected between the nmos and pmos stacks
		
		// Pick whichever stack currently has fewer columns as long as there are transistors left to route in that stack
		int i = 0;
		if (j[0] >= (int)stack[0].mos.size() or (j[1] < (int)stack[1].mos.size() and stack[1].col.size() < stack[0].col.size())) {
			i = 1;
		}

		int chan_cost = 0;
		int col_cost = 0;
		int edge_cost = 0;

		//printf("\n%s\n", i == 0 ? "NMOS" : "PMOS");
		/*for (int n = 0; n < (int)stack[i].ovr.size(); n++) {
			printf("node %d: gate:%d/%d link:%d/%d\n", n, stack[i].ovr[n].gateIdx, stack[i].ovr[n].gates, stack[i].ovr[n].linkIdx, stack[i].ovr[n].links);
		}*/
		for (int k = 0; k < (int)stack[i].mos.size(); k++) {
			if (not stack[i].mos[k].selected) {
				for (int f = 0; f < (int)2; f++) {
					int chan = 0;
					int col = 0;
					int edge = 0;
					
					// compute the cost of selecting this stack
					col += stack[i].stageStack(k, f);
				
					edge += stack[i].layer.stashCost();

					/*printf("chan:%d/%d col:%d/%d edge:%d/%d\n", chan, chan_cost, col, col_cost, edge, edge_cost);

					for (int n = 0; n < (int)stack[i].ovr.size(); n++) {
						printf("node %d: gate:%d/%d link:%d/%d\n", n, stack[i].ovr[n].gateIdx, stack[i].ovr[n].gates, stack[i].ovr[n].linkIdx, stack[i].ovr[n].links);
					}*/

					//if (stack[i].stage[0] == stack[i].stage[1] or (edge < edge_cost or (edge == edge_cost and col < col_cost))) {
					if (stack[i].stage[0] == stack[i].stage[1] or (chan < chan_cost or (chan == chan_cost and (col < col_cost or (col == col_cost and edge < edge_cost))))) {
						chan_cost = chan;
						col_cost = col;
						edge_cost = edge;
						stack[i].stash();
					} else {
						stack[i].clear();
					}

					/*for (int n = 0; n < (int)stack[i].ovr.size(); n++) {
						printf("node %d: gate:%d/%d link:%d/%d\n", n, stack[i].ovr[n].gateIdx, stack[i].ovr[n].gates, stack[i].ovr[n].linkIdx, stack[i].ovr[n].links);
					}*/
				}
			}
		}

		stack[i].commit();
		/*for (int n = 0; n < (int)stack[i].ovr.size(); n++) {
			printf("node %d: gate:%d/%d link:%d/%d\n", n, stack[i].ovr[n].gateIdx, stack[i].ovr[n].gates, stack[i].ovr[n].linkIdx, stack[i].ovr[n].links);
		}*/

		j[i] += 1;
	}
}

void Layout::routeChannel()
{
}

void processCell()
{
	bool discrete_lengths = false;
	int n_fold = 10;
	int p_fold = 10;

	Layout task;

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

					A_NEW(task.stack[e->type].mos, Term);
					new (&A_NEXT(task.stack[e->type].mos)) Term(e->g->i, e->a->i, e->b->i, e->bulk->i, w, l);
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
	orderStacks(&task);

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
