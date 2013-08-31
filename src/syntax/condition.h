/*
 * condition.h
 *
 * A condition is a fundamental CHP syntax that takes the form [G1->S1[]G2->S2]
 * Where G1 and G2 are guards and S1 and S2 are statements to execute. If G1 evaluates.
 * S1 is to execute. If G2 evaluates, S2 is to execute. If neither guard evaluates,
 * the statement is to sequential until one of the guards does evaluate. The environment guarantees
 * mutual exclusion for [] (deterministic selection). Nondeterministic selection is yet to be
 * implemented. TODO: Description on what condition programatically does.
 */

#ifndef condition_h
#define condition_h

#include "guard.h"
#include "sequential.h"
#include "control.h"

struct condition : control
{
	condition();
	condition(instruction *parent, string chp, variable_space *vars, flag_space *flags);
	~condition();

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
