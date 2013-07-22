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
	parallel(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~parallel();

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	void merge();
	vector<int> generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch);

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t = "", ostream *fout = &cout);

	void push(instruction *i);
};

#endif
