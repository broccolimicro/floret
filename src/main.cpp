#include <iostream>
#include <string>
#include <vector>

#include <phy/Tech.h>
#include <phy/Script.h>
#include <interpret_rect/ActConfig.h>
#include <sch/Library.h>

#include "Timer.h"
#include "interpret.h"

using namespace std;

void printHelp() {
	printf("Usage: floret [options] <tech.py> <file.spi>...\n");
	printf("A cell generator designed for advanced nodes.\n");
	printf("\nOptions:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" --gds <libName> <libPath.gds> emit cell layouts as a .gds library.\n");
	printf(" --rect <rectDir> <layoutPath.conf> emit cell layouts as .rect files.\n");
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
	string confPath = "";

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

			i++;
			if (i < argc) {
				confPath = argv[i];
			} else {
				cout << "expected path to layout.conf" << endl;
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

	Timer timer;
	Tech tech;
	printf("loading tech -- ");
	fflush(stdout);
	timer.reset();
	loadTech(tech, techPath);
	Library cellLib(tech);
	printf("[%f]\n", timer.since());
	for (int i = 0; i < (int)spiceFiles.size(); i++) {
		printf("loading %s -- ", spiceFiles[i].c_str());
		fflush(stdout);
		timer.reset();
		if (not loadFile(cellLib, spiceFiles[i])) {
			printf("file not found: '%s'\n", spiceFiles[i].c_str());
		}
		printf("[%f]\n", timer.since());
	}
	printf("building cells -- ");
	fflush(stdout);
	timer.reset();
	cellLib.build(cellNames);
	printf("[%f]\n", timer.since());
	if (gdsPath != "") {
		cellLib.emitGDS(gdsName, gdsPath, cellNames);
	}
	if (rectPath != "") {
		act::ActConfig act;
		if (not act.load(tech, confPath+"/global.conf")) {
			printf("error loading: '%s/global.conf'\n", confPath.c_str());
		}
		if (not act.load(tech, confPath+"/layout.conf")) {
			printf("error loading: '%s/layout.conf'\n", confPath.c_str());
		}
		cellLib.emitRect(act, rectPath, cellNames);
	}
}

