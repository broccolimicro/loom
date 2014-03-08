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

struct petri_net;

struct rule_space
{
	rule_space();
	~rule_space();

	svector<rule> rules;
	svector<pair<svector<int>, int> > excl;
	petri_net *net;

	void insert(rule r);
	int size();

	rule &operator[](int i);

	void apply_one_of(canonical *s, svector<int> a, int v);
	canonical apply(canonical s, canonical covered = canonical(), sstring t = "");

	void generate_minterms(flag_space *flags);
	void check();

	smap<pair<int, int>, pair<bool, bool> > gen_bubble_reshuffle_graph();

	void print(ostream *fout);
	void print_enable_graph(ostream *fout, sstring name);
};

#endif
