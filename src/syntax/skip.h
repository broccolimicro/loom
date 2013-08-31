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
	skip(instruction *parent, string chp, variable_space *vars, flag_space *flags);
	~skip();

	canonical solution;

	instruction *duplicate(instruction *parent, variable_space *vars, map<string, string> convert);

	void expand_shortcuts();
	void parse();
	void simulate();
	void rewrite();
	void reorder();
	vector<int> generate_states(petri *n, rule_space *p, vector<int> f, map<int, int> pbranch, map<int, int> cbranch);

	void print_hse(string t = "", ostream *fout = &cout);
};


#endif
