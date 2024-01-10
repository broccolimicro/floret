#include "Library.h"

#include <vector>

using namespace std;

// load a spice AST into the layout engine
void Library::loadSpice(pgen::spice_t spice, pgen::lexer_t &lexer, pgen::parsing ast) {
	for (auto tok = ast.tree.tokens.begin(); tok != ast.tree.tokens.end(); tok++) {
		if (tok->type == spice.SUBCKT) {
			
			tok->emit(lexer);
		}
	}
}

void Library::loadFile(string path) {
	// Initialize the grammar
	pgen::grammar_t gram;
	pgen::spice_t spice;
	spice.load(gram);

	// Load the file into the lexer
	pgen::lexer_t lexer;
	lexer.open(path);

	// Parse the file with the grammar
	pgen::parsing ast = gram.parse(lexer);
	if (ast.msgs.size() == 0) {
		// no errors, print the parsed abstract syntax tree
		loadSpice(spice, lexer, ast);
	} else {
		// there were parsing errors, print them out
		for (int i = 0; i < (int)ast.msgs.size(); i++) {
			cout << ast.msgs[i];
		}
	}
}
