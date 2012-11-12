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
