#pragma once

#include "spice.h"
#include "Cell.h"

struct Library {
	vector<string> spicePaths;
	string libPath;

	vector<Cell> cells;

	void loadSpice(pgen::spice_t spice, pgen::lexer_t &lexer, pgen::parsing ast);
	void loadFile(string path); 
};

