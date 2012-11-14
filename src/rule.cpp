/*
 * rule.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: Ned Bingham
 */

#include "rule.h"
#include "space.h"
#include "state.h"

rule::rule()
{
	plus.states.clear();
	plus.var = "";
	minus.states.clear();
	minus.var = "";
	var = "";
}

rule::~rule()
{
	plus.states.clear();
	plus.var = "";
	minus.states.clear();
	minus.var = "";
	var = "";
}

rule &rule::operator=(rule s)
{
	var = s.var;
	plus = s.plus;
	minus = s.minus;
	return *this;
}

int rule::check()
{
	return count((plus<1) & (minus<1));
}

void rule::clear(int n)
{
	plus.states.clear();
	for (int i = 0; i < n; i++)
		plus.states.push_back(state("1", false));
	plus.var = "";
	minus.states.clear();
	for (int i = 0; i < n; i++)
		minus.states.push_back(state("1", false));
	minus.var = "";
	var = "";
}

ostream &operator<<(ostream &os, rule r)
{
	list<state>::iterator i;

    os << "{" << r.plus.var << " -> " << r.var << "+ : ";
    for (i = r.plus.states.begin(); i != r.plus.states.end(); i++)
    	os << *i << " ";
    cout << ", ";

    os << r.minus.var << " -> " << r.var << "- : ";
	for (i = r.minus.states.begin(); i != r.minus.states.end(); i++)
		os << *i << " ";
    cout << "}";

    return os;
}
