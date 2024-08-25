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

