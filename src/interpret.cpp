#include "interpret.h"
#include "Timer.h"

#include <utility>
#include <math.h>
#include <string>
#include <stdlib.h>

using namespace std;

string lower(string str) {
	for (auto c = str.begin(); c != str.end(); c++) {
		*c = tolower(*c);
	}
	return str;
}

double loadValue(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &val) {
	string constStr = "0.0";
	if (val.tokens.size() == 0) {
		constStr = lexer.read(val.begin, val.end);
	} else {
		constStr = lexer.read(val.tokens[0].begin, val.tokens[0].end);
	}

	string unitStr = "";
	if (val.tokens.size() == 2) {
		unitStr = lower(lexer.read(val.tokens[1].begin, val.tokens[1].end));
	}

	double value = stod(constStr);
	if (unitStr.size() == 0) {
		return value;
	}

	int unit = (int)(string("afpnumkxg").find(unitStr));
	if (unit >= 0) {
		int exp = 3*unit - 18;
		value *= pow(10.0, (double)exp);
	}

	return value;
}

bool loadDevice(Circuit &ckt, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev) {
	// deviceType deviceName paramList
	if (dev.tokens.size() < 3) {
		printf("not a device\n");
		return false;
	}

	string devType = lower(lexer.read(dev.tokens[0].begin, dev.tokens[0].end));
	string devName = lexer.read(dev.tokens[1].begin, dev.tokens[1].end);

	// DESIGN(edward.bingham) Since we're focused on digital design, we'll only support transistor layout for now.
	if (string("mx").find(devType) == string::npos) {
		printf("not transistor or subckt %s%s\n", devType.c_str(), devName.c_str());
		return false;
	}

	auto args = dev.tokens.begin() + 2;
	// Moss must have the following args
	// drain gate source base modelName
	if (args->tokens.size() < 5) {
		printf("not enough args\n");
		return false;
	}

	// modelName cannot be a number or assignment
	if (args->tokens[4].type != lang.NAME) {
		printf("model name not present\n");
		return false;
	}

	string modelName = lexer.read(args->tokens[4].begin, args->tokens[4].end);
	int modelIdx = ckt.tech.findModel(modelName);
	// if the modelName isn't in the model list, then this is a non-transistor subckt
	if (modelIdx < 0) {
		printf("model not found %s\n", modelName.c_str());
		return false;
	}

	int port = 0;
	int type = ckt.tech.models[modelIdx].type;
	ckt.mos.push_back(Mos(modelIdx, type));
	for (auto arg = args->tokens.begin(); arg != args->tokens.end(); arg++) {
		if (arg->type == lang.PARAM) {
			string paramName = lower(lexer.read(arg->tokens[0].begin, arg->tokens[0].end));
			vector<double> values;
			for (auto value = arg->tokens.begin()+1; value != arg->tokens.end(); value++) {
				values.push_back(loadValue(lang, lexer, *value));
			}
			if (paramName == "w") {
				ckt.mos.back().size[1] = int(values[0]/ckt.tech.dbunit);
			} else if (paramName == "l") {
				ckt.mos.back().size[0] = int(values[0]/ckt.tech.dbunit);
			} else {
				ckt.mos.back().params.insert(pair<string, vector<double> >(paramName, values));
			}
		} else if (port < 4) {
			string netName = lexer.read(arg->begin, arg->end);
			int net = ckt.findNet(netName, true);
			if (port == 1) {
				ckt.nets[net].gates[type]++;
				ckt.mos.back().gate = net;
			} else if (port == 3) {
				ckt.mos.back().bulk = net;
			} else {
				ckt.nets[net].ports[type]++;
				ckt.mos.back().ports.push_back(net);
			}
			port++;
		}
	}

	return true;
}

void loadSubckt(Circuit &ckt, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt) {
	for (auto tok = subckt.tokens.begin(); tok != subckt.tokens.end(); tok++) {
		if (tok->type == lang.NAME) {
			ckt.name = lexer.read(tok->begin, tok->end);
		} else if (tok->type == lang.PORT_LIST) {
			for (auto port = tok->tokens.begin(); port != tok->tokens.end(); port++) {
				ckt.nets.push_back(Net(lexer.read(port->begin, port->end), true));
			}
		} else if (tok->type == lang.DEVICE) {
			if (not loadDevice(ckt, lang, lexer, *tok)) {
				printf("unrecognized device\n");
			}
		}
	}
}

// load a spice AST into the layout engine
void loadSpice(Library &lib, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice) {
	for (auto tok = spice.tokens.begin(); tok != spice.tokens.end(); tok++) {
		if (tok->type == lang.SUBCKT) {
			lib.cells.push_back(Circuit(*lib.tech));
			loadSubckt(lib.cells.back(), lang, lexer, *tok);
		}
	}
}

bool loadFile(Library &lib, string path) {
	Timer timer;
	printf("loadFile\n");

	printf("lang.load -- ");
	fflush(stdout);
	timer.reset();
	// Initialize the grammar
	pgen::grammar_t gram;
	pgen::spice_t lang;
	lang.load(gram);
	printf("[%f]\n", timer.since());

	printf("lexer.open -- ");
	fflush(stdout);
	timer.reset();
	// Load the file into the lexer
	pgen::lexer_t lexer;
	if (not lexer.open(path)) {
		return false;
	}
	printf("[%f]\n", timer.since());

	printf("gram.parse -- ");
	fflush(stdout);
	timer.reset();
	// Parse the file with the grammar
	pgen::parsing ast = gram.parse(lexer);
	printf("[%f]\n", timer.since());
	if (ast.msgs.size() != 0) {
		// there were parsing errors, print them out
		for (int i = 0; i < (int)ast.msgs.size(); i++) {
			cout << ast.msgs[i];
		}
		return false;
	}

	printf("loadSpice -- ");
	fflush(stdout);
	timer.reset();
	// no errors, print the parsed abstract syntax tree
	loadSpice(lib, lang, lexer, ast.tree);
	printf("[%f]\n", timer.since());
	return true;
}

