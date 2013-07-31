/*
 * guard.h
 *
 * A guard is a fundamental CHP syntax. It is an expression that can evaluate to true or false.
 *  TODO: expand
 */

#ifndef guard_h
#define guard_h

#include "instruction.h"

struct guard : instruction
{
	guard();
	guard(instruction *parent, string chp, variable_space *vars, flag_space *flags);
	~guard();

	bdd solution;

	instruction *duplicate(instruction *parent, variable_space *vars, map<string, string> convert);
	vector<int> variant();
	vector<int> active_variant();
	vector<int> passive_variant();

	void expand_shortcuts();
	void parse();
	void merge();
	vector<int> generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch);

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t = "", ostream *fout = &cout);
};

#endif
