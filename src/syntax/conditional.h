/*
 * conditional.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#ifndef conditional_h
#define conditional_h

#include "guard.h"
#include "block.h"
#include "parallel.h"

enum conditional_type
{
	unknown = 0,
	mutex = 1,
	choice = 2
};

struct conditional : parallel
{
	conditional();
	conditional(string chp, vspace *vars, string tab, int verbosity);
	~conditional();

	conditional_type type;
	list<pair<block*, guard*> > instrs;		//Guards index instructions

	conditional &operator=(conditional c);

	instruction *duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	int generate_states(graph *trans, int init);
	void generate_prs();
	void generate_statevars();
	// void handshaking_reshuffle();
	void bubble_reshuffle();

	void print_hse();
};

#endif
