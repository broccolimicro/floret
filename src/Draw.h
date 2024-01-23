#pragma once

#include <ruler/Tech.h>
#include <ruler/Layout.h>
#include <ruler/vector.h>

#include <iostream>

#include "Circuit.h"
#include "Solution.h"

using namespace ruler;

void drawTransistor(const Tech &tech, Layout &dst, const Mos &mos, vec2i pos=vec2i(0,0), vec2i dir=vec2i(1,1)); 
void drawVia(const Tech &tech, Layout &dst, int net, int downLevel, int upLevel, vec2i size=vec2i(0,0), vec2i pos=vec2i(0,0), vec2i dir=vec2i(1,1));
void drawWire(const Tech &tech, Layout &dst, const Solution *ckt, const Wire &wire, vec2i pos, vec2i dir);
void drawPin(const Tech &tech, Layout &dst, const Solution *ckt, int type, int pinID, vec2i pos=vec2i(0,0), vec2i dir=vec2i(1,1));
void drawCell(const Tech &tech, Layout &dst, const Solution *ckt);
void drawLayout(const Tech &tech, Layout &dst, const Layout &src, vec2i pos, vec2i dir);
