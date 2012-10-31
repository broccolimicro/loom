/*
 * conditional.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham
 */

#include "block.h"
#include "common.h"

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
	conditional(string raw, map<string, variable*> svars, string tab);
	~conditional();

	conditional_type type;
	map<string, instruction>		instrs;

	void parse(string raw, map<string, variable*> svars, string tab);
};

#endif
