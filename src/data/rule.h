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
#include "expression.h"

#ifndef rule_h
#define rule_h

struct rule
{
	rule();
	rule(int uid);
	rule(int uid, graph *g, vspace *v);
	~rule();

	int uid;
	string name;
	expression up, down;

	rule &operator=(rule r);

	void gen_minterms(graph *g);
	void gen_primes();
	void gen_essentials();
	void gen_output(vspace *v);
};

ostream &operator<<(ostream &os, rule r);


#endif
