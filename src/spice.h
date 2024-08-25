#pragma once

#include <pgen/grammar.h>

namespace pgen
{

struct spice_t
{
	int32_t SPICE;
	int32_t __;
	int32_t SUBCKT;
	int32_t MODEL;
	int32_t OPTIONS;
	int32_t NODESET;
	int32_t IC;
	int32_t END_K;
	int32_t SUBCKT_K;
	int32_t _;
	int32_t NAME;
	int32_t PORT_LIST;
	int32_t DEVICE;
	int32_t ENDS_K;
	int32_t INTEGER;
	int32_t PARAM_LIST;
	int32_t PARAM;
	int32_t VALUE;
	int32_t MODEL_K;
	int32_t OPTIONS_K;
	int32_t NODESET_K;
	int32_t INITIAL;
	int32_t IC_K;
	int32_t MEASURE;
	int32_t CONSTANT;
	int32_t UNIT;
	int32_t BOOL;
	int32_t TEXT;
	int32_t REAL;

	void load(grammar_t &grammar);
};

}

