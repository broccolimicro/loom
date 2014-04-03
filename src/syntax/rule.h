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

void merge_guards(canonical &guard0, canonical implicant0, canonical &guard1, canonical implicant1);

struct reductionpath
{
	reductionpath();
	reductionpath(petri_index d, bool c, bool p);
	~reductionpath();

	svector<petri_index> begin;
	svector<petri_index> data;
	bool conflict;
	bool placeholder;

	void merge(reductionpath &r);
	void push_back(petri_index i);
};

struct reductionhdl
{
	reductionhdl();
	reductionhdl(const reductionhdl &r);
	reductionhdl(petri_net *net, petri_index start);
	~reductionhdl();

	canonical implicant;
	svector<reductionpath> path;
	canonical guard;
	svector<bool> covered;

	reductionhdl &operator=(reductionhdl r);
};

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

	bool separate(reductionhdl &reduction, int t);
	void strengthen(int t);
	void gen_minterms();
	void gen_bubbleless_minterms();

	bool is_combinational();
	void invert();

	void print(ostream &os, sstring prefix = "");
};

ostream &operator<<(ostream &os, rule r);


#endif
