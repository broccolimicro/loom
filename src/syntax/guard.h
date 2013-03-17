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

	int uid;					// indexes into the state in the state space
	state solution;

	guard &operator=(guard g);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	state variant();

	void expand_shortcuts();
	void parse();
	int generate_states(graph *g, int init, state filter);
	void generate_scribes();

	void print_hse();
};

#endif
