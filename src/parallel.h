/*
 * parallel.h
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham
 */

#include "block.h"

#ifndef parallel_h
#define parallel_h

struct parallel : block
{
	parallel();
	parallel(string raw, map<string, variable*> svars, string tab);
	~parallel();

	void parse(string raw, map<string, variable*> svars, string tab);
};

#endif
