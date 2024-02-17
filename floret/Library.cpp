#include "Library.h"
#include "Draw.h"
#include "Placer.h"
#include "Router.h"

#include <ruler/Layout.h>
#include <vector>
#include <set>

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

bool Library::loadFile(const Tech &tech, string path) {
	// Initialize the grammar
	pgen::grammar_t gram;
	pgen::spice_t lang;
	lang.load(gram);

	// Load the file into the lexer
	pgen::lexer_t lexer;
	if (not lexer.open(path)) {
		return false;
	}

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

	return true;
}

void Library::build(const Tech &tech, set<string> cellNames) {
	gdstk::Library lib = {};
	lib.init("test", tech.dbunit*1e-6, tech.dbunit*1e-6);
	for (int i = 0; i < (int)cells.size(); i++) {
		if (cellNames.empty() or cellNames.find(cells[i].name) != cellNames.end()) {
			printf("\rPlacing %s\n", cells[i].name.c_str());
			Placement::solve(tech, &cells[i]);
			printf("\rRouting %s\n", cells[i].name.c_str());
			Router router(&cells[i]);
			router.solve(tech);
			//router.print();
			printf("\rDrawing %s\n", cells[i].name.c_str());
			Layout layout;
			cells[i].draw(tech, layout);
			layout.emit(tech, lib);
			printf("\rDone %s\n", cells[i].name.c_str());
		}
	}
	lib.write_gds("test.gds", 0, NULL);
	lib.free_all();
}
