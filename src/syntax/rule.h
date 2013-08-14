/*
 * rule.h
 *
 * Rule is a data structure that contains firing information for a given variable. It contains
 * name of the variable, as well as the up and down firings for that variable. Program utilizes
 * a vector of rules to generate a full set of PRs.
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
	rule(string u, string d, string v, variable_space *vars, petri *net, flag_space *flags);
	~rule();

	petri *net;
	variable_space *vars;

	int uid;
	logic up, down;

	flag_space *flags;

	rule &operator=(rule r);

	void gen_minterms();
	void gen_bubbleless_minterms();

	void print(ostream &os, string prefix = "");
};

ostream &operator<<(ostream &os, rule r);


#endif
