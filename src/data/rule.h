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

#ifndef rule_h
#define rule_h

struct rule
{
	rule();
	~rule();

	string left, right;
	trace actual, desired;

	rule &operator=(rule s);

	void clear(int n);
	int index(int n);
};

ostream &operator<<(ostream &os, rule r);

#endif
