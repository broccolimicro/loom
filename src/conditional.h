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
#include "state.h"
#include "guard.h"
#include "parallel.h"

#ifndef conditional_h
#define conditional_h

enum conditional_type
{
	unknown = 0,
	mutex = 1,
	choice = 2
};

struct conditional : parallel
{
	conditional();
	conditional(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity);
	~conditional();

	conditional_type type;
	list<pair<block*, guard*> > instrs;		//Guards index instructions

	conditional &operator=(conditional c);

	instruction *duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse(map<string, keyword*> types);
	int generate_states(state_space *space, graph *trans, int init);
	void generate_prs();
	void generate_statevars();
	// void handshaking_reshuffle();
	void bubble_reshuffle();

	void print_hse();
};

#endif
