/*
 * loop.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "conditional.h"
#include "common.h"

#ifndef loop_h
#define loop_h

struct loop : conditional
{
	loop();
	loop(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
	~loop();

	void parse(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
	void pass(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
};

#endif
