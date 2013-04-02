/*
 * guard.h
 *
 *  Created on: Jan 22, 2013
 *      Author: nbingham
 */

#ifndef guard_h
#define guard_h

#include "instruction.h"

struct guard : instruction
{
	guard();
	guard(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~guard();

	state solution;

	guard &operator=(guard g);

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

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t);
};

#endif
