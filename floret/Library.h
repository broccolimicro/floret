#pragma once

#include "spice.h"
#include "Circuit.h"

#include <ruler/Tech.h>
#include <set>

using namespace std;

struct Library {
	Library();
	Library(const Tech &tech);
	~Library();

	const Tech *tech;

	vector<string> spicePaths;
	string libPath;

	vector<Circuit> cells;

	void loadSpice(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice);
	bool loadFile(string path); 

	void build(set<string> cellNames = set<string>());

	void emitGDS(string libname, string filename, set<string> cellNames = set<string>());
	void emitRect(string path, set<string> cellNames = set<string>());
};

