/*
 * rule.h
 *
 *  Created on: Nov 11, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "graph.h"
#include "vspace.h"
#include "trace.h"
#include "state.h"

#ifndef rule_h
#define rule_h

struct rule
{
	rule();
	rule(int uid);
	rule(int uid, graph *g, vspace *v);
	~rule();

	int uid;
	string up, down;
	vector<state> up_implicants;
	vector<state> up_primes;
	vector<vector<state> > up_essential;

	vector<state> down_implicants;
	vector<state> down_primes;
	vector<vector<state> > down_essential;

	rule &operator=(rule r);

	void gen_minterms(graph *g);
	void gen_primes();
	void gen_essentials();
	void gen_output(vspace *v);

	void clear();
};

/*rule reduce_to_prime(rule pr);
rule remove_too_strong(rule pr);
rule minimize_rule(rule pr);
vector<rule> minimize_rule_vector(vector<rule> prs);*/

void print_implicant_tags(vector<state> implicants);
ostream &operator<<(ostream &os, rule r);


#endif
