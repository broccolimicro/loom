/*
 * loop.h
 *
 * Loop is a fundamental CHP structure of the form *[G1->S1[]G2->S2] such that G1 and G2 are
 * guards and S1 and S2 are statements to be executed. If G1 evaluates true, S1 executes, and
 * likewise for G2 and S2. As long as one of the guards evaluates to true, the process will
 * continue executing iterations of the loop. If no guard evaluates true, the loop terminates.
 * TODO: Functional information
 */


#ifndef loop_h
#define loop_h

#include "control.h"

struct loop : control
{
	loop();
	loop(instruction *parent, sstring raw, variable_space *vars, flag_space *flags);
	~loop();

	instruction *duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert);

	void expand_shortcuts();
	void parse();
	void simulate();
	void rewrite();
	void reorder();
	svector<int> generate_states(petri *n, rule_space *p, svector<int> f, smap<int, int> pbranch, smap<int, int> cbranch);

	void print_hse(sstring t = "", ostream *fout = &cout);
};

#endif
