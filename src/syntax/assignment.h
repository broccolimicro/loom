/*
 * assignment.h
 *
 * An assignment is a basic CHP syntax of the form variable:=value, often shorthanded
 * with variable+ for assignment to 1 and variable- for assignment to 0.
 * The assignment structure contains a uid indicating the statespace state it affects,
 * as well as a list of simultaneous assignments (expression variable pairs).
 */

#ifndef assignment_h
#define assignment_h

#include "instruction.h"

struct assignment : instruction
{
	assignment();
	assignment(instruction *parent, string chp, variable_space *vars, flag_space *flags);
	~assignment();

	list<pair<string, string> > expr;

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
