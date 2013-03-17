/*
 * parallel.h
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */


#ifndef parallel_h
#define parallel_h

#include "block.h"

struct parallel : block
{
	parallel();
	parallel(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~parallel();

	int uid;					// indexes into the state in the state space

	parallel &operator=(parallel p);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	state variant();
	state active_variant();
	state passive_variant();

	void expand_shortcuts();
	void parse();
	int generate_states(graph *trans, int init, state filter);
	void generate_scribes();

	void print_hse();

	void push(instruction *i);
};

#endif
