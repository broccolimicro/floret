#pragma once

#include <sch/Circuit.h>
#include <sch/Library.h>

#include <string>

#include "spice.h"

using namespace std;

string lower(string str);
double loadValue(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &val);
bool loadDevice(Circuit &ckt, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev);
void loadSubckt(Circuit &ckt, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt);
void loadSpice(Library &lib, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice);
bool loadFile(Library &lib, string path);
