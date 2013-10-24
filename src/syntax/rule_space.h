/*
 * rule_space.h
 *
 *  Created on: Aug 22, 2013
 *      Author: nbingham
 */

#include "rule.h"
#include "../data.h"
#include "../common.h"

#ifndef rule_space_h
#define rule_space_h

struct rule_space
{
	rule_space();
	~rule_space();

	svector<rule> rules;
	svector<pair<svector<int>, int> > excl;
	variable_space *vars;

	void insert(rule r);
	int size();

	rule &operator[](int i);

	void apply_one_of(logic *s, svector<int> a, int v);
	logic apply(logic s, logic covered = logic(), sstring t = "");

	void generate_minterms(petri *net, flag_space *flags);
	void check(petri *net);

	smap<pair<int, int>, pair<bool, bool> > gen_bubble_reshuffle_graph();

	void print(ostream *fout);
	void print_enable_graph(ostream *fout, petri *net, sstring name);
};

#endif
