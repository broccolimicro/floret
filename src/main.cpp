#include "spice.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Config {
	
};

void print_help() {
	printf("Usage: floret [options] <file.spi>...\n");
	printf("A cell generator designed for advanced nodes.\n");
	printf("\nOptions:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" -c,--cells <dir> Look for fully or partially completed cell layouts in this directory (default: use current working directory)\n");
	printf(" -o,--output <dir> Place output files in this directory (default: use cells directory)\n");
}

void print_version() {
	printf("floret 0.0.0\n");
	printf("Copyright (C) 2024 Broccoli, LLC.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}

int main(int argc, char **argv) {
	string cellsDir = "";
	string outputDir = "";
	vector<string> spiceFiles;
	for (int i = 1; i < argc; i++) {
		string arg = argv[i];
		if (arg == "--help" or arg == "-h") {
			print_help();
			return 0;
		} else if (arg == "--version") {
			print_version();
			return 0;
		} else if (arg == "-c" or arg == "--cells") {
			i++;
			if (i < argc) {
				cellsDir = argv[i];
			} else {
				cout << "expected rect directory" << endl;
				return 1;
			}
		} else if (arg == "-o" or arg == "--output") {
			i++;
			if (i < argc) {
				outputDir = argv[i];
			} else {
				cout << "expected output directory" << endl;
				return 1;
			}
		} else {
			spiceFiles.push_back(argv[i]);
		}
	}

	if (spiceFiles.size() == 0) {
		print_help();
		return 0;
	}

	Config config;

	// Initialize the grammar
	pgen::grammar_t gram;
	pgen::spice_t spice;
	spice.load(gram);

	// Load the file into the lexer
	pgen::lexer_t lexer;
	lexer.open(spiceFiles[0]);

	// Parse the file with the grammar
	pgen::parsing result = gram.parse(lexer);
	if (result.msgs.size() == 0) {
		// no errors, print the parsed abstract syntax tree
		result.tree.emit(lexer);
	} else {
		// there were parsing errors, print them out
		for (int i = 0; i < (int)result.msgs.size(); i++) {
			cout << result.msgs[i];
		}
	}
}

