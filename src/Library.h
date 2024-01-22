#pragma once

#include <ruler/Tech.h>

#include "spice.h"
#include "Circuit.h"

struct Library {
	vector<string> spicePaths;
	string libPath;

	vector<Circuit> cells;

	void loadSpice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice);
	bool loadFile(const Tech &tech, string path); 

	void build(const Tech &tech);
};

