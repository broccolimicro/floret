#include "Common.h"
#include <utility>
#include <math.h>
#include <string>
#include <stdlib.h>

using namespace std;

string lower(string str) {
	for (auto c = str.begin(); c != str.end(); c++) {
		*c = tolower(*c);
	}
	return str;
}

