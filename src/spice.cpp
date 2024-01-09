#include "spice.h"

#include <pgen/default.h>

namespace pgen
{

void spice_t::load(grammar_t &grammar)
{
	SPICE = grammar.rules.size();
	grammar.rules.push_back(rule_t(SPICE, "spice::spice", true, true));
	__ = grammar.rules.size();
	grammar.rules.push_back(rule_t(__, "spice::__", false, true));
	STATEMENT = grammar.rules.size();
	grammar.rules.push_back(rule_t(STATEMENT, "spice::statement", true, true));
	END_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(END_K, "spice::end_k", false, true));
	SUBCKT = grammar.rules.size();
	grammar.rules.push_back(rule_t(SUBCKT, "spice::subckt", true, true));
	MODEL = grammar.rules.size();
	grammar.rules.push_back(rule_t(MODEL, "spice::model", true, true));
	OPTIONS = grammar.rules.size();
	grammar.rules.push_back(rule_t(OPTIONS, "spice::options", true, true));
	NODESET = grammar.rules.size();
	grammar.rules.push_back(rule_t(NODESET, "spice::nodeset", true, true));
	IC = grammar.rules.size();
	grammar.rules.push_back(rule_t(IC, "spice::ic", true, true));
	SUBCKT_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(SUBCKT_K, "spice::subckt_k", false, true));
	_ = grammar.rules.size();
	grammar.rules.push_back(rule_t(_, "spice::_", false, true));
	NAME = grammar.rules.size();
	grammar.rules.push_back(rule_t(NAME, "spice::name", true, true));
	DEVICE = grammar.rules.size();
	grammar.rules.push_back(rule_t(DEVICE, "spice::device", true, true));
	ENDS_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(ENDS_K, "spice::ends_k", false, true));
	R = grammar.rules.size();
	grammar.rules.push_back(rule_t(R, "spice::R", true, true));
	C = grammar.rules.size();
	grammar.rules.push_back(rule_t(C, "spice::C", true, true));
	L = grammar.rules.size();
	grammar.rules.push_back(rule_t(L, "spice::L", true, true));
	K = grammar.rules.size();
	grammar.rules.push_back(rule_t(K, "spice::K", true, true));
	D = grammar.rules.size();
	grammar.rules.push_back(rule_t(D, "spice::D", true, true));
	Q = grammar.rules.size();
	grammar.rules.push_back(rule_t(Q, "spice::Q", true, true));
	J = grammar.rules.size();
	grammar.rules.push_back(rule_t(J, "spice::J", true, true));
	V = grammar.rules.size();
	grammar.rules.push_back(rule_t(V, "spice::V", true, true));
	E = grammar.rules.size();
	grammar.rules.push_back(rule_t(E, "spice::E", true, true));
	F = grammar.rules.size();
	grammar.rules.push_back(rule_t(F, "spice::F", true, true));
	G = grammar.rules.size();
	grammar.rules.push_back(rule_t(G, "spice::G", true, true));
	H = grammar.rules.size();
	grammar.rules.push_back(rule_t(H, "spice::H", true, true));
	M = grammar.rules.size();
	grammar.rules.push_back(rule_t(M, "spice::M", true, true));
	NODE = grammar.rules.size();
	grammar.rules.push_back(rule_t(NODE, "spice::node", true, true));
	VALUE = grammar.rules.size();
	grammar.rules.push_back(rule_t(VALUE, "spice::value", true, true));
	PARAM = grammar.rules.size();
	grammar.rules.push_back(rule_t(PARAM, "spice::param", true, true));
	I = grammar.rules.size();
	grammar.rules.push_back(rule_t(I, "spice::I", true, true));
	MODEL_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(MODEL_K, "spice::model_k", false, true));
	OPTIONS_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(OPTIONS_K, "spice::options_k", false, true));
	NODESET_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(NODESET_K, "spice::nodeset_k", false, true));
	INITIAL = grammar.rules.size();
	grammar.rules.push_back(rule_t(INITIAL, "spice::initial", true, true));
	IC_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(IC_K, "spice::ic_k", false, true));
	MEASURE = grammar.rules.size();
	grammar.rules.push_back(rule_t(MEASURE, "spice::measure", true, true));
	CONSTANT = grammar.rules.size();
	grammar.rules.push_back(rule_t(CONSTANT, "spice::constant", true, true));
	INTEGER = grammar.rules.size();
	grammar.rules.push_back(rule_t(INTEGER, "spice::integer", true, true));
	UNIT = grammar.rules.size();
	grammar.rules.push_back(rule_t(UNIT, "spice::unit", true, true));
	BOOL = grammar.rules.size();
	grammar.rules.push_back(rule_t(BOOL, "spice::bool", true, true));
	INT = grammar.rules.size();
	grammar.rules.push_back(rule_t(INT, "spice::int", true, true));
	TEXT = grammar.rules.size();
	grammar.rules.push_back(rule_t(TEXT, "spice::text", true, true));
	REAL = grammar.rules.size();
	grammar.rules.push_back(rule_t(REAL, "spice::real", true, true));
	INTEGER16 = grammar.rules.size();
	grammar.rules.push_back(rule_t(INTEGER16, "spice::integer16", true, true));
	INTEGER2 = grammar.rules.size();
	grammar.rules.push_back(rule_t(INTEGER2, "spice::integer2", true, true));

	symbol_t *n[340];
	n[0] = grammar.insert(new regular_expression("[ \\t]*(\\*[^\\n\\r]*)?[\\n\\r]+", true));
	n[1] = grammar.insert(new regular_expression("[ \\t]*", true));
	n[2] = grammar.insert(new regular_expression("\\\"([^\\\"\\\\]|\\\\.)*\\\"", false));
	n[3] = grammar.insert(new regular_expression("[0-9]+", false));
	n[4] = grammar.insert(new regular_expression("[0-9a-fA-F]+", false));
	n[5] = grammar.insert(new regular_expression("[01]+", false));
	n[6] = grammar.insert(new regular_expression("false", false));
	n[7] = grammar.insert(new regular_expression("true", false));
	n[8] = grammar.insert(new stem(38, true));
	n[9] = grammar.insert(new regular_expression("-", true));
	n[10] = grammar.insert(new regular_expression("e", true));
	n[11] = grammar.insert(new stem(38, true));
	n[12] = grammar.insert(new regular_expression(".", true));
	n[13] = grammar.insert(new stem(38, true));
	n[14] = grammar.insert(new stem(38, true));
	n[15] = grammar.insert(new stem(45, true));
	n[16] = grammar.insert(new regular_expression("0b", false));
	n[17] = grammar.insert(new stem(44, true));
	n[18] = grammar.insert(new regular_expression("0x", false));
	n[19] = grammar.insert(new stem(43, true));
	n[20] = grammar.insert(new stem(42, true));
	n[21] = grammar.insert(new stem(41, true));
	n[22] = grammar.insert(new stem(40, true));
	n[23] = grammar.insert(new regular_expression("[aAfFpPnNuUmMkKxXgG]", false));
	n[24] = grammar.insert(new stem(39, true));
	n[25] = grammar.insert(new stem(37, true));
	n[26] = grammar.insert(new regular_expression("[#_a-zA-Z][_a-zA-Z0-9]*", true));
	n[27] = grammar.insert(new stem(11, true));
	n[28] = grammar.insert(new stem(38, true));
	n[29] = grammar.insert(new stem(10, true));
	n[30] = grammar.insert(new stem(37, true));
	n[31] = grammar.insert(new stem(10, true));
	n[32] = grammar.insert(new regular_expression(",", true));
	n[33] = grammar.insert(new stem(10, true));
	n[34] = grammar.insert(new stem(37, true));
	n[35] = grammar.insert(new stem(10, true));
	n[36] = grammar.insert(new regular_expression("=", true));
	n[37] = grammar.insert(new stem(10, true));
	n[38] = grammar.insert(new stem(11, true));
	n[39] = grammar.insert(new regular_expression("\\)", false));
	n[40] = grammar.insert(new stem(27, true));
	n[41] = grammar.insert(new regular_expression("\\(", false));
	n[42] = grammar.insert(new regular_expression("[vViI]", true));
	n[43] = grammar.insert(new stem(10, true));
	n[44] = grammar.insert(new stem(37, true));
	n[45] = grammar.insert(new stem(10, true));
	n[46] = grammar.insert(new regular_expression("=", true));
	n[47] = grammar.insert(new stem(10, true));
	n[48] = grammar.insert(new stem(36, true));
	n[49] = grammar.insert(new regular_expression("\\.IC", true));
	n[50] = grammar.insert(new regular_expression("\\.ic", true));
	n[51] = grammar.insert(new stem(1, true));
	n[52] = grammar.insert(new stem(34, true));
	n[53] = grammar.insert(new stem(10, true));
	n[54] = grammar.insert(new stem(35, true));
	n[55] = grammar.insert(new regular_expression("\\.NODESET", true));
	n[56] = grammar.insert(new regular_expression("\\.nodeset", true));
	n[57] = grammar.insert(new stem(1, true));
	n[58] = grammar.insert(new stem(34, true));
	n[59] = grammar.insert(new stem(10, true));
	n[60] = grammar.insert(new stem(33, true));
	n[61] = grammar.insert(new regular_expression("\\.OPTIONS", true));
	n[62] = grammar.insert(new regular_expression("\\.options", true));
	n[63] = grammar.insert(new stem(1, true));
	n[64] = grammar.insert(new stem(29, true));
	n[65] = grammar.insert(new stem(10, true));
	n[66] = grammar.insert(new stem(32, true));
	n[67] = grammar.insert(new regular_expression("\\.MODEL", true));
	n[68] = grammar.insert(new regular_expression("\\.model", true));
	n[69] = grammar.insert(new stem(1, true));
	n[70] = grammar.insert(new stem(29, true));
	n[71] = grammar.insert(new stem(10, true));
	n[72] = grammar.insert(new stem(11, true));
	n[73] = grammar.insert(new stem(10, true));
	n[74] = grammar.insert(new stem(11, true));
	n[75] = grammar.insert(new stem(10, true));
	n[76] = grammar.insert(new stem(31, true));
	n[77] = grammar.insert(new stem(1, true));
	n[78] = grammar.insert(new stem(29, true));
	n[79] = grammar.insert(new stem(10, true));
	n[80] = grammar.insert(new stem(28, true));
	n[81] = grammar.insert(new stem(29, true));
	n[82] = grammar.insert(new stem(10, true));
	n[83] = grammar.insert(new stem(11, true));
	n[84] = grammar.insert(new stem(10, true));
	n[85] = grammar.insert(new stem(27, true));
	n[86] = grammar.insert(new stem(10, true));
	n[87] = grammar.insert(new stem(27, true));
	n[88] = grammar.insert(new stem(10, true));
	n[89] = grammar.insert(new stem(27, true));
	n[90] = grammar.insert(new stem(10, true));
	n[91] = grammar.insert(new stem(27, true));
	n[92] = grammar.insert(new stem(10, true));
	n[93] = grammar.insert(new stem(11, true));
	n[94] = grammar.insert(new regular_expression("[mM]", true));
	n[95] = grammar.insert(new stem(1, true));
	n[96] = grammar.insert(new stem(28, true));
	n[97] = grammar.insert(new stem(10, true));
	n[98] = grammar.insert(new stem(11, true));
	n[99] = grammar.insert(new stem(10, true));
	n[100] = grammar.insert(new stem(27, true));
	n[101] = grammar.insert(new stem(10, true));
	n[102] = grammar.insert(new stem(27, true));
	n[103] = grammar.insert(new stem(10, true));
	n[104] = grammar.insert(new stem(11, true));
	n[105] = grammar.insert(new regular_expression("[hH]", true));
	n[106] = grammar.insert(new stem(1, true));
	n[107] = grammar.insert(new stem(28, true));
	n[108] = grammar.insert(new stem(10, true));
	n[109] = grammar.insert(new stem(27, true));
	n[110] = grammar.insert(new stem(10, true));
	n[111] = grammar.insert(new stem(27, true));
	n[112] = grammar.insert(new stem(10, true));
	n[113] = grammar.insert(new stem(27, true));
	n[114] = grammar.insert(new stem(10, true));
	n[115] = grammar.insert(new stem(27, true));
	n[116] = grammar.insert(new stem(10, true));
	n[117] = grammar.insert(new stem(11, true));
	n[118] = grammar.insert(new regular_expression("[gG]", true));
	n[119] = grammar.insert(new stem(1, true));
	n[120] = grammar.insert(new stem(28, true));
	n[121] = grammar.insert(new stem(10, true));
	n[122] = grammar.insert(new stem(11, true));
	n[123] = grammar.insert(new stem(10, true));
	n[124] = grammar.insert(new stem(27, true));
	n[125] = grammar.insert(new stem(10, true));
	n[126] = grammar.insert(new stem(27, true));
	n[127] = grammar.insert(new stem(10, true));
	n[128] = grammar.insert(new stem(11, true));
	n[129] = grammar.insert(new regular_expression("[fF]", true));
	n[130] = grammar.insert(new stem(1, true));
	n[131] = grammar.insert(new stem(28, true));
	n[132] = grammar.insert(new stem(10, true));
	n[133] = grammar.insert(new stem(27, true));
	n[134] = grammar.insert(new stem(10, true));
	n[135] = grammar.insert(new stem(27, true));
	n[136] = grammar.insert(new stem(10, true));
	n[137] = grammar.insert(new stem(27, true));
	n[138] = grammar.insert(new stem(10, true));
	n[139] = grammar.insert(new stem(27, true));
	n[140] = grammar.insert(new stem(10, true));
	n[141] = grammar.insert(new stem(11, true));
	n[142] = grammar.insert(new regular_expression("[eE]", true));
	n[143] = grammar.insert(new stem(10, true));
	n[144] = grammar.insert(new stem(28, true));
	n[145] = grammar.insert(new stem(10, true));
	n[146] = grammar.insert(new stem(28, true));
	n[147] = grammar.insert(new regular_expression("DISTOF2", true));
	n[148] = grammar.insert(new regular_expression("distof2", true));
	n[149] = grammar.insert(new stem(10, true));
	n[150] = grammar.insert(new stem(28, true));
	n[151] = grammar.insert(new stem(10, true));
	n[152] = grammar.insert(new stem(28, true));
	n[153] = grammar.insert(new regular_expression("DISTOF1", true));
	n[154] = grammar.insert(new regular_expression("distof1", true));
	n[155] = grammar.insert(new stem(10, true));
	n[156] = grammar.insert(new stem(28, true));
	n[157] = grammar.insert(new stem(10, true));
	n[158] = grammar.insert(new stem(28, true));
	n[159] = grammar.insert(new stem(10, true));
	n[160] = grammar.insert(new regular_expression("AC", true));
	n[161] = grammar.insert(new stem(10, true));
	n[162] = grammar.insert(new regular_expression("ac", true));
	n[163] = grammar.insert(new stem(28, true));
	n[164] = grammar.insert(new stem(10, true));
	n[165] = grammar.insert(new regular_expression("DC", true));
	n[166] = grammar.insert(new stem(10, true));
	n[167] = grammar.insert(new regular_expression("dc", true));
	n[168] = grammar.insert(new stem(10, true));
	n[169] = grammar.insert(new stem(27, true));
	n[170] = grammar.insert(new stem(10, true));
	n[171] = grammar.insert(new stem(27, true));
	n[172] = grammar.insert(new stem(10, true));
	n[173] = grammar.insert(new stem(11, true));
	n[174] = grammar.insert(new regular_expression("[iI]", true));
	n[175] = grammar.insert(new stem(10, true));
	n[176] = grammar.insert(new stem(28, true));
	n[177] = grammar.insert(new stem(10, true));
	n[178] = grammar.insert(new stem(28, true));
	n[179] = grammar.insert(new regular_expression("DISTOF2", true));
	n[180] = grammar.insert(new regular_expression("distof2", true));
	n[181] = grammar.insert(new stem(10, true));
	n[182] = grammar.insert(new stem(28, true));
	n[183] = grammar.insert(new stem(10, true));
	n[184] = grammar.insert(new stem(28, true));
	n[185] = grammar.insert(new regular_expression("DISTOF1", true));
	n[186] = grammar.insert(new regular_expression("distof1", true));
	n[187] = grammar.insert(new stem(10, true));
	n[188] = grammar.insert(new stem(28, true));
	n[189] = grammar.insert(new stem(10, true));
	n[190] = grammar.insert(new stem(28, true));
	n[191] = grammar.insert(new stem(10, true));
	n[192] = grammar.insert(new regular_expression("AC", true));
	n[193] = grammar.insert(new stem(10, true));
	n[194] = grammar.insert(new regular_expression("ac", true));
	n[195] = grammar.insert(new stem(28, true));
	n[196] = grammar.insert(new stem(10, true));
	n[197] = grammar.insert(new regular_expression("DC", true));
	n[198] = grammar.insert(new stem(10, true));
	n[199] = grammar.insert(new regular_expression("dc", true));
	n[200] = grammar.insert(new stem(10, true));
	n[201] = grammar.insert(new stem(27, true));
	n[202] = grammar.insert(new stem(10, true));
	n[203] = grammar.insert(new stem(27, true));
	n[204] = grammar.insert(new stem(10, true));
	n[205] = grammar.insert(new stem(11, true));
	n[206] = grammar.insert(new regular_expression("[vV]", true));
	n[207] = grammar.insert(new stem(1, true));
	n[208] = grammar.insert(new stem(29, true));
	n[209] = grammar.insert(new stem(10, true));
	n[210] = grammar.insert(new stem(28, true));
	n[211] = grammar.insert(new stem(10, true));
	n[212] = grammar.insert(new stem(28, true));
	n[213] = grammar.insert(new stem(27, true));
	n[214] = grammar.insert(new stem(10, true));
	n[215] = grammar.insert(new stem(27, true));
	n[216] = grammar.insert(new stem(10, true));
	n[217] = grammar.insert(new stem(27, true));
	n[218] = grammar.insert(new stem(10, true));
	n[219] = grammar.insert(new stem(27, true));
	n[220] = grammar.insert(new stem(10, true));
	n[221] = grammar.insert(new stem(11, true));
	n[222] = grammar.insert(new regular_expression("[jJ]", true));
	n[223] = grammar.insert(new stem(1, true));
	n[224] = grammar.insert(new stem(29, true));
	n[225] = grammar.insert(new stem(10, true));
	n[226] = grammar.insert(new stem(28, true));
	n[227] = grammar.insert(new stem(10, true));
	n[228] = grammar.insert(new stem(28, true));
	n[229] = grammar.insert(new stem(10, true));
	n[230] = grammar.insert(new stem(27, true));
	n[231] = grammar.insert(new stem(10, true));
	n[232] = grammar.insert(new stem(27, true));
	n[233] = grammar.insert(new stem(10, true));
	n[234] = grammar.insert(new stem(27, true));
	n[235] = grammar.insert(new stem(10, true));
	n[236] = grammar.insert(new stem(27, true));
	n[237] = grammar.insert(new stem(10, true));
	n[238] = grammar.insert(new stem(27, true));
	n[239] = grammar.insert(new stem(10, true));
	n[240] = grammar.insert(new stem(11, true));
	n[241] = grammar.insert(new regular_expression("[qQ]", true));
	n[242] = grammar.insert(new stem(1, true));
	n[243] = grammar.insert(new stem(29, true));
	n[244] = grammar.insert(new stem(10, true));
	n[245] = grammar.insert(new stem(28, true));
	n[246] = grammar.insert(new stem(10, true));
	n[247] = grammar.insert(new stem(28, true));
	n[248] = grammar.insert(new stem(10, true));
	n[249] = grammar.insert(new stem(27, true));
	n[250] = grammar.insert(new stem(10, true));
	n[251] = grammar.insert(new stem(27, true));
	n[252] = grammar.insert(new stem(10, true));
	n[253] = grammar.insert(new stem(27, true));
	n[254] = grammar.insert(new stem(10, true));
	n[255] = grammar.insert(new stem(11, true));
	n[256] = grammar.insert(new regular_expression("[dD]", true));
	n[257] = grammar.insert(new stem(1, true));
	n[258] = grammar.insert(new stem(29, true));
	n[259] = grammar.insert(new stem(10, true));
	n[260] = grammar.insert(new stem(28, true));
	n[261] = grammar.insert(new stem(10, true));
	n[262] = grammar.insert(new stem(27, true));
	n[263] = grammar.insert(new regular_expression("[lL]", true));
	n[264] = grammar.insert(new stem(10, true));
	n[265] = grammar.insert(new stem(27, true));
	n[266] = grammar.insert(new regular_expression("[lL]", true));
	n[267] = grammar.insert(new stem(10, true));
	n[268] = grammar.insert(new stem(11, true));
	n[269] = grammar.insert(new regular_expression("[kK]", true));
	n[270] = grammar.insert(new stem(1, true));
	n[271] = grammar.insert(new stem(29, true));
	n[272] = grammar.insert(new stem(10, true));
	n[273] = grammar.insert(new stem(28, true));
	n[274] = grammar.insert(new stem(10, true));
	n[275] = grammar.insert(new stem(27, true));
	n[276] = grammar.insert(new stem(10, true));
	n[277] = grammar.insert(new stem(27, true));
	n[278] = grammar.insert(new stem(10, true));
	n[279] = grammar.insert(new stem(11, true));
	n[280] = grammar.insert(new regular_expression("[lL]", true));
	n[281] = grammar.insert(new stem(1, true));
	n[282] = grammar.insert(new stem(29, true));
	n[283] = grammar.insert(new stem(10, true));
	n[284] = grammar.insert(new stem(28, true));
	n[285] = grammar.insert(new stem(10, true));
	n[286] = grammar.insert(new stem(27, true));
	n[287] = grammar.insert(new stem(10, true));
	n[288] = grammar.insert(new stem(11, true));
	n[289] = grammar.insert(new regular_expression("[cC]", true));
	n[290] = grammar.insert(new stem(1, true));
	n[291] = grammar.insert(new stem(29, true));
	n[292] = grammar.insert(new stem(10, true));
	n[293] = grammar.insert(new stem(28, true));
	n[294] = grammar.insert(new stem(10, true));
	n[295] = grammar.insert(new stem(27, true));
	n[296] = grammar.insert(new stem(10, true));
	n[297] = grammar.insert(new stem(27, true));
	n[298] = grammar.insert(new stem(10, true));
	n[299] = grammar.insert(new stem(11, true));
	n[300] = grammar.insert(new regular_expression("[rR]", true));
	n[301] = grammar.insert(new stem(26, true));
	n[302] = grammar.insert(new stem(25, true));
	n[303] = grammar.insert(new stem(24, true));
	n[304] = grammar.insert(new stem(23, true));
	n[305] = grammar.insert(new stem(22, true));
	n[306] = grammar.insert(new stem(21, true));
	n[307] = grammar.insert(new stem(20, true));
	n[308] = grammar.insert(new stem(19, true));
	n[309] = grammar.insert(new stem(18, true));
	n[310] = grammar.insert(new stem(17, true));
	n[311] = grammar.insert(new stem(16, true));
	n[312] = grammar.insert(new stem(15, true));
	n[313] = grammar.insert(new stem(14, true));
	n[314] = grammar.insert(new regular_expression("\\.ENDS", true));
	n[315] = grammar.insert(new regular_expression("\\.ends", true));
	n[316] = grammar.insert(new regular_expression("\\.SUBCKT", true));
	n[317] = grammar.insert(new regular_expression("\\.subckt", true));
	n[318] = grammar.insert(new stem(1, true));
	n[319] = grammar.insert(new stem(13, true));
	n[320] = grammar.insert(new stem(1, true));
	n[321] = grammar.insert(new stem(12, true));
	n[322] = grammar.insert(new stem(1, true));
	n[323] = grammar.insert(new stem(11, true));
	n[324] = grammar.insert(new stem(10, true));
	n[325] = grammar.insert(new stem(11, true));
	n[326] = grammar.insert(new stem(10, true));
	n[327] = grammar.insert(new stem(9, true));
	n[328] = grammar.insert(new stem(8, true));
	n[329] = grammar.insert(new stem(7, true));
	n[330] = grammar.insert(new stem(6, true));
	n[331] = grammar.insert(new stem(5, true));
	n[332] = grammar.insert(new stem(4, true));
	n[333] = grammar.insert(new regular_expression("\\.END", true));
	n[334] = grammar.insert(new regular_expression("\\.end", true));
	n[335] = grammar.insert(new regular_expression("[\\0]", true));
	n[336] = grammar.insert(new stem(1, true));
	n[337] = grammar.insert(new stem(3, true));
	n[338] = grammar.insert(new stem(2, true));
	n[339] = grammar.insert(new stem(1, true));

	n[0]->next.push_back(NULL);
	n[1]->next.push_back(NULL);
	n[2]->next.push_back(NULL);
	n[3]->next.push_back(NULL);
	n[4]->next.push_back(NULL);
	n[5]->next.push_back(NULL);
	n[6]->next.push_back(NULL);
	n[7]->next.push_back(NULL);
	n[8]->next.push_back(NULL);
	n[9]->next.push_back(n[8]);
	n[10]->next.push_back(n[9]);
	n[10]->next.push_back(n[8]);
	n[11]->next.push_back(n[10]);
	n[11]->next.push_back(NULL);
	n[12]->next.push_back(n[11]);
	n[13]->next.push_back(n[12]);
	n[13]->next.push_back(n[10]);
	n[13]->next.push_back(NULL);
	n[14]->next.push_back(NULL);
	n[15]->next.push_back(NULL);
	n[16]->next.push_back(n[15]);
	n[17]->next.push_back(NULL);
	n[18]->next.push_back(n[17]);
	n[19]->next.push_back(NULL);
	n[20]->next.push_back(NULL);
	n[21]->next.push_back(NULL);
	n[22]->next.push_back(NULL);
	n[23]->next.push_back(NULL);
	n[24]->next.push_back(NULL);
	n[25]->next.push_back(n[24]);
	n[26]->next.push_back(NULL);
	n[27]->next.push_back(NULL);
	n[28]->next.push_back(NULL);
	n[29]->next.push_back(n[32]);
	n[29]->next.push_back(NULL);
	n[30]->next.push_back(n[29]);
	n[31]->next.push_back(n[30]);
	n[32]->next.push_back(n[31]);
	n[33]->next.push_back(n[32]);
	n[33]->next.push_back(NULL);
	n[34]->next.push_back(n[33]);
	n[35]->next.push_back(n[34]);
	n[36]->next.push_back(n[35]);
	n[37]->next.push_back(n[36]);
	n[38]->next.push_back(n[37]);
	n[39]->next.push_back(NULL);
	n[40]->next.push_back(n[39]);
	n[41]->next.push_back(n[40]);
	n[42]->next.push_back(n[41]);
	n[43]->next.push_back(NULL);
	n[44]->next.push_back(n[43]);
	n[45]->next.push_back(n[44]);
	n[46]->next.push_back(n[45]);
	n[47]->next.push_back(n[46]);
	n[48]->next.push_back(n[47]);
	n[49]->next.push_back(NULL);
	n[50]->next.push_back(NULL);
	n[51]->next.push_back(NULL);
	n[52]->next.push_back(n[52]);
	n[52]->next.push_back(n[51]);
	n[53]->next.push_back(n[52]);
	n[53]->next.push_back(n[51]);
	n[54]->next.push_back(n[53]);
	n[55]->next.push_back(NULL);
	n[56]->next.push_back(NULL);
	n[57]->next.push_back(NULL);
	n[58]->next.push_back(n[58]);
	n[58]->next.push_back(n[57]);
	n[59]->next.push_back(n[58]);
	n[59]->next.push_back(n[57]);
	n[60]->next.push_back(n[59]);
	n[61]->next.push_back(NULL);
	n[62]->next.push_back(NULL);
	n[63]->next.push_back(NULL);
	n[64]->next.push_back(n[64]);
	n[64]->next.push_back(n[63]);
	n[65]->next.push_back(n[64]);
	n[65]->next.push_back(n[63]);
	n[66]->next.push_back(n[65]);
	n[67]->next.push_back(NULL);
	n[68]->next.push_back(NULL);
	n[69]->next.push_back(NULL);
	n[70]->next.push_back(n[70]);
	n[70]->next.push_back(n[69]);
	n[71]->next.push_back(n[70]);
	n[71]->next.push_back(n[69]);
	n[72]->next.push_back(n[71]);
	n[73]->next.push_back(n[72]);
	n[74]->next.push_back(n[73]);
	n[75]->next.push_back(n[74]);
	n[76]->next.push_back(n[75]);
	n[77]->next.push_back(NULL);
	n[78]->next.push_back(n[78]);
	n[78]->next.push_back(n[77]);
	n[79]->next.push_back(n[78]);
	n[79]->next.push_back(n[77]);
	n[80]->next.push_back(n[79]);
	n[81]->next.push_back(n[81]);
	n[81]->next.push_back(n[80]);
	n[81]->next.push_back(n[78]);
	n[81]->next.push_back(n[77]);
	n[82]->next.push_back(n[81]);
	n[82]->next.push_back(n[80]);
	n[82]->next.push_back(n[78]);
	n[82]->next.push_back(n[77]);
	n[83]->next.push_back(n[82]);
	n[84]->next.push_back(n[83]);
	n[85]->next.push_back(n[84]);
	n[86]->next.push_back(n[85]);
	n[87]->next.push_back(n[86]);
	n[88]->next.push_back(n[87]);
	n[89]->next.push_back(n[88]);
	n[90]->next.push_back(n[89]);
	n[91]->next.push_back(n[90]);
	n[92]->next.push_back(n[91]);
	n[93]->next.push_back(n[92]);
	n[94]->next.push_back(n[93]);
	n[95]->next.push_back(NULL);
	n[96]->next.push_back(n[95]);
	n[97]->next.push_back(n[96]);
	n[98]->next.push_back(n[97]);
	n[99]->next.push_back(n[98]);
	n[100]->next.push_back(n[99]);
	n[101]->next.push_back(n[100]);
	n[102]->next.push_back(n[101]);
	n[103]->next.push_back(n[102]);
	n[104]->next.push_back(n[103]);
	n[105]->next.push_back(n[104]);
	n[106]->next.push_back(NULL);
	n[107]->next.push_back(n[106]);
	n[108]->next.push_back(n[107]);
	n[109]->next.push_back(n[108]);
	n[110]->next.push_back(n[109]);
	n[111]->next.push_back(n[110]);
	n[112]->next.push_back(n[111]);
	n[113]->next.push_back(n[112]);
	n[114]->next.push_back(n[113]);
	n[115]->next.push_back(n[114]);
	n[116]->next.push_back(n[115]);
	n[117]->next.push_back(n[116]);
	n[118]->next.push_back(n[117]);
	n[119]->next.push_back(NULL);
	n[120]->next.push_back(n[119]);
	n[121]->next.push_back(n[120]);
	n[122]->next.push_back(n[121]);
	n[123]->next.push_back(n[122]);
	n[124]->next.push_back(n[123]);
	n[125]->next.push_back(n[124]);
	n[126]->next.push_back(n[125]);
	n[127]->next.push_back(n[126]);
	n[128]->next.push_back(n[127]);
	n[129]->next.push_back(n[128]);
	n[130]->next.push_back(NULL);
	n[131]->next.push_back(n[130]);
	n[132]->next.push_back(n[131]);
	n[133]->next.push_back(n[132]);
	n[134]->next.push_back(n[133]);
	n[135]->next.push_back(n[134]);
	n[136]->next.push_back(n[135]);
	n[137]->next.push_back(n[136]);
	n[138]->next.push_back(n[137]);
	n[139]->next.push_back(n[138]);
	n[140]->next.push_back(n[139]);
	n[141]->next.push_back(n[140]);
	n[142]->next.push_back(n[141]);
	n[143]->next.push_back(NULL);
	n[144]->next.push_back(n[143]);
	n[145]->next.push_back(n[144]);
	n[145]->next.push_back(NULL);
	n[146]->next.push_back(n[145]);
	n[147]->next.push_back(n[146]);
	n[147]->next.push_back(NULL);
	n[148]->next.push_back(n[146]);
	n[148]->next.push_back(NULL);
	n[149]->next.push_back(NULL);
	n[150]->next.push_back(n[149]);
	n[151]->next.push_back(n[150]);
	n[151]->next.push_back(NULL);
	n[152]->next.push_back(n[151]);
	n[153]->next.push_back(n[152]);
	n[153]->next.push_back(NULL);
	n[154]->next.push_back(n[152]);
	n[154]->next.push_back(NULL);
	n[155]->next.push_back(NULL);
	n[156]->next.push_back(n[155]);
	n[157]->next.push_back(n[156]);
	n[157]->next.push_back(NULL);
	n[158]->next.push_back(n[157]);
	n[159]->next.push_back(n[158]);
	n[159]->next.push_back(NULL);
	n[160]->next.push_back(n[159]);
	n[161]->next.push_back(n[158]);
	n[161]->next.push_back(NULL);
	n[162]->next.push_back(n[161]);
	n[163]->next.push_back(NULL);
	n[164]->next.push_back(n[163]);
	n[165]->next.push_back(n[164]);
	n[166]->next.push_back(n[163]);
	n[167]->next.push_back(n[166]);
	n[168]->next.push_back(n[167]);
	n[168]->next.push_back(n[165]);
	n[168]->next.push_back(n[163]);
	n[168]->next.push_back(n[162]);
	n[168]->next.push_back(n[160]);
	n[168]->next.push_back(n[154]);
	n[168]->next.push_back(n[153]);
	n[168]->next.push_back(n[148]);
	n[168]->next.push_back(n[147]);
	n[169]->next.push_back(n[168]);
	n[170]->next.push_back(n[169]);
	n[171]->next.push_back(n[170]);
	n[172]->next.push_back(n[171]);
	n[173]->next.push_back(n[172]);
	n[174]->next.push_back(n[173]);
	n[175]->next.push_back(NULL);
	n[176]->next.push_back(n[175]);
	n[177]->next.push_back(n[176]);
	n[177]->next.push_back(NULL);
	n[178]->next.push_back(n[177]);
	n[179]->next.push_back(n[178]);
	n[179]->next.push_back(NULL);
	n[180]->next.push_back(n[178]);
	n[180]->next.push_back(NULL);
	n[181]->next.push_back(NULL);
	n[182]->next.push_back(n[181]);
	n[183]->next.push_back(n[182]);
	n[183]->next.push_back(NULL);
	n[184]->next.push_back(n[183]);
	n[185]->next.push_back(n[184]);
	n[185]->next.push_back(NULL);
	n[186]->next.push_back(n[184]);
	n[186]->next.push_back(NULL);
	n[187]->next.push_back(NULL);
	n[188]->next.push_back(n[187]);
	n[189]->next.push_back(n[188]);
	n[189]->next.push_back(NULL);
	n[190]->next.push_back(n[189]);
	n[191]->next.push_back(n[190]);
	n[191]->next.push_back(NULL);
	n[192]->next.push_back(n[191]);
	n[193]->next.push_back(n[190]);
	n[193]->next.push_back(NULL);
	n[194]->next.push_back(n[193]);
	n[195]->next.push_back(NULL);
	n[196]->next.push_back(n[195]);
	n[197]->next.push_back(n[196]);
	n[198]->next.push_back(n[195]);
	n[199]->next.push_back(n[198]);
	n[200]->next.push_back(n[199]);
	n[200]->next.push_back(n[197]);
	n[200]->next.push_back(n[195]);
	n[200]->next.push_back(n[194]);
	n[200]->next.push_back(n[192]);
	n[200]->next.push_back(n[186]);
	n[200]->next.push_back(n[185]);
	n[200]->next.push_back(n[180]);
	n[200]->next.push_back(n[179]);
	n[201]->next.push_back(n[200]);
	n[202]->next.push_back(n[201]);
	n[203]->next.push_back(n[202]);
	n[204]->next.push_back(n[203]);
	n[205]->next.push_back(n[204]);
	n[206]->next.push_back(n[205]);
	n[207]->next.push_back(NULL);
	n[208]->next.push_back(n[208]);
	n[208]->next.push_back(n[207]);
	n[209]->next.push_back(n[208]);
	n[209]->next.push_back(n[207]);
	n[210]->next.push_back(n[209]);
	n[211]->next.push_back(n[210]);
	n[211]->next.push_back(n[208]);
	n[211]->next.push_back(n[207]);
	n[212]->next.push_back(n[211]);
	n[213]->next.push_back(n[212]);
	n[213]->next.push_back(n[208]);
	n[213]->next.push_back(n[207]);
	n[214]->next.push_back(n[213]);
	n[215]->next.push_back(n[214]);
	n[216]->next.push_back(n[215]);
	n[217]->next.push_back(n[216]);
	n[218]->next.push_back(n[217]);
	n[219]->next.push_back(n[218]);
	n[220]->next.push_back(n[219]);
	n[221]->next.push_back(n[220]);
	n[222]->next.push_back(n[221]);
	n[223]->next.push_back(NULL);
	n[224]->next.push_back(n[224]);
	n[224]->next.push_back(n[223]);
	n[225]->next.push_back(n[224]);
	n[225]->next.push_back(n[223]);
	n[226]->next.push_back(n[225]);
	n[227]->next.push_back(n[226]);
	n[227]->next.push_back(n[224]);
	n[227]->next.push_back(n[223]);
	n[228]->next.push_back(n[227]);
	n[229]->next.push_back(n[228]);
	n[229]->next.push_back(n[224]);
	n[229]->next.push_back(n[223]);
	n[230]->next.push_back(n[229]);
	n[231]->next.push_back(n[230]);
	n[232]->next.push_back(n[231]);
	n[233]->next.push_back(n[232]);
	n[233]->next.push_back(n[230]);
	n[234]->next.push_back(n[233]);
	n[235]->next.push_back(n[234]);
	n[236]->next.push_back(n[235]);
	n[237]->next.push_back(n[236]);
	n[238]->next.push_back(n[237]);
	n[239]->next.push_back(n[238]);
	n[240]->next.push_back(n[239]);
	n[241]->next.push_back(n[240]);
	n[242]->next.push_back(NULL);
	n[243]->next.push_back(n[243]);
	n[243]->next.push_back(n[242]);
	n[244]->next.push_back(n[243]);
	n[244]->next.push_back(n[242]);
	n[245]->next.push_back(n[244]);
	n[246]->next.push_back(n[245]);
	n[246]->next.push_back(n[243]);
	n[246]->next.push_back(n[242]);
	n[247]->next.push_back(n[246]);
	n[248]->next.push_back(n[247]);
	n[248]->next.push_back(n[243]);
	n[248]->next.push_back(n[242]);
	n[249]->next.push_back(n[248]);
	n[250]->next.push_back(n[249]);
	n[251]->next.push_back(n[250]);
	n[252]->next.push_back(n[251]);
	n[253]->next.push_back(n[252]);
	n[254]->next.push_back(n[253]);
	n[255]->next.push_back(n[254]);
	n[256]->next.push_back(n[255]);
	n[257]->next.push_back(NULL);
	n[258]->next.push_back(n[258]);
	n[258]->next.push_back(n[257]);
	n[259]->next.push_back(n[258]);
	n[259]->next.push_back(n[257]);
	n[260]->next.push_back(n[259]);
	n[261]->next.push_back(n[260]);
	n[262]->next.push_back(n[261]);
	n[263]->next.push_back(n[262]);
	n[264]->next.push_back(n[263]);
	n[265]->next.push_back(n[264]);
	n[266]->next.push_back(n[265]);
	n[267]->next.push_back(n[266]);
	n[268]->next.push_back(n[267]);
	n[269]->next.push_back(n[268]);
	n[270]->next.push_back(NULL);
	n[271]->next.push_back(n[271]);
	n[271]->next.push_back(n[270]);
	n[272]->next.push_back(n[271]);
	n[272]->next.push_back(n[270]);
	n[273]->next.push_back(n[272]);
	n[274]->next.push_back(n[273]);
	n[275]->next.push_back(n[274]);
	n[276]->next.push_back(n[275]);
	n[277]->next.push_back(n[276]);
	n[278]->next.push_back(n[277]);
	n[279]->next.push_back(n[278]);
	n[280]->next.push_back(n[279]);
	n[281]->next.push_back(NULL);
	n[282]->next.push_back(n[282]);
	n[282]->next.push_back(n[281]);
	n[283]->next.push_back(n[282]);
	n[283]->next.push_back(n[281]);
	n[284]->next.push_back(n[283]);
	n[285]->next.push_back(n[284]);
	n[286]->next.push_back(n[285]);
	n[287]->next.push_back(n[286]);
	n[288]->next.push_back(n[287]);
	n[289]->next.push_back(n[288]);
	n[290]->next.push_back(NULL);
	n[291]->next.push_back(n[291]);
	n[291]->next.push_back(n[290]);
	n[292]->next.push_back(n[291]);
	n[292]->next.push_back(n[290]);
	n[293]->next.push_back(n[292]);
	n[294]->next.push_back(n[293]);
	n[295]->next.push_back(n[294]);
	n[296]->next.push_back(n[295]);
	n[297]->next.push_back(n[296]);
	n[298]->next.push_back(n[297]);
	n[299]->next.push_back(n[298]);
	n[300]->next.push_back(n[299]);
	n[301]->next.push_back(NULL);
	n[302]->next.push_back(NULL);
	n[303]->next.push_back(NULL);
	n[304]->next.push_back(NULL);
	n[305]->next.push_back(NULL);
	n[306]->next.push_back(NULL);
	n[307]->next.push_back(NULL);
	n[308]->next.push_back(NULL);
	n[309]->next.push_back(NULL);
	n[310]->next.push_back(NULL);
	n[311]->next.push_back(NULL);
	n[312]->next.push_back(NULL);
	n[313]->next.push_back(NULL);
	n[314]->next.push_back(NULL);
	n[315]->next.push_back(NULL);
	n[316]->next.push_back(NULL);
	n[317]->next.push_back(NULL);
	n[318]->next.push_back(NULL);
	n[319]->next.push_back(n[318]);
	n[320]->next.push_back(n[321]);
	n[320]->next.push_back(n[320]);
	n[320]->next.push_back(n[319]);
	n[321]->next.push_back(n[321]);
	n[321]->next.push_back(n[320]);
	n[321]->next.push_back(n[319]);
	n[322]->next.push_back(n[321]);
	n[322]->next.push_back(n[320]);
	n[322]->next.push_back(n[319]);
	n[323]->next.push_back(n[324]);
	n[323]->next.push_back(n[322]);
	n[324]->next.push_back(n[323]);
	n[325]->next.push_back(n[324]);
	n[325]->next.push_back(n[322]);
	n[326]->next.push_back(n[325]);
	n[327]->next.push_back(n[326]);
	n[328]->next.push_back(NULL);
	n[329]->next.push_back(NULL);
	n[330]->next.push_back(NULL);
	n[331]->next.push_back(NULL);
	n[332]->next.push_back(NULL);
	n[333]->next.push_back(NULL);
	n[334]->next.push_back(NULL);
	n[335]->next.push_back(NULL);
	n[336]->next.push_back(n[336]);
	n[336]->next.push_back(n[335]);
	n[337]->next.push_back(n[336]);
	n[337]->next.push_back(n[335]);
	n[338]->next.push_back(n[339]);
	n[338]->next.push_back(n[338]);
	n[338]->next.push_back(n[337]);
	n[338]->next.push_back(n[335]);
	n[339]->next.push_back(n[339]);
	n[339]->next.push_back(n[338]);
	n[339]->next.push_back(n[337]);
	n[339]->next.push_back(n[335]);

	grammar.rules[0].start.push_back(n[339]);
	grammar.rules[0].start.push_back(n[338]);
	grammar.rules[0].start.push_back(n[337]);
	grammar.rules[1].start.push_back(n[0]);
	grammar.rules[2].start.push_back(n[332]);
	grammar.rules[2].start.push_back(n[331]);
	grammar.rules[2].start.push_back(n[330]);
	grammar.rules[2].start.push_back(n[329]);
	grammar.rules[2].start.push_back(n[328]);
	grammar.rules[3].start.push_back(n[334]);
	grammar.rules[3].start.push_back(n[333]);
	grammar.rules[4].start.push_back(n[327]);
	grammar.rules[5].start.push_back(n[76]);
	grammar.rules[6].start.push_back(n[66]);
	grammar.rules[7].start.push_back(n[60]);
	grammar.rules[8].start.push_back(n[54]);
	grammar.rules[9].start.push_back(n[317]);
	grammar.rules[9].start.push_back(n[316]);
	grammar.rules[10].start.push_back(n[1]);
	grammar.rules[11].start.push_back(n[26]);
	grammar.rules[12].start.push_back(n[313]);
	grammar.rules[12].start.push_back(n[312]);
	grammar.rules[12].start.push_back(n[311]);
	grammar.rules[12].start.push_back(n[310]);
	grammar.rules[12].start.push_back(n[309]);
	grammar.rules[12].start.push_back(n[308]);
	grammar.rules[12].start.push_back(n[307]);
	grammar.rules[12].start.push_back(n[306]);
	grammar.rules[12].start.push_back(n[305]);
	grammar.rules[12].start.push_back(n[304]);
	grammar.rules[12].start.push_back(n[303]);
	grammar.rules[12].start.push_back(n[302]);
	grammar.rules[12].start.push_back(n[301]);
	grammar.rules[13].start.push_back(n[315]);
	grammar.rules[13].start.push_back(n[314]);
	grammar.rules[14].start.push_back(n[300]);
	grammar.rules[15].start.push_back(n[289]);
	grammar.rules[16].start.push_back(n[280]);
	grammar.rules[17].start.push_back(n[269]);
	grammar.rules[18].start.push_back(n[256]);
	grammar.rules[19].start.push_back(n[241]);
	grammar.rules[20].start.push_back(n[222]);
	grammar.rules[21].start.push_back(n[206]);
	grammar.rules[22].start.push_back(n[142]);
	grammar.rules[23].start.push_back(n[129]);
	grammar.rules[24].start.push_back(n[118]);
	grammar.rules[25].start.push_back(n[105]);
	grammar.rules[26].start.push_back(n[94]);
	grammar.rules[27].start.push_back(n[28]);
	grammar.rules[27].start.push_back(n[27]);
	grammar.rules[28].start.push_back(n[25]);
	grammar.rules[29].start.push_back(n[38]);
	grammar.rules[30].start.push_back(n[174]);
	grammar.rules[31].start.push_back(n[68]);
	grammar.rules[31].start.push_back(n[67]);
	grammar.rules[32].start.push_back(n[62]);
	grammar.rules[32].start.push_back(n[61]);
	grammar.rules[33].start.push_back(n[56]);
	grammar.rules[33].start.push_back(n[55]);
	grammar.rules[34].start.push_back(n[48]);
	grammar.rules[35].start.push_back(n[50]);
	grammar.rules[35].start.push_back(n[49]);
	grammar.rules[36].start.push_back(n[42]);
	grammar.rules[37].start.push_back(n[22]);
	grammar.rules[37].start.push_back(n[21]);
	grammar.rules[37].start.push_back(n[20]);
	grammar.rules[37].start.push_back(n[19]);
	grammar.rules[38].start.push_back(n[3]);
	grammar.rules[39].start.push_back(n[23]);
	grammar.rules[40].start.push_back(n[7]);
	grammar.rules[40].start.push_back(n[6]);
	grammar.rules[41].start.push_back(n[18]);
	grammar.rules[41].start.push_back(n[16]);
	grammar.rules[41].start.push_back(n[14]);
	grammar.rules[42].start.push_back(n[2]);
	grammar.rules[43].start.push_back(n[13]);
	grammar.rules[44].start.push_back(n[4]);
	grammar.rules[45].start.push_back(n[5]);
}

}

