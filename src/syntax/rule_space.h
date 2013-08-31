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

	vector<rule> rules;
	vector<pair<vector<int>, int> > excl;
	variable_space *vars;

	void insert(rule r);
	int size();

	rule &operator[](int i);

	void apply_one_of(logic *s, vector<int> a, int v);
	logic apply(logic s);
};

#endif
