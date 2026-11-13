#include <iostream>
#include <string>
#include <vector>

#include <phy/Tech.h>
#include <phy/Script.h>
#include <interpret_phy/import.h>
#include <interpret_phy/export.h>
//#include <interpret_rect/ActConfig.h>
//#include <interpret_rect/RectFile.h>
#include <phy/Layout.h>
#include <sch/Subckt.h>
#include <sch/Tapeout.h>
#include <sch/Placer.h>
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

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

std::string readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary);
	if (not file.is_open()) {
		throw std::runtime_error("Cannot open file: " + filename);
	}

	return std::string(
		std::istreambuf_iterator<char>{file},
		std::istreambuf_iterator<char>{}
	);
}

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

namespace cell {

void export_cell(std::string path, const phy::Tech &tech, const sch::Subckt &ckt, const phy::Layout &macro) {
	string cellPath = path + "/" + ckt.name;
	export_spi(cellPath+".spi", tech, ckt);
	export_layout(cellPath+".gds", macro);
	export_lef(cellPath+".lef", macro);
}

void export_cells(std::string path, const phy::Tech &tech, std::vector<sch::Subckt> &subckts, std::vector<phy::Layout> &macros) {
	if (not filesystem::exists(path)) {
		filesystem::create_directory(path);
	}
	for (size_t i = 0; i < subckts.size(); i++) {
		if (subckts[i].isCell) {
			export_cell(path, tech, subckts[i], macros[i]);
		}
	}
}

// returns whether the cell was imported
bool import_cell(std::string path, const phy::Tech &tech, std::vector<sch::Subckt> &subckts, std::vector<phy::Layout> &macros, int index, bool progress, bool debug) {
	string cellPath = path + "/" + subckts[index].name+".gds";
	if (progress) {
		printf("  %s...", subckts[index].name.c_str());
		fflush(stdout);
		printf("[");
	}

	Timer tmr;
	float searchDelay = 0.0;
	float genDelay = 0.0;

	subckts[index].cleanDangling(true);
	subckts[index].combineDevices();
	subckts[index].canonicalize();

	phy::Layout macro(tech);
	if (filesystem::exists(cellPath)) {
		bool imported = import_layout(macro, cellPath, subckts[index].name);
		if (progress) {
			if (imported) {
				macro.trace();
				sch::Subckt gdsNet(true);
				extract(gdsNet, macro, true);
				gdsNet.cleanDangling(true);
				gdsNet.combineDevices();
				gdsNet.canonicalize();
				searchDelay = tmr.since();
				if (gdsNet.compare(subckts[index]) == 0) {
					printf("%sFOUND %d DBUNIT2 AREA%s]\t%gs\n", KGRN, macro.box.area(), KNRM, searchDelay);
				} else {
					printf("%sFAILED LVS%s, ", KRED, KNRM);
					imported = false;
				}
			} else {
				searchDelay = tmr.since();
				printf("%sFAILED IMPORT%s, ", KRED, KNRM);
			}
		}
		if (imported) {
			macros[index] = macro;
			return true;
		} else {
			macro.clear();
		}
	}

	tmr.reset();

	int result = sch::buildCell(macro, subckts[index]);
	if (progress) {
		if (result == 1) {
			genDelay = tmr.since();
			printf("%sFAILED PLACEMENT%s]\t(%gs %gs)\n", KRED, KNRM, searchDelay, genDelay);
		} else if (result == 2) {
			genDelay = tmr.since();
			printf("%sFAILED ROUTING%s]\t(%gs %gs)\n", KRED, KNRM, searchDelay, genDelay);
		} else {
			sch::Subckt gdsNet(true);
			extract(gdsNet, macro, true);
			gdsNet.cleanDangling(true);
			gdsNet.combineDevices();
			gdsNet.canonicalize();

			genDelay = tmr.since();
			if (gdsNet.compare(subckts[index]) == 0) {
				printf("%sGENERATED %d DBUNIT2 AREA%s]\t(%gs %gs)\n", KGRN, macro.box.area(), KNRM, searchDelay, genDelay);
				macros[index] = macro;
			} else {
				printf("%sFAILED LVS%s]\t(%gs %gs)\n", KRED, KNRM, searchDelay, genDelay);
				if (debug) {
					gdsNet.print();
					subckts[index].print();
				}
			}
		}
	}
	return false;
}

void update_library(std::string path, const phy::Tech &tech, std::vector<sch::Subckt> &subckts, std::vector<phy::Layout> &macros, bool progress, bool debug) {
	bool libFound = filesystem::exists(path);
	if (progress) {
		printf("Load cell layouts:\n");
	}

	Timer tmr;
	for (size_t i = 0; i < subckts.size(); i++) {
		if (subckts[i].isCell) {
			
			if (not import_cell(path, tech, subckts, macros, i, progress, debug)) {
				// We generated a new cell, save this to the cell library
				if (not libFound) {
					filesystem::create_directory(path);
					libFound = true;
				}
				export_cell(path, tech, subckts[i], macros[i]);
			}
		}
	}
	if (progress) {
		printf("done\t%gs\n\n", tmr.since());
	}
}

}

