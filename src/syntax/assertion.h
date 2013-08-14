/*
 * assertion.h
 *
 *  Created on: Aug 13, 2013
 *      Author: nbingham
 */

#include "instruction.h"

#ifndef assertion_h
#define assertion_h

enum assertion_type
{
	assertion,
	requirement
};

struct assertion : instruction
{
	assertion();
	assertion(instruction *parent, string chp, variable_space *vars, flag_space *flags);
	~assertion();

	assertion_type type;

	instruction *duplicate(instruction *parent, variable_space *vars, map<string, string> convert);

	void expand_shortcuts();
	void parse();
	void merge();
	vector<int> generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch);

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t, ostream *fout = &cout);
};

#endif
