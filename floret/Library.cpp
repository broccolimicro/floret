#include "Library.h"
#include "Draw.h"
#include "Placer.h"
#include "Router.h"

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
	if (ast.msgs.size() != 0) {
		// there were parsing errors, print them out
		for (int i = 0; i < (int)ast.msgs.size(); i++) {
			cout << ast.msgs[i];
		}
		return false;
	}

	// no errors, print the parsed abstract syntax tree
	loadSpice(lang, lexer, ast.tree);
	return true;
}

void Library::loadConfValue(pgen::conf_t lang, pgen::lexer_t &lexer, pgen::token_t &value, string name, map<string, string> &mtrlMap) {
	string kind = lexer.read(value.tokens[0].begin, value.tokens[0].end);
	string attr = lexer.read(value.tokens[1].begin, value.tokens[1].end);

	if (kind == "string" and (name.rfind(".vias", 0) == 0 or name.rfind(".materials.metal", 0) == 0) and attr.rfind("_name") == string::npos and attr.rfind("_lefname") == string::npos) {
		attr = attr.substr(0, attr.rfind("_"));
		string str = lexer.read(value.tokens[2].begin, value.tokens[2].end);
		str = str.substr(1, str.size()-2);
		mtrlMap[attr] = str;
	} else if (kind == "string_table" and (name.rfind(".materials", 0) == 0 or name.rfind(".vias", 0) == 0) and attr.rfind("gds") != string::npos) {
		vector<int> layers;
		for (auto tok = value.tokens.begin()+2; tok != value.tokens.end(); tok++) {
			string layer = lexer.read(tok->begin, tok->end);
			layer = layer.substr(1, layer.size()-2);
			int idx = tech->findPaint(layer);
			if (idx >= 0) {
				layers.push_back(idx);
			}
		}
		
		if (not layers.empty()) {
			string mtrl;
			if (attr.rfind("_gds") != string::npos) {
				mtrl = attr.substr(0, attr.rfind("_"));
				mtrl = mtrlMap[mtrl];
			} else {
				mtrl = name.substr(name.rfind(".")+1);
			}

			mtrls.push_back(pair<string, vector<int> >(mtrl, layers));
		}
	}
}

void Library::loadConfBlock(pgen::conf_t lang, pgen::lexer_t &lexer, pgen::token_t &block, string name) {
	map<string, string> mtrlMap;
	for (auto tok = block.tokens.begin(); tok != block.tokens.end(); tok++) {
		if (tok->type == lang.TABLE) {
			loadConfValue(lang, lexer, *tok, name, mtrlMap);
		} else if (tok->type == lang.VALUE) {
			loadConfValue(lang, lexer, *tok, name, mtrlMap);
		} else if (tok->type == lang.SECTION) {
			string sub = name + "." + lexer.read(tok->tokens[0].begin, tok->tokens[0].end);
			loadConfBlock(lang, lexer, tok->tokens[1], sub);
		}
	}
}

bool Library::loadLayoutConf(string path) {
	// Initialize the grammar
	pgen::grammar_t gram;
	pgen::conf_t lang;
	lang.load(gram);

	// Load the file into the lexer
	pgen::lexer_t lexer;
	if (not lexer.open(path)) {
		return false;
	}

	// Parse the file with the grammar
	pgen::parsing ast = gram.parse(lexer);
	if (ast.msgs.size() != 0) {
		// there were parsing errors, print them out
		for (int i = 0; i < (int)ast.msgs.size(); i++) {
			cout << ast.msgs[i];
		}
		return false;
	}

	// no errors, print the parsed abstract syntax tree
	loadConfBlock(lang, lexer, ast.tree.tokens[0], "");

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

void Library::emitRect(string path, set<string> cellNames) {
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
			layout.emitRect(fptr, mtrls);
			fclose(fptr);
		}
	}
}

