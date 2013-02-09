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
	guard(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity);
	~guard();

	int uid;					// indexes into the state in the state space

	guard &operator=(guard g);

	instruction *duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse(map<string, keyword*> types);
	int generate_states(state_space *space, graph *trans, int init);
	void generate_prs();

	void print_hse();
};

state solve(string raw, map<string, variable> *vars, string tab, int verbosity);

#endif
