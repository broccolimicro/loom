/*
 * loop.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */


#ifndef loop_h
#define loop_h

#include "conditional.h"

struct loop : conditional
{
	loop();
	loop(instruction *parent, string raw, vspace *vars, string tab, int verbosity);
	~loop();

	loop &operator=(loop l);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	state variant();
	state active_variant();
	state passive_variant();

	void expand_shortcuts();
	void parse();
	void merge();
	int generate_states(graph *trans, int init, state filter);
	state simulate_states(state init, state filter);
	void generate_scribes();

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t);
};

#endif
