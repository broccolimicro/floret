#pragma once

#include <floret/Circuit.h>
#include <floret/Library.h>

#include "spice.h"

double loadValue(pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &val);
bool loadDevice(Circuit &ckt, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &dev);
void loadSubckt(Circuit &ckt, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &subckt);
void loadSpice(Library &lib, pgen::spice_t lang, pgen::lexer_t &lexer, pgen::token_t &spice);
bool loadFile(Library &lib, string path);