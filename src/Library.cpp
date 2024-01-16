#include "Library.h"
#include "Solution.h"

#include <vector>

using namespace std;

// load a spice AST into the layout engine
void Library::loadSpice(const Tech &tech, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice) {
	for (auto tok = spice.tokens.begin(); tok != spice.tokens.end(); tok++) {
		if (tok->type == lang.SUBCKT) {
			cells.push_back(Circuit());
			cells.back().loadSubckt(tech, lang, lexer, *tok);
		}
	}
}

void Library::loadFile(const Tech &tech, string path) {
	// Initialize the grammar
	pgen::grammar_t gram;
	pgen::spice_t lang;
	lang.load(gram);

	// Load the file into the lexer
	pgen::lexer_t lexer;
	lexer.open(path);

	// Parse the file with the grammar
	pgen::parsing ast = gram.parse(lexer);
	if (ast.msgs.size() == 0) {
		// no errors, print the parsed abstract syntax tree
		loadSpice(tech, lang, lexer, ast.tree);
	} else {
		// there were parsing errors, print them out
		for (int i = 0; i < (int)ast.msgs.size(); i++) {
			cout << ast.msgs[i];
		}
	}
}

void Library::build(const Tech &tech) {
	for (int i = 0; i < (int)cells.size(); i++) {
		cells[i].solve(tech);
		if (cells[i].layout != nullptr) {
			cells[i].layout->draw(tech);
		}
	}
}
