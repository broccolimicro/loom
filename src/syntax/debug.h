/*
 * debug.h
 *
 *  Created on: Aug 13, 2013
 *      Author: nbingham
 */

#include "instruction.h"

#ifndef debug_h
#define debug_h

struct debug : instruction
{
	debug();
	debug(instruction *parent, string chp, variable_space *vars, flag_space *flags);
	~debug();

	string type;

	instruction *duplicate(instruction *parent, variable_space *vars, map<string, string> convert);

	void expand_shortcuts();
	void parse();
	void merge();
	vector<int> generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch);
	void generate_class_requirements();

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t, ostream *fout = &cout);
};

#endif
