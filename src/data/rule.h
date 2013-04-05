/*
 * rule.h
 *
 * Rule is a data structure that contains firing information for a given variable. It contains
 * name of the variable, as well as the up and down firings for that variable. Program utilizes
 * a vector of rules to generate a full set of PRs.
 */

#include "../common.h"
#include "graph.h"
#include "vspace.h"
#include "trace.h"
#include "state.h"
#include "expression.h"

#ifndef rule_h
#define rule_h

struct rule
{
	rule();
	rule(int uid);
	rule(int uid, graph *g, vspace *v, int verbosity);
	~rule();

	int uid;
	string name;
	expression up, down;

	int verbosity;

	//I had a vector of bools, an int, and a string here. No clue what I actually want.
	int var_usage_up;
	int var_usage_down;

	rule &operator=(rule r);

	void gen_minterms(graph *g);
	void gen_primes();
	void gen_essentials();
	void gen_output(vspace *v);
	void find_var_usage_up();
	void find_var_usage_down();
};

ostream &operator<<(ostream &os, rule r);


#endif
