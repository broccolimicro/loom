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
	conditional(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~conditional();

	conditional_type type;
	list<pair<block*, guard*> > instrs;		//Guards index instructions

	conditional &operator=(conditional c);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	void simplify();
	int generate_states(graph *trans, int init);
	void generate_scribes();

	void print_hse();
};

#endif
