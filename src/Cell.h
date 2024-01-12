#pragma once

#include <vector>
#include <string>
#include "Stack.h"
#include "spice.h"
#include "Tech.h"

using namespace std;

struct Route
{
	vector<int> assign;
};

struct Cell
{
	Cell();
	~Cell();

	string name;

	Stack stack[2];
	vector<Net> nets;

	vector<Route> cols;
	int stage[2];

	void stash();
	void commit();
	void clear();
	void reset();
	void stageChannel();

	void collectStacks();
	void orderStacks();
	void routeChannel();
	void fullLayout();

	int findNet(string name);

	bool loadDevice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev);
	void loadSubckt(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt);
};

