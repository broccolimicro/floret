#include <iostream>
#include <string>
#include <vector>

#include <phy/Tech.h>
#include <phy/Script.h>
#include <interpret_phy/import.h>
#include <interpret_phy/export.h>
#include <interpret_rect/ActConfig.h>
#include <interpret_rect/RectFile.h>
#include <phy/Library.h>
#include <sch/Netlist.h>
#include <sch/Tapeout.h>
#include <interpret_sch/import.h>
#include <interpret_sch/export.h>

#include "Timer.h"

#include <filesystem>
#include <chrono>
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
using namespace std::chrono;


using namespace std;

void printHelp() {
	printf("Usage: floret [options] <file.spi>\n");
	printf("An automated cell layout engine.\n");
	printf("\nOptions:\n");
	printf(" -h,--help      display this information\n");
	printf("    --version   display version information\n");
	printf(" -d,--debug     display internal debugging messages\n");
	printf(" -p,--progress  display progress information\n");
	printf("\n");
	printf(" -t,--tech <techfile>    manually specify the technology file and arguments\n");
	printf(" -c,--cells <celldir>    manually specify the cell directory\n");
	printf("\n");
	printf(" -o,--out       set the filename prefix for the saved intermediate stages\n\n");
	printf(" --no-cells     do not automatically break subcircuits into cells\n\n");
}

