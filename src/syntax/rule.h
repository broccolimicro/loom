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
	rule(int uid, petri_net *g, flag_space *flags, bool bubble);
	rule(sstring u, sstring d, sstring v, petri_net *net, flag_space *flags);
	~rule();

	petri_net *net;

	int uid;
	canonical guards[2];
	canonical explicit_guards[2];
	svector<petri_index> implicants[2];

	flag_space *flags;

	rule &operator=(rule r);

	canonical &up();
	canonical &down();

	void strengthen(int t);
	void gen_minterms();
	void gen_bubbleless_minterms();

	bool is_combinational();
	void invert();

	void print(ostream &os, sstring prefix = "");
};

ostream &operator<<(ostream &os, rule r);


#endif
