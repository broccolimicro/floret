#include <iostream>
#include <string>
#include <vector>

#include "Tech.h"
#include "Library.h"
#include "Layout.h"

using namespace std;

void printHelp() {
	printf("Usage: floret [options] <file.spi>...\n");
	printf("A cell generator designed for advanced nodes.\n");
	printf("\nOptions:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" -c,--cells <dir> Look for fully or partially completed cell layouts in this directory (default: use current working directory)\n");
	printf(" -o,--output <dir> Place output files in this directory (default: use cells directory)\n");
}

void printVersion() {
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
			printHelp();
			return 0;
		} else if (arg == "--version") {
			printVersion();
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
		printHelp();
		return 0;
	}

	Tech tech;
	Library cellLib;
	cellLib.loadFile(tech, spiceFiles[0]);
	cellLib.fullLayout();
	
	Layout layout;
	layout.drawCell(tech, vec2i(), cellLib.cells[0]);
	layout.cleanup();
	layout.emit(tech, "test");
}

