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
	guard(string chp, vspace *vars, string tab, int verbosity);
	~guard();

	int uid;					// indexes into the state in the state space

	guard &operator=(guard g);

	instruction *duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	int generate_states(graph *trans, int init);
	void generate_prs();

	void print_hse();
};

state solve(string raw, vspace *vars, string tab, int verbosity);

#endif
