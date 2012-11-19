/*
 * conditional.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "block.h"
#include "common.h"
#include "keyword.h"

#ifndef conditional_h
#define conditional_h

enum conditional_type
{
	unknown = 0,
	mutex = 1,
	choice = 2
};

struct conditional : block
{
	conditional();
	conditional(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab);
	~conditional();

	conditional_type type;
	map<string, instruction*> instrs;		//Guards index instructions


	void parse(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab);
};

map<string, state> guard(string raw, map<string, variable*> vars, string tab);

#endif
