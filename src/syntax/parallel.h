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

#include "composition.h"

struct parallel : composition
{
	parallel();
	parallel(instruction *parent, string chp, variable_space *vars, flag_space *flags);
	~parallel();

	instruction *duplicate(instruction *parent, variable_space *vars, map<string, string> convert);

	void expand_shortcuts();
	void parse();
	void simulate();
	void rewrite();
	void reorder();
	vector<int> generate_states(petri *n, rule_space *p, vector<int> f, map<int, int> pbranch, map<int, int> cbranch);

	void print_hse(string t = "", ostream *fout = &cout);

	void push(instruction *i);
};

#endif
