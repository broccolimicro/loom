/*
 * rule.h
 *
 *  Created on: Nov 11, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "trace.h"
#include "state.h"

#ifndef rule_h
#define rule_h

struct rule
{
	rule();
	~rule();

	string left, right;
	vector<state> implicants;

	//rule's variable uid
	int uid;
	//Is it an up production rule?
	bool up;
	trace output;

	rule &operator=(rule s);

	void clear(int n);
	//int index(int n);
};

rule reduce_to_prime(rule pr);
rule remove_too_strong(rule pr);
rule minimize_rule(rule pr);
vector<rule> minimize_rule_vector(vector<rule> prs);

void print_implicant_tags(vector<state> implicants);
ostream &operator<<(ostream &os, rule r);


#endif
