#pragma once

#include <string>
#include "spice.h"

using namespace std;

string lower(string str);
double loadValue(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &val);
