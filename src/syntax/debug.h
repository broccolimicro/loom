/*
 * debug.h
 *
 *  Created on: Aug 13, 2013
 *      Author: nbingham
 */

#include "instruction.h"

#ifndef debug_h
#define debug_h

/**
 * assert	- Throws an error if the statement isn't true at a particular point in time
 * require	- Throws an error if the statement isn't true at any point in time
 * assume	- Tries to make the statement true at a particular time. Throws an error if the statement
 * 			  conflicts with known information about the state space.
 * enforce	- Tries to make the statement true at all points in time. Throws an error if the statement
 * 			  conflicts with known information about the state space.
 *
 * TODO
 * allow	- Modifies the timing assumptions used for a particular process.
 * 			  __di__		Force a strict delay insensitive circuit
 * 			  __qdi__		Allow isochronic forks.
 * 			  __bounded__	Allow delay lines with a bounded delay.
 * 			  __mesosync__	Allow mesosynchronoous architecture.
 * 			  __sync__		Allow synchronous architecture.
 */
struct debug : instruction
{
	debug();
	debug(instruction *parent, sstring chp, variable_space *vars, flag_space *flags);
	~debug();

	sstring type;

	instruction *duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert);

	void expand_shortcuts();
	void parse();
	void simulate();
	void rewrite();
	void reorder();
	svector<int> generate_states(petri *n, rule_space *p, svector<int> f, smap<int, int> pbranch, smap<int, int> cbranch);
	void generate_class_requirements();

	void print_hse(sstring t, ostream *fout = &cout);
};

#endif
