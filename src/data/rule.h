/*
 * rule.h
 *
 * Rule is a data structure that contains firing information for a given variable. It contains
 * name of the variable, as well as the up and down firings for that variable. Program utilizes
 * a vector of rules to generate a full set of PRs.
 */

#include "../common.h"
#include "vspace.h"
#include "expression.h"
#include "petri.h"

#ifndef rule_h
#define rule_h

struct rule
{
	rule();
	rule(int uid);
	rule(int uid, petri *g, vspace *v, int verbosity);
	~rule();

	petri *net;
	vspace *vars;

	int uid;
	int up, down;

	int verbosity;

	rule &operator=(rule r);

	void gen_minterms();
};

ostream &operator<<(ostream &os, rule r);


#endif
