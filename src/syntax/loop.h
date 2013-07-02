/*
 * loop.h
 *
 * Loop is a fundamental CHP structure of the form *[G1->S1[]G2->S2] such that G1 and G2 are
 * guards and S1 and S2 are statements to be executed. If G1 evaluates true, S1 executes, and
 * likewise for G2 and S2. As long as one of the guards evaluates to true, the process will
 * continue executing iterations of the loop. If no guard evaluates true, the loop terminates.
 * TODO: Functional information
 */


#ifndef loop_h
#define loop_h

#include "condition.h"

struct loop : condition
{
	loop();
	loop(instruction *parent, string raw, vspace *vars, string tab, int verbosity);
	~loop();

	loop &operator=(loop l);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	void merge();
	vector<int> generate_states(petri *n, vector<int> f, map<int, int> branch, vector<int> filter);

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t = "", ostream *fout = &cout);
};

#endif