void printVersion() {
	printf("floret 1.1.0\n");
	printf("Copyright (C) 2024 Broccoli, LLC.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}

void export_cells(const phy::Library &lib, const sch::Netlist &net) {
	if (not filesystem::exists(lib.tech->lib)) {
		filesystem::create_directory(lib.tech->lib);
	}
	for (int i = 0; i < (int)lib.macros.size(); i++) {
		if (lib.macros[i].name.rfind("cell_", 0) == 0) {
			string cellPath = lib.tech->lib + "/" + lib.macros[i].name;
			if (not filesystem::exists(cellPath+".gds")) {
				export_layout(cellPath+".gds", lib.macros[i]);
				export_lef(cellPath+".lef", lib.macros[i]);
				if (i < (int)net.subckts.size()) {
					export_spi(cellPath+".spi", net, net.subckts[i]);
				}
			}
		}
	}
}

// returns whether the cell was imported
bool loadCell(phy::Library &lib, sch::Netlist &lst, int idx, bool progress=false, bool debug=false) {
	if (idx >= (int)lib.macros.size()) {
		lib.macros.resize(idx+1, Layout(*lib.tech));
	}
	lib.macros[idx].name = lst.subckts[idx].name;
	string cellPath = lib.tech->lib + "/" + lib.macros[idx].name+".gds";
	if (progress) {
		printf("  %s...", lib.macros[idx].name.c_str());
		fflush(stdout);
		printf("[");
	}

	sch::Subckt spiNet = lst.subckts[idx];
	spiNet.cleanDangling(true);
	spiNet.combineDevices();
	spiNet.canonicalize();

	if (filesystem::exists(cellPath)) {
		bool imported = import_layout(lib.macros[idx], cellPath, lib.macros[idx].name);
		if (progress) {
			if (imported) {
				sch::Subckt gdsNet(true);
				extract(gdsNet, lib.macros[idx], true);
				gdsNet.cleanDangling(true);
				gdsNet.combineDevices();
				gdsNet.canonicalize();
				if (gdsNet.compare(spiNet) == 0) {
					printf("%sFOUND%s]\n", KGRN, KNRM);
				} else {
					printf("%sFAILED LVS%s, ", KRED, KNRM);
					imported = false;
				}
			} else {
				printf("%sFAILED IMPORT%s, ", KRED, KNRM);
			}
		}
		if (imported) {
			return true;
		} else {
			lib.macros[idx].clear();
		}
	}

	int result = sch::routeCell(lib, lst, idx);
	if (progress) {
		if (result == 1) {
			printf("%sFAILED PLACEMENT%s]\n", KRED, KNRM);
		} else if (result == 2) {
			printf("%sFAILED ROUTING%s]\n", KRED, KNRM);
		} else {
			sch::Subckt gdsNet(true);
			extract(gdsNet, lib.macros[idx], true);
			gdsNet.cleanDangling(true);
			gdsNet.combineDevices();
			gdsNet.canonicalize();

			if (gdsNet.compare(spiNet) == 0) {
				printf("%sGENERATED%s]\n", KGRN, KNRM);
			} else {
				printf("%sFAILED LVS%s]\n", KRED, KNRM);
				if (debug) {
					gdsNet.print();
					spiNet.print();
				}
			}
		}
	}
	return false;
}

void loadCells(phy::Library &lib, sch::Netlist &lst, gdstk::GdsWriter *out=nullptr, bool progress=false, bool debug=false) {
	bool libFound = filesystem::exists(lib.tech->lib);
	if (progress) {
		printf("Load cell layouts:\n");
	}
	steady_clock::time_point start = steady_clock::now();
	lib.macros.reserve(lst.subckts.size()+lib.macros.size());
	for (int i = 0; i < (int)lst.subckts.size(); i++) {
		if (lst.subckts[i].isCell) {
			if (not loadCell(lib, lst, i, progress, debug)) {
				// We generated a new cell, save this to the cell library
				if (not libFound) {
					filesystem::create_directory(lib.tech->lib);
					libFound = true;
				}
				string cellPath = lib.tech->lib + "/" + lib.macros[i].name;
				export_layout(cellPath+".gds", lib.macros[i]);
				export_lef(cellPath+".lef", lib.macros[i]);
				export_spi(cellPath+".spi", lst, lst.subckts[i]);
			}
			if (out != nullptr) {
				out->write_cell(*phy::export_layout(lib.macros[i]));
			}
		}
	}
	steady_clock::time_point finish = steady_clock::now();
	if (progress) {
		printf("done [%gs]\n\n", ((float)duration_cast<milliseconds>(finish - start).count())/1000.0);
	}
}


int main(int argc, char **argv) {
	std::filesystem::path current = std::filesystem::current_path();

	char *loom_tech = std::getenv("LOOM_TECH");
	string techDir = "/usr/local/share/tech";
	if (loom_tech != nullptr) {
		techDir = string(loom_tech);
		if ((int)techDir.size() > 1 and techDir.back() == '/') {
			techDir.pop_back();
		}
	}

	string techPath;
	string cellsDir;
	bool manualCells = false;

	if (not techDir.empty()) {
		string tech = "sky130";
		std::filesystem::path search = current;
		while (not search.empty()) {
			std::ifstream fptr(search / "lm.mod");
			if (fptr) {
				tech = "";
				getline(fptr, tech, '\n');
				fptr.close();
				break;
			} else if (search.parent_path() == search) {
				break;
			}
			search = search.parent_path();
		}

		techPath = techDir + "/" + tech + "/tech.py";
		cellsDir = techDir + "/" + tech + "/cells";
	} else {
		techPath = "";
		cellsDir = "cells";
	}

	string filename = "";
	string prefix = "";
	string format = "";

	configuration config;
	config.set_working_directory(argv[0]);

	tokenizer tokens;
	bool noCells = false;

	bool progress = false;
	bool debug = false;

	for (int i = 1; i < argc; i++) {
		string arg = argv[i];
		if (arg == "--help" or arg == "-h") {
			printHelp();
			return 0;
		} else if (arg == "--version") {
			printVersion();
			return 0;
		} else if (arg == "--debug" or arg == "-d") {
			set_debug(true);
			debug = true;
		} else if (arg == "--progress" or arg == "-p") {
			progress = true;
		} else if (arg == "--tech" or arg == "-t") {
			if (++i >= argc) {
				printf("expected path to tech file.\n");
				return 0;
			}
			arg = argv[i];

			string path = extractPath(arg);
			string opt = (arg.size() > path.size() ? arg.substr(path.size()+1) : "");

			size_t dot = path.find_last_of(".");
			string ext = "";
			if (dot != string::npos) {
				ext = path.substr(dot+1);
			}

			if (ext == "py") {
				techPath = arg;
				if (not manualCells) {
					cellsDir = "cells";
				}
			} else if (ext == "") {
				if (not techDir.empty()) {
					techPath = techDir + "/" + path + "/tech.py" + opt;
					if (not manualCells) {
						cellsDir = techDir + "/" + path + "/cells";
					}
				} else {
					techPath = path+".py" + opt;
					if (not manualCells) {
						cellsDir = "cells";
					}
				}
			}
		} else if (arg == "--cells" or arg == "-c") {
			if (++i >= argc) {
				printf("expected path to cell directory.\n");
				return 0;
			}

			cellsDir = argv[i];
			manualCells = true;
		} else if (arg == "--no-cells") {
			noCells = true;
		} else if (arg == "-o" or arg == "--out") {
			if (++i >= argc) {
				printf("expected output prefix\n");
			}

			prefix = argv[i];
		} else {
			string path = extractPath(arg);
			string opt = (arg.size() > path.size() ? arg.substr(path.size()+1) : "");

			size_t dot = path.find_last_of(".");
			string ext = "";
			if (dot != string::npos) {
				ext = path.substr(dot+1);
				if (ext == "spice"
					or ext == "sp"
					or ext == "s") {
					ext = "spi";
				}
			}

			filename = path;
			format = ext;

			if (prefix == "") {
				prefix = dot != string::npos ? filename.substr(0, dot) : filename;
			}

			if (ext != "spi") {
				printf("unrecognized file format '%s'\n", ext.c_str());
				return 0;
			}
		}
	}

	if (filename.empty()) {
		printHelp();
		return 0;
	}

	Timer timer;

	if (techPath.empty()) {
		cout << "please provide a python techfile." << endl;
		if (!is_clean()) {
			complete();
			return 1;
		}
		return 0;
	}

	phy::Tech tech;
	if (not phy::loadTech(tech, techPath, cellsDir)) {
		cout << "techfile does not exist \'" + techPath + "\'." << endl;
		return 1;
	}
	sch::Netlist net(tech);

	if (format == "spi") {
		parse_spice::netlist::register_syntax(tokens);
		config.load(tokens, filename, "");

		tokens.increment(false);
		tokens.expect<parse_spice::netlist>();
		if (tokens.decrement(__FILE__, __LINE__))
		{
			parse_spice::netlist syntax(tokens);
			sch::import_netlist(syntax, net, &tokens);
		}
	}

	if (noCells) {
		for (int i = 0; i < (int)net.subckts.size(); i++) {
			net.subckts[i].isCell = true;
		}
	}

	net.mapCells(progress);

	if (format != "spi") {
		FILE *fout = stdout;
		if (prefix != "") {
			fout = fopen((prefix+".spi").c_str(), "w");
		}
		fprintf(fout, "%s", sch::export_netlist(net).to_string().c_str());
		fclose(fout);
	}

	phy::Library lib(tech);
	gdstk::GdsWriter gds = {};
	gds = gdstk::gdswriter_init((prefix+".gds").c_str(), prefix.c_str(), ((double)tech.dbunit)*1e-6, ((double)tech.dbunit)*1e-6, 4, nullptr, nullptr);

	if (format == "chp"
		or format == "hse"
		or format == "astg"
		or format == "prs"
		or format == "spi") {
		loadCells(lib, net, &gds, progress, debug);
	}

	gds.close();

	if (!is_clean()) {
		complete();
		return 1;
	}

	/*if (gdsPath != "") {
		emitGDS(gdsName, gdsPath, cellLib, cellNames);
	}
	if (rectPath != "") {
		act::ActConfig act;
		if (not act.load(tech, confPath+"/global.conf")) {
			printf("error loading: '%s/global.conf'\n", confPath.c_str());
		}
		if (not act.load(tech, confPath+"/layout.conf")) {
			printf("error loading: '%s/layout.conf'\n", confPath.c_str());
		}
		emitRect(rectPath, act, cellLib, cellNames);
	}*/
}

