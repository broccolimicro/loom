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
	guard(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~guard();

	bdd solution;

	guard &operator=(guard g);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	minterm variant();
	minterm active_variant();
	minterm passive_variant();

	void expand_shortcuts();
	void parse();
	void merge();
	pids generate_states(petri *n, pids f, bids b, minterm filter);
	place simulate_states(place init, minterm filter);

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t = "", ostream *fout = &cout);
};

#endif
