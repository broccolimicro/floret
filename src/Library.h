#pragma once

#include "spice.h"
#include "Cell.h"

struct Library {
	vector<string> spicePaths;
	string libPath;

	vector<Cell> cells;

	void loadSpice(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice);
	void loadFile(string path); 
};

