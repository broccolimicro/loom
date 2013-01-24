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
	conditional(string uid, string chp, map<string, keyword*> *types, map<string, variable*> globals, string tab, int verbosity);
	~conditional();

	conditional_type type;
	map<string, block*> instrs;		//Guards index instructions

	void expand_shortcuts();
	void parse(map<string, keyword*> *types);
	void generate_states(map<string, state> init);
	void generate_prs(map<string, variable*> globals);
	void generate_statevars();
	// void handshaking_reshuffle();
	void bubble_reshuffle();
};

map<string, state> guard(string raw, map<string, variable*> vars, string tab, int verbosity);

#endif
