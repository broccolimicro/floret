#include "Library.h"
#include "Draw.h"
#include "Placer.h"
#include "Router.h"
#include "Timer.h"

#include <ruler/Layout.h>
#include <vector>
#include <set>
#include <sys/stat.h>

using namespace std;

Library::Library() {
	this->tech = nullptr;
}

Library::Library(const Tech &tech) {
	this->tech = &tech;
}

Library::~Library() {
}

// load a spice AST into the layout engine
void Library::loadSpice(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice) {
	for (auto tok = spice.tokens.begin(); tok != spice.tokens.end(); tok++) {
		if (tok->type == lang.SUBCKT) {
			cells.push_back(Circuit(*tech));
			cells.back().loadSubckt(lang, lexer, *tok);
		}
	}
}

bool Library::loadFile(string path) {
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
	loadSpice(lang, lexer, ast.tree);
	printf("[%f]\n", timer.since());
	return true;
}

void Library::build(set<string> cellNames) {
	for (int i = 0; i < (int)cells.size(); i++) {
		if (cellNames.empty() or cellNames.find(cells[i].name) != cellNames.end()) {
			printf("\rPlacing %s\n", cells[i].name.c_str());
			Placement::solve(*tech, &cells[i]);
			printf("\rRouting %s\n", cells[i].name.c_str());
			Router router(&cells[i]);
			router.solve(*tech);
			//router.print();
			printf("\rDone %s\n", cells[i].name.c_str());
		}
	}
}

void Library::emitGDS(string libname, string filename, set<string> cellNames) {
	gdstk::Library lib = {};
	lib.init(libname.c_str(), tech->dbunit*1e-6, tech->dbunit*1e-6);
	for (int i = 0; i < (int)cells.size(); i++) {
		if (cellNames.empty() or cellNames.find(cells[i].name) != cellNames.end()) {
			Layout layout(*tech);
			cells[i].draw(layout);
			layout.emitGDS(lib);
		}
	}
	lib.write_gds(filename.c_str(), 0, NULL);
	lib.free_all();
}

void Library::emitRect(const ActConfig &act, string path, set<string> cellNames) {
	mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	for (int i = 0; i < (int)cells.size(); i++) {
		if (cellNames.empty() or cellNames.find(cells[i].name) != cellNames.end()) {
			string fpath = path;
			if (path.back() != '/') {
				fpath += "/";
			}
			fpath += cells[i].name + ".rect";
			printf("creating %s\n", fpath.c_str());
			FILE *fptr = fopen(fpath.c_str(), "w");
			Layout layout(*tech);
			cells[i].draw(layout);
			layout.emitRect(act, fptr);
			fclose(fptr);
		}
	}
}

