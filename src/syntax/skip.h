/*
 * skip.h
 *
 *  Created on: Aug 21, 2013
 *      Author: nbingham
 */

#ifndef skip_h
#define skip_h

#include "instruction.h"

struct skip : instruction
{
	skip();
	skip(instruction *parent, sstring chp, variable_space *vars, flag_space *flags);
	~skip();

	canonical solution;

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
