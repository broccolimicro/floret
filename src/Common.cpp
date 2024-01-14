#include "Common.h"
#include <utility>
#include <math.h>
#include <string>

using namespace std;

string lower(string str) {
	for (auto c = str.begin(); c != str.end(); c++) {
		*c = tolower(*c);
	}
	return str;
}

double loadValue(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &val) {
	string constStr = "0.0";
	if (val.tokens.size() == 0) {
		constStr = lexer.read(val.begin, val.end);
	} else {
		constStr = lexer.read(val.tokens[0].begin, val.tokens[0].end);
	}

	string unitStr = "";
	if (val.tokens.size() == 2) {
		unitStr = lower(lexer.read(val.tokens[1].begin, val.tokens[1].end));
	}

	double value = stod(constStr);
	if (unitStr.size() == 0) {
		return value;
	}

	int unit = (int)(string("afpnumkxg").find(unitStr));
	if (unit >= 0) {
		int exp = 3*unit - 18;
		value *= pow(10.0, (double)exp);
	}

	return value;
}
