#include <stdio.h>
#include <string.h>
#include <set>
#include <map>
#include <utility>
#include <stdint.h>
#include "Cell.h"

Cell::Cell()
{
}

Cell::~Cell()
{
}

void Cell::stash()
{
	//printf("STASH\n");
	cols.erase(cols.begin()+stage[0], cols.begin()+stage[1]);
	stage[1] = cols.size();
}

void Cell::commit()
{
	//printf("COMMIT\n");
	cols.erase(cols.begin()+stage[1], cols.end());
	stage[0] = cols.size();
	stage[1] = cols.size();
}

void Cell::clear()
{
	//printf("CLEAR\n");
	cols.resize(stage[1]);
}

void Cell::reset()
{
	cols.resize(stage[0]);
	stage[1] = stage[0];
}

void Cell::stageChannel()
{
	//A_NEW(cols, Route);
	if (cols.size() <= 1) {
	} else {
		//A_NEWP(A_NEXT(cols).assign, int, )
	}
}

void Cell::collectStacks()
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

void Cell::orderStacks()
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

void Cell::routeChannel()
{
}

void Cell::fullLayout()
{
	// allocate the arrays for the pull up and pull down stacks
	stack[0].init(nets.size());
	stack[1].init(nets.size());

	printf("\n\nStarting Cell\n");

	// TODO: figure out the stack ordering
	// TODO: route above and below the stacks
	// TODO: route the channel
	// TODO: parse the DRC rule files
	// TODO: add the DRC constraints into the routers

	collectStacks();
	orderStacks();

	for (int i = 0; i < (int)nets.size(); i++) {
		printf("%d: \"%s\" pmos:%d,%d nmos:%d,%d\n", i, nets[i].name.c_str(), stack[1].ovr[i].links, stack[1].ovr[i].gates, stack[0].ovr[i].links, stack[0].ovr[i].gates);
	}

	stack[0].print("nmos");
	stack[1].print("pmos");
}

int Cell::findNet(string name) {
	for (int i = 0; i < (int)nets.size(); i++) {
		if (nets[i].name == name) {
			return i;
		}
	}
	return -1;
}

bool Cell::loadDevice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev) {
	// deviceType deviceName paramList
	if (dev.tokens.size() < 3) {
		return false;
	}

	string devType = lexer.read(dev.tokens[0].begin, dev.tokens[0].end);
	string devName = lexer.read(dev.tokens[1].begin, dev.tokens[1].end);

	// DESIGN(edward.bingham) Since we're focused on digital design, we'll only support transistor layout for now.
	if (string("mMxX").find(devType) == string::npos) {
		return false;
	}

	auto args = dev.tokens.begin() + 2;
	// Transistors must have the following args
	// drain gate source base modelName
	if (args->tokens.size() < 5) {
		return false;
	}

	// modelName cannot be a number or assignment
	if (args->tokens[4].type != lang.NAME) {
		return false;
	}

	string modelName = lexer.read(args->tokens[4].begin, args->tokens[4].end);
	int modelIdx = tech.findModel(modelName);
	// if the modelName isn't in the model list, then this is a non-transistor subckt
	if (modelIdx < 0) {
		return false;
	}
	int type = tech.models[modelIdx].type;

	vector<int> ports;
	map<string, float> params;
	double width = 0.0, length = 0.0;
	for (auto arg = args->tokens.begin(); arg != args->tokens.end(); arg++) {
		if (arg->type == lang.PARAM) {
			string paramName = lexer.read(arg->tokens[0].begin, arg->tokens[0].end);
			vector<double> values;
			for (auto value = arg->tokens.begin()+1; value != arg->tokens.end(); value++) {
				string valueStr = lexer.read(value->begin, value->end);
				printf("%s %s\n", paramName.c_str(), valueStr.c_str());
				values.push_back(stod(valueStr));
			}
			// TODO(edward.bingham) implement a lower() function
			// TODO(edward.bingham) implement a value parser that handles units
			if (paramName == "w" or paramName == "W") {
				width = values[0];
			} else if (paramName == "l" or paramName == "L") {
				length = values[0];
			}
		} else if (ports.size() < 4) {
			string netName = lexer.read(arg->begin, arg->end);
			int netIdx = findNet(netName);
			if (netIdx < 0) {
				netIdx = (int)nets.size();
				nets.push_back(Net(netName)); 
			}
			ports.push_back(netIdx);
		}
	}

	stack[type].mos.push_back(Term(modelIdx, ports[1], ports[2], ports[0], ports[3], (int)(width/tech.dbunit), (int)(length/tech.dbunit)));
	printf("%s %d %d %d %d %f %f\n", modelName.c_str(), ports[0], ports[1], ports[2], ports[3], width, length);

	return true;
}

void Cell::loadSubckt(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt) {
	for (auto tok = subckt.tokens.begin(); tok != subckt.tokens.end(); tok++) {
		if (tok->type == lang.NAME) {
			name = lexer.read(tok->begin, tok->end);
		} else if (tok->type == lang.PORT_LIST) {
			for (auto port = tok->tokens.begin(); port != tok->tokens.end(); port++) {
				nets.push_back(Net(lexer.read(port->begin, port->end)));
			}
		} else if (tok->type == lang.DEVICE) {
			if (not loadDevice(tech, lang, lexer, *tok)) {
				
				printf("unrecognized device\n");
			}
		}
	}
}
