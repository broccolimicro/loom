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

	logic solution;
	logic expected;

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
