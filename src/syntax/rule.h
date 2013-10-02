/*
 * rule.h
 *
 * Rule is a data structure that contains firing information for a given variable. It contains
 * name of the variable, as well as the up and down firings for that variable. Program utilizes
 * a vec of rules to generate a full set of PRs.
 */

#include "../common.h"
#include "../data.h"

#ifndef rule_h
#define rule_h

struct rule
{
	rule();
	rule(int uid);
	rule(int uid, petri *g, variable_space *v, flag_space *flags, bool bubble);
	rule(sstring u, sstring d, sstring v, variable_space *vars, petri *net, flag_space *flags);
	~rule();

	petri *net;
	variable_space *vars;

	int uid;
	logic guards[2];
	logic explicit_guards[2];
	svector<int> implicants[2];

	flag_space *flags;

	rule &operator=(rule r);

	logic &up();
	logic &down();

	pair<svector<int>, logic> closest_transition(int p, int tid, logic conflicting_state, logic rule_guard, logic implicant_state, svector<int> tail, smap<int, logic> mutables, svector<bool> *covered, int i = 0);
	pair<svector<int>, logic> strengthen(int p, int tid, svector<bool> *covered, logic rule_guard, logic implicant_state, int t, svector<int> tail, smap<int, logic> mutables, int i = 0);
	void gen_minterms();
	void gen_bubbleless_minterms();

	void print(ostream &os, sstring prefix = "");
};

ostream &operator<<(ostream &os, rule r);


#endif
