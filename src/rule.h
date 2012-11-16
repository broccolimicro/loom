/*
 * rule.h
 *
 *  Created on: Nov 11, 2012
 *      Author: Ned Bingham
 */

#include "common.h"
#include "space.h"

#ifndef rule_h
#define rule_h

struct rule
{
	rule();
	~rule();

	space left, right;

	rule &operator=(rule s);

	void clear(int n);
};

ostream &operator<<(ostream &os, rule r);

#endif
