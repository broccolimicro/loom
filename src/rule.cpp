/*
 * rule.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "rule.h"
#include "space.h"
#include "state.h"

rule::rule()
{
	left.states.clear();
	left.var = "";
	right.states.clear();
	right.var = "";
}

rule::~rule()
{
	left.states.clear();
	left.var = "";
	right.states.clear();
	right.var = "";
}

rule &rule::operator=(rule s)
{
	left = s.left;
	right = s.right;
	return *this;
}

void rule::clear(int n)
{
	left.states.clear();
	for (int i = 0; i < n; i++)
		left.states.push_back(state("1", false));
	left.var = "";
	right.states.clear();
	right.var = "";
}

/* This function returns the nth necessary firing of a production rule.
 *
 */
int rule::index(int n)
{
	list<state>::iterator i;
	int j;
	for (i = right.states.begin(), j = 0; i != right.states.end() && n > 0; i++, j++)
		if (i->data == "1" && i->prs)
			n--;

	return j;
}

ostream &operator<<(ostream &os, rule r)
{
	list<state>::iterator i;

    os << r.left << " -> " << r.right;

    return os;
}

rule simplify(rule r, map<string, space> states)
{
	return r;
}
