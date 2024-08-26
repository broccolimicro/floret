#pragma once

#include <sch/Subckt.h>
#include <sch/Netlist.h>

#include <string>

#include "spice.h"

using namespace std;
using namespace sch;

string lower(string str);
double loadValue(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &val);
bool loadDevice(Subckt &ckt, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev);
void loadSubckt(Subckt &ckt, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt);
void loadSpice(Netlist &lib, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice);
bool loadFile(Netlist &lib, string path);
