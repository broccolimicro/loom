/*
 * expression.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Ned Bingham
 */

#include "common.h"
#include "variable.h"

#ifndef expression_h
#define expression_h

struct expression
{
	expression();
	~expression();

	string parse(string raw, map<string, string> input);
};

#endif
