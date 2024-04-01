#include <iostream>
#include <string>
#include <vector>

#include <ruler/Tech.h>
#include <ruler/Script.h>
#include <floret/Library.h>

using namespace std;

void printHelp() {
	printf("Usage: floret [options] <tech.py> <file.spi>...\n");
	printf("A cell generator designed for advanced nodes.\n");
	printf("\nOptions:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" --gds <name> <path.gds> emit cell layouts as a .gds library.\n");
	printf(" --rect <path> emit cell layouts as .rect files.\n");
	printf("--------------------------------------------------\n");
	printf(" -s,--select <cell>,<cell>,... do layout for only these cells from the spice file\n");
}

void printVersion() {
	printf("floret 1.0.0\n");
	printf("Copyright (C) 2024 Broccoli, LLC.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}

int main(int argc, char **argv) {
	string techPath = "";
	vector<string> spiceFiles;

	string gdsName = "";
	string gdsPath = "";
	string rectPath = "";

	set<string> cellNames;
	for (int i = 1; i < argc; i++) {
		string arg = argv[i];
		if (arg == "--help" or arg == "-h") {
			printHelp();
			return 0;
		} else if (arg == "--version") {
			printVersion();
			return 0;
		} else if (arg == "-s" or arg == "--select") {
			i++;
			if (i < argc) {
				string cellList = argv[i];
				size_t pos = 0;
				std::string token;
				while ((pos = cellList.find(",")) != string::npos) {
					cellNames.insert(cellList.substr(0, pos));
					cellList.erase(0, pos + 1);
				}
				cellNames.insert(cellList);
			} else {
				cout << "expected list of cell names" << endl;
				return 1;
			}
		} else if (arg == "--gds") {
			i++;
			if (i < argc) {
				gdsName = argv[i];
			} else {
				cout << "expected name of .gds library" << endl;
				return 1;
			}

			i++;
			if (i < argc) {
				gdsPath = argv[i];
			} else {
				cout << "expected output path for .gds library" << endl;
				return 1;
			}
		} else if (arg == "--rect") {
			i++;
			if (i < argc) {
				rectPath = argv[i];
			} else {
				cout << "expected output directory for .rect files" << endl;
				return 1;
			}
		} else if (techPath == "") {
			techPath = argv[i];
		} else {
			spiceFiles.push_back(argv[i]);
		}
	}

	if (spiceFiles.size() == 0) {
		printHelp();
		return 0;
	}

	Tech tech;
	loadTech(tech, techPath);
	Library cellLib(tech);
	for (int i = 0; i < (int)spiceFiles.size(); i++) {
		if (not cellLib.loadFile(spiceFiles[i])) {
			printf("file not found: '%s'\n", spiceFiles[i].c_str());
		}
	}
	cellLib.build(cellNames);
	if (gdsPath != "") {
		cellLib.emitGDS(gdsName, gdsPath, cellNames);
	}
	if (rectPath != "") {
		cellLib.emitRect(rectPath, cellNames);
	}
}

