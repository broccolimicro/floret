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
	END_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(END_K, "spice::end_k", false, true));
	SUBCKT_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(SUBCKT_K, "spice::subckt_k", false, true));
	_ = grammar.rules.size();
	grammar.rules.push_back(rule_t(_, "spice::_", false, true));
	NAME = grammar.rules.size();
	grammar.rules.push_back(rule_t(NAME, "spice::name", true, true));
	PORT_LIST = grammar.rules.size();
	grammar.rules.push_back(rule_t(PORT_LIST, "spice::port_list", true, true));
	DEVICE = grammar.rules.size();
	grammar.rules.push_back(rule_t(DEVICE, "spice::device", true, true));
	ENDS_K = grammar.rules.size();
	grammar.rules.push_back(rule_t(ENDS_K, "spice::ends_k", false, true));
	INTEGER = grammar.rules.size();
	grammar.rules.push_back(rule_t(INTEGER, "spice::integer", true, true));
	PARAM_LIST = grammar.rules.size();
	grammar.rules.push_back(rule_t(PARAM_LIST, "spice::param_list", true, true));
	PARAM = grammar.rules.size();
	grammar.rules.push_back(rule_t(PARAM, "spice::param", true, true));
	VALUE = grammar.rules.size();
	grammar.rules.push_back(rule_t(VALUE, "spice::value", true, true));
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
	UNIT = grammar.rules.size();
	grammar.rules.push_back(rule_t(UNIT, "spice::unit", true, true));
	BOOL = grammar.rules.size();
	grammar.rules.push_back(rule_t(BOOL, "spice::bool", true, true));
	TEXT = grammar.rules.size();
	grammar.rules.push_back(rule_t(TEXT, "spice::text", true, true));
	REAL = grammar.rules.size();
	grammar.rules.push_back(rule_t(REAL, "spice::real", true, true));

	symbol_t *n[112];
	n[0] = grammar.insert(new regular_expression("[\\n\\r]+", true));
	n[1] = grammar.insert(new regular_expression("", true));
	n[2] = grammar.insert(new regular_expression("\\*[^\\n\\r]*", true));
	n[3] = grammar.insert(new regular_expression("[ \\t]*", true));
	n[4] = grammar.insert(new regular_expression("[\\n\\r]\\+[ \\t]*", true));
	n[5] = grammar.insert(new regular_expression("", true));
	n[6] = grammar.insert(new regular_expression("\\*[^\\n\\r]*", true));
	n[7] = grammar.insert(new regular_expression("[ \\t]*", true));
	n[8] = grammar.insert(new regular_expression("\\\"([^\\\"\\\\]|\\\\.)*\\\"", false));
	n[9] = grammar.insert(new regular_expression("[0-9]+", false));
	n[10] = grammar.insert(new regular_expression("false", false));
	n[11] = grammar.insert(new regular_expression("true", false));
	n[12] = grammar.insert(new stem(14, false));
	n[13] = grammar.insert(new regular_expression("[-+]", false));
	n[14] = grammar.insert(new regular_expression("e", false));
	n[15] = grammar.insert(new stem(14, false));
	n[16] = grammar.insert(new regular_expression("\\.", false));
	n[17] = grammar.insert(new stem(14, false));
	n[18] = grammar.insert(new stem(28, true));
	n[19] = grammar.insert(new stem(27, true));
	n[20] = grammar.insert(new stem(26, true));
	n[21] = grammar.insert(new regular_expression("[aAfFpPnNuUmMkKxXgG]", false));
	n[22] = grammar.insert(new stem(25, true));
	n[23] = grammar.insert(new stem(24, true));
	n[24] = grammar.insert(new regular_expression("[#_a-zA-Z][#_a-zA-Z0-9]*", false));
	n[25] = grammar.insert(new stem(9, true));
	n[26] = grammar.insert(new stem(17, true));
	n[27] = grammar.insert(new stem(9, true));
	n[28] = grammar.insert(new regular_expression(",", false));
	n[29] = grammar.insert(new stem(9, true));
	n[30] = grammar.insert(new stem(17, true));
	n[31] = grammar.insert(new stem(9, true));
	n[32] = grammar.insert(new regular_expression("=", false));
	n[33] = grammar.insert(new stem(9, true));
	n[34] = grammar.insert(new stem(10, true));
	n[35] = grammar.insert(new regular_expression("\\)", false));
	n[36] = grammar.insert(new stem(10, true));
	n[37] = grammar.insert(new stem(14, true));
	n[38] = grammar.insert(new regular_expression("\\(", false));
	n[39] = grammar.insert(new regular_expression("[vViI]", true));
	n[40] = grammar.insert(new stem(9, true));
	n[41] = grammar.insert(new stem(17, true));
	n[42] = grammar.insert(new stem(9, true));
	n[43] = grammar.insert(new regular_expression("=", false));
	n[44] = grammar.insert(new stem(9, true));
	n[45] = grammar.insert(new stem(23, true));
	n[46] = grammar.insert(new regular_expression("\\.IC", true));
	n[47] = grammar.insert(new regular_expression("\\.ic", true));
	n[48] = grammar.insert(new stem(1, true));
	n[49] = grammar.insert(new stem(21, true));
	n[50] = grammar.insert(new stem(9, true));
	n[51] = grammar.insert(new stem(22, true));
	n[52] = grammar.insert(new regular_expression("\\.NODESET", true));
	n[53] = grammar.insert(new regular_expression("\\.nodeset", true));
	n[54] = grammar.insert(new stem(1, true));
	n[55] = grammar.insert(new stem(21, true));
	n[56] = grammar.insert(new stem(9, true));
	n[57] = grammar.insert(new stem(20, true));
	n[58] = grammar.insert(new regular_expression("\\.OPTIONS", true));
	n[59] = grammar.insert(new regular_expression("\\.options", true));
	n[60] = grammar.insert(new stem(1, true));
	n[61] = grammar.insert(new stem(16, true));
	n[62] = grammar.insert(new stem(9, true));
	n[63] = grammar.insert(new stem(19, true));
	n[64] = grammar.insert(new regular_expression("\\.MODEL", true));
	n[65] = grammar.insert(new regular_expression("\\.model", true));
	n[66] = grammar.insert(new stem(1, true));
	n[67] = grammar.insert(new stem(16, true));
	n[68] = grammar.insert(new stem(9, true));
	n[69] = grammar.insert(new stem(10, true));
	n[70] = grammar.insert(new stem(9, true));
	n[71] = grammar.insert(new stem(10, true));
	n[72] = grammar.insert(new stem(9, true));
	n[73] = grammar.insert(new stem(18, true));
	n[74] = grammar.insert(new stem(9, true));
	n[75] = grammar.insert(new stem(10, true));
	n[76] = grammar.insert(new stem(9, true));
	n[77] = grammar.insert(new stem(17, true));
	n[78] = grammar.insert(new stem(16, true));
	n[79] = grammar.insert(new stem(1, true));
	n[80] = grammar.insert(new stem(15, true));
	n[81] = grammar.insert(new stem(9, true));
	n[82] = grammar.insert(new stem(10, true));
	n[83] = grammar.insert(new stem(14, true));
	n[84] = grammar.insert(new regular_expression("[rRcClLdDqQjJmMxXfFhHeEgGkKvViI]", true));
	n[85] = grammar.insert(new stem(9, true));
	n[86] = grammar.insert(new stem(10, true));
	n[87] = grammar.insert(new regular_expression("\\.ENDS", true));
	n[88] = grammar.insert(new regular_expression("\\.ends", true));
	n[89] = grammar.insert(new regular_expression("\\.SUBCKT", true));
	n[90] = grammar.insert(new regular_expression("\\.subckt", true));
	n[91] = grammar.insert(new stem(1, true));
	n[92] = grammar.insert(new stem(13, true));
	n[93] = grammar.insert(new stem(1, true));
	n[94] = grammar.insert(new stem(12, true));
	n[95] = grammar.insert(new stem(1, true));
	n[96] = grammar.insert(new stem(11, true));
	n[97] = grammar.insert(new stem(9, true));
	n[98] = grammar.insert(new stem(10, true));
	n[99] = grammar.insert(new stem(9, true));
	n[100] = grammar.insert(new stem(8, true));
	n[101] = grammar.insert(new regular_expression("\\.END", true));
	n[102] = grammar.insert(new regular_expression("\\.end", true));
	n[103] = grammar.insert(new regular_expression("[\\0]", true));
	n[104] = grammar.insert(new stem(1, true));
	n[105] = grammar.insert(new stem(7, true));
	n[106] = grammar.insert(new stem(6, true));
	n[107] = grammar.insert(new stem(5, true));
	n[108] = grammar.insert(new stem(4, true));
	n[109] = grammar.insert(new stem(3, true));
	n[110] = grammar.insert(new stem(2, true));
	n[111] = grammar.insert(new stem(1, true));

	n[0]->next.push_back(NULL);
	n[1]->next.push_back(n[0]);
	n[2]->next.push_back(n[0]);
	n[3]->next.push_back(n[2]);
	n[3]->next.push_back(n[1]);
	n[4]->next.push_back(n[6]);
	n[4]->next.push_back(n[5]);
	n[4]->next.push_back(NULL);
	n[5]->next.push_back(n[4]);
	n[6]->next.push_back(n[4]);
	n[7]->next.push_back(n[6]);
	n[7]->next.push_back(n[5]);
	n[7]->next.push_back(NULL);
	n[8]->next.push_back(NULL);
	n[9]->next.push_back(NULL);
	n[10]->next.push_back(NULL);
	n[11]->next.push_back(NULL);
	n[12]->next.push_back(NULL);
	n[13]->next.push_back(n[12]);
	n[14]->next.push_back(n[13]);
	n[14]->next.push_back(n[12]);
	n[15]->next.push_back(n[14]);
	n[15]->next.push_back(NULL);
	n[16]->next.push_back(n[15]);
	n[17]->next.push_back(n[16]);
	n[17]->next.push_back(n[14]);
	n[17]->next.push_back(NULL);
	n[18]->next.push_back(NULL);
	n[19]->next.push_back(NULL);
	n[20]->next.push_back(NULL);
	n[21]->next.push_back(NULL);
	n[22]->next.push_back(NULL);
	n[23]->next.push_back(n[22]);
	n[23]->next.push_back(NULL);
	n[24]->next.push_back(NULL);
	n[25]->next.push_back(n[28]);
	n[25]->next.push_back(NULL);
	n[26]->next.push_back(n[25]);
	n[27]->next.push_back(n[26]);
	n[28]->next.push_back(n[27]);
	n[29]->next.push_back(n[28]);
	n[29]->next.push_back(NULL);
	n[30]->next.push_back(n[29]);
	n[31]->next.push_back(n[30]);
	n[32]->next.push_back(n[31]);
	n[33]->next.push_back(n[32]);
	n[34]->next.push_back(n[33]);
	n[35]->next.push_back(NULL);
	n[36]->next.push_back(n[35]);
	n[37]->next.push_back(n[35]);
	n[38]->next.push_back(n[37]);
	n[38]->next.push_back(n[36]);
	n[39]->next.push_back(n[38]);
	n[40]->next.push_back(NULL);
	n[41]->next.push_back(n[40]);
	n[42]->next.push_back(n[41]);
	n[43]->next.push_back(n[42]);
	n[44]->next.push_back(n[43]);
	n[45]->next.push_back(n[44]);
	n[46]->next.push_back(NULL);
	n[47]->next.push_back(NULL);
	n[48]->next.push_back(NULL);
	n[49]->next.push_back(n[49]);
	n[49]->next.push_back(n[48]);
	n[50]->next.push_back(n[49]);
	n[50]->next.push_back(n[48]);
	n[51]->next.push_back(n[50]);
	n[52]->next.push_back(NULL);
	n[53]->next.push_back(NULL);
	n[54]->next.push_back(NULL);
	n[55]->next.push_back(n[55]);
	n[55]->next.push_back(n[54]);
	n[56]->next.push_back(n[55]);
	n[56]->next.push_back(n[54]);
	n[57]->next.push_back(n[56]);
	n[58]->next.push_back(NULL);
	n[59]->next.push_back(NULL);
	n[60]->next.push_back(NULL);
	n[61]->next.push_back(n[61]);
	n[61]->next.push_back(n[60]);
	n[62]->next.push_back(n[61]);
	n[62]->next.push_back(n[60]);
	n[63]->next.push_back(n[62]);
	n[64]->next.push_back(NULL);
	n[65]->next.push_back(NULL);
	n[66]->next.push_back(NULL);
	n[67]->next.push_back(n[67]);
	n[67]->next.push_back(n[66]);
	n[68]->next.push_back(n[67]);
	n[68]->next.push_back(n[66]);
	n[69]->next.push_back(n[68]);
	n[70]->next.push_back(n[69]);
	n[71]->next.push_back(n[70]);
	n[72]->next.push_back(n[71]);
	n[73]->next.push_back(n[72]);
	n[74]->next.push_back(n[78]);
	n[74]->next.push_back(n[77]);
	n[74]->next.push_back(n[75]);
	n[74]->next.push_back(NULL);
	n[75]->next.push_back(n[74]);
	n[76]->next.push_back(n[78]);
	n[76]->next.push_back(n[77]);
	n[76]->next.push_back(n[75]);
	n[76]->next.push_back(NULL);
	n[77]->next.push_back(n[76]);
	n[78]->next.push_back(n[78]);
	n[78]->next.push_back(n[77]);
	n[78]->next.push_back(n[75]);
	n[78]->next.push_back(NULL);
	n[79]->next.push_back(NULL);
	n[80]->next.push_back(n[79]);
	n[81]->next.push_back(n[80]);
	n[82]->next.push_back(n[81]);
	n[83]->next.push_back(n[81]);
	n[84]->next.push_back(n[83]);
	n[84]->next.push_back(n[82]);
	n[85]->next.push_back(n[86]);
	n[85]->next.push_back(NULL);
	n[86]->next.push_back(n[85]);
	n[87]->next.push_back(NULL);
	n[88]->next.push_back(NULL);
	n[89]->next.push_back(NULL);
	n[90]->next.push_back(NULL);
	n[91]->next.push_back(NULL);
	n[92]->next.push_back(n[91]);
	n[93]->next.push_back(n[94]);
	n[93]->next.push_back(n[93]);
	n[93]->next.push_back(n[92]);
	n[94]->next.push_back(n[94]);
	n[94]->next.push_back(n[93]);
	n[94]->next.push_back(n[92]);
	n[95]->next.push_back(n[94]);
	n[95]->next.push_back(n[93]);
	n[95]->next.push_back(n[92]);
	n[96]->next.push_back(n[95]);
	n[97]->next.push_back(n[96]);
	n[98]->next.push_back(n[97]);
	n[99]->next.push_back(n[98]);
	n[100]->next.push_back(n[99]);
	n[101]->next.push_back(NULL);
	n[102]->next.push_back(NULL);
	n[103]->next.push_back(NULL);
	n[104]->next.push_back(n[104]);
	n[104]->next.push_back(n[103]);
	n[105]->next.push_back(n[104]);
	n[105]->next.push_back(n[103]);
	n[106]->next.push_back(n[111]);
	n[106]->next.push_back(n[110]);
	n[106]->next.push_back(n[109]);
	n[106]->next.push_back(n[108]);
	n[106]->next.push_back(n[107]);
	n[106]->next.push_back(n[106]);
	n[106]->next.push_back(n[105]);
	n[106]->next.push_back(n[103]);
	n[107]->next.push_back(n[111]);
	n[107]->next.push_back(n[110]);
	n[107]->next.push_back(n[109]);
	n[107]->next.push_back(n[108]);
	n[107]->next.push_back(n[107]);
	n[107]->next.push_back(n[106]);
	n[107]->next.push_back(n[105]);
	n[107]->next.push_back(n[103]);
	n[108]->next.push_back(n[111]);
	n[108]->next.push_back(n[110]);
	n[108]->next.push_back(n[109]);
	n[108]->next.push_back(n[108]);
	n[108]->next.push_back(n[107]);
	n[108]->next.push_back(n[106]);
	n[108]->next.push_back(n[105]);
	n[108]->next.push_back(n[103]);
	n[109]->next.push_back(n[111]);
	n[109]->next.push_back(n[110]);
	n[109]->next.push_back(n[109]);
	n[109]->next.push_back(n[108]);
	n[109]->next.push_back(n[107]);
	n[109]->next.push_back(n[106]);
	n[109]->next.push_back(n[105]);
	n[109]->next.push_back(n[103]);
	n[110]->next.push_back(n[111]);
	n[110]->next.push_back(n[110]);
	n[110]->next.push_back(n[109]);
	n[110]->next.push_back(n[108]);
	n[110]->next.push_back(n[107]);
	n[110]->next.push_back(n[106]);
	n[110]->next.push_back(n[105]);
	n[110]->next.push_back(n[103]);
	n[111]->next.push_back(n[111]);
	n[111]->next.push_back(n[110]);
	n[111]->next.push_back(n[109]);
	n[111]->next.push_back(n[108]);
	n[111]->next.push_back(n[107]);
	n[111]->next.push_back(n[106]);
	n[111]->next.push_back(n[105]);
	n[111]->next.push_back(n[103]);

	grammar.rules[0].start.push_back(n[111]);
	grammar.rules[0].start.push_back(n[110]);
	grammar.rules[0].start.push_back(n[109]);
	grammar.rules[0].start.push_back(n[108]);
	grammar.rules[0].start.push_back(n[107]);
	grammar.rules[0].start.push_back(n[106]);
	grammar.rules[0].start.push_back(n[105]);
	grammar.rules[1].start.push_back(n[3]);
	grammar.rules[2].start.push_back(n[100]);
	grammar.rules[3].start.push_back(n[73]);
	grammar.rules[4].start.push_back(n[63]);
	grammar.rules[5].start.push_back(n[57]);
	grammar.rules[6].start.push_back(n[51]);
	grammar.rules[7].start.push_back(n[102]);
	grammar.rules[7].start.push_back(n[101]);
	grammar.rules[8].start.push_back(n[90]);
	grammar.rules[8].start.push_back(n[89]);
	grammar.rules[9].start.push_back(n[7]);
	grammar.rules[10].start.push_back(n[24]);
	grammar.rules[11].start.push_back(n[86]);
	grammar.rules[11].start.push_back(NULL);
	grammar.rules[12].start.push_back(n[84]);
	grammar.rules[13].start.push_back(n[88]);
	grammar.rules[13].start.push_back(n[87]);
	grammar.rules[14].start.push_back(n[9]);
	grammar.rules[15].start.push_back(n[78]);
	grammar.rules[15].start.push_back(n[77]);
	grammar.rules[15].start.push_back(n[75]);
	grammar.rules[15].start.push_back(NULL);
	grammar.rules[16].start.push_back(n[34]);
	grammar.rules[17].start.push_back(n[23]);
	grammar.rules[18].start.push_back(n[65]);
	grammar.rules[18].start.push_back(n[64]);
	grammar.rules[19].start.push_back(n[59]);
	grammar.rules[19].start.push_back(n[58]);
	grammar.rules[20].start.push_back(n[53]);
	grammar.rules[20].start.push_back(n[52]);
	grammar.rules[21].start.push_back(n[45]);
	grammar.rules[22].start.push_back(n[47]);
	grammar.rules[22].start.push_back(n[46]);
	grammar.rules[23].start.push_back(n[39]);
	grammar.rules[24].start.push_back(n[20]);
	grammar.rules[24].start.push_back(n[19]);
	grammar.rules[24].start.push_back(n[18]);
	grammar.rules[25].start.push_back(n[21]);
	grammar.rules[26].start.push_back(n[11]);
	grammar.rules[26].start.push_back(n[10]);
	grammar.rules[27].start.push_back(n[8]);
	grammar.rules[28].start.push_back(n[17]);
}

}

