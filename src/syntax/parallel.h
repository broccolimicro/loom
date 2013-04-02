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

	parallel &operator=(parallel p);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	state variant();
	state active_variant();
	state passive_variant();

	void expand_shortcuts();
	void parse();
	void merge();
	int generate_states(graph *g, int init, state filter);
	state simulate_states(state init, state filter);
	void generate_scribes();


	void recursive_branch_set(graph *g, int from, pair<int, int> id);
	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t);

	void push(instruction *i);
};

#endif
