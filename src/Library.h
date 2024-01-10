#pragma once

#include "spice.h"
#include "Tech.h"
#include "Cell.h"

struct Library {
	vector<string> spicePaths;
	string libPath;

	vector<Cell> cells;

	void loadSpice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice);
	void loadFile(const Tech &tech, string path); 

	void fullLayout();
};

