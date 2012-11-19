/*
 * assertion.h
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"

#ifndef assertion_h
#define assertion_h

struct assertion
{
	assertion();
	assertion(string raw);
	~assertion();

	void parse(string raw);
};

#endif