bool mapCells(const phy::Tech &tech, std::vector<sch::Subckt> &subckts, std::vector<phy::Layout> &macros, int index, bool progress) {
	sch::Subckt &ckt = subckts[index];
	if (ckt.isCell) {
		return true;
	}

	vector<sch::Subckt> cells = sch::mapCells(tech, ckt, progress);
	for (sch::Subckt &cell : cells) {
		/*std::string originalName = cell.name;
		cell.name = "cell_" + encodeBase32(cell.id);
		for (auto i = ckt.inst.begin(); i != ckt.inst.end(); i++) {
			if (i->type == originalName) {
				i->type = cell.name;
			}
		}*/
		bool found = false;
		for (size_t i = 0; i < subckts.size(); i++) {
			if (subckts[i].name == cell.name) {
				found = true;
				break;
			}
		}
		if (not found) {
			subckts.push_back(cell);
			macros.push_back(Layout(tech, cell.name));
		}
	}
	return true;
}

struct SchLinker : sch::Linker {
	const phy::Tech &tech;
	vector<sch::Subckt> &subckts;
	vector<phy::Layout> &macros;

	SchLinker(const phy::Tech &tech, vector<sch::Subckt> &subckts, vector<phy::Layout> &macros) : tech(tech), subckts(subckts), macros(macros) {
	}
	~SchLinker() {}

	sch::Implementation find(const sch::Instance &inst) override {
		int index = -1;
		for (size_t i = 0; i < subckts.size(); i++) {
			if (subckts[i].name == inst.type) {
				index = (int)i;
				break;
			}
		}
		sch::Implementation result;
		if (index < 0) {
			error("", "unable to link '" + inst.type + "'", __FILE__, __LINE__);
			return result;
		}
		
		result.ckt = &subckts[index];
		result.macro = &macros[index];
		if (result.ckt != nullptr and result.macro != nullptr) {
			result.cktToMacro = result.ckt->mapToLayout(*result.macro);
		}
		return result;
	}
};

bool spiToGds(std::string path, const phy::Tech &tech, std::vector<sch::Subckt> &subckts, std::vector<phy::Layout> &macros, int index, bool progress, bool debug) {
	sch::Subckt &ckt = subckts[index];
	if (ckt.inst.empty() and not ckt.mos.empty()) {
		if (not cell::import_cell(path, tech, subckts, macros, index, progress, debug)) {
			// We generated a new cell, save this to the cell library
			if (not filesystem::exists(path)) {
				filesystem::create_directory(path);
			}
			cell::export_cell(path, tech, subckts[index], macros[index]);
		}
		return true;
	} else if (ckt.mos.empty() and not ckt.inst.empty()) {
		if (progress) {
			printf("Placing %s...", ckt.name.c_str());
			fflush(stdout);
		}
		Timer tmr;
		phy::Layout &macro = macros[index];

		SchLinker linker(tech, subckts, macros);
		sch::Placer placer(&linker, 0, 0, true, false);
		placer.load(sch::Implementation(&ckt, &macro));
		sch::Placement prob(placer, 0);
		prob.solve();
		prob.save(macro);
		if (progress) {
			printf("[%sDONE%s]\t%gs\n", KGRN, KNRM, tmr.since());
		}
		return true;
	} else if (ckt.mos.empty() and ckt.inst.empty()) {
		warning("", "found blackbox \"" + ckt.name + "\"", __FILE__, __LINE__);
		return true;
	} else {
		error("", "no support for mixed macro/micro layout in \"" + ckt.name + "\"", __FILE__, __LINE__);
		return false;
	}
	return false;
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
	if (not phy::loadTech(&tech, techPath, vector<std::string>())) {
		cout << "Unable to load techfile \'" + techPath + "\'." << endl;
		return 1;
	}
	std::vector<sch::Subckt> subckts;
	std::vector<phy::Layout> macros;

	if (format == "spi") {
		tokenizer tokens;
		parse_spice::netlist::register_syntax(tokens);

		tokens.insert(filename, readFile(filename));

		tokens.increment(false);
		tokens.expect<parse_spice::netlist>();
		if (tokens.decrement(__FILE__, __LINE__))
		{
			parse_spice::netlist syntax(tokens);
			sch::import_netlist(tech, subckts, syntax, &tokens);
		}
	}

	if (noCells) {
		for (int i = 0; i < (int)subckts.size(); i++) {
			subckts[i].isCell = true;
		}
	}

	if (progress) printf("Break subckts into cells:\n");
	Timer cellsTmr;
	for (size_t i = 0; i < subckts.size(); i++) {
		macros.push_back(phy::Layout(tech, subckts[i].name));
	}
	for (int i = subckts.size()-1; i >= 0; i--) {
		mapCells(tech, subckts, macros, i, progress);
	}
	if (progress) printf("done\t%gs\n\n", cellsTmr.since());

	if (format != "spi") {
		// This spice netlist is always exported
		FILE *fout = stdout;
		if (prefix != "") {
			fout = fopen((prefix+".spi").c_str(), "w");
		}
		fprintf(fout, "%s", sch::export_netlist(tech, subckts).to_string().c_str());
		fclose(fout);
	}

	cell::update_library(cellsDir, tech, subckts, macros, progress, debug);

	for (size_t i = 0; i < subckts.size(); i++) {
		spiToGds(cellsDir, tech, subckts, macros, i, progress, debug);
	}

	gdstk::GdsWriter gds = {};
	gds = gdstk::gdswriter_init((prefix+".gds").c_str(), prefix.c_str(), ((double)tech.dbunit)*1e-6, ((double)tech.dbunit)*1e-6, 4, nullptr, nullptr);

	for (size_t i = 0; i < macros.size(); i++) {
		phy::export_layout(gds, macros[i]);
	}

	gds.close();

	if (progress) printf("compiled in %gs\n\n", timer.since());

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

