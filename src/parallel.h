/*
 * parallel.h
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham
 */

#include "block.h"
#include "keyword.h"

#ifndef parallel_h
#define parallel_h

struct parallel : block
{
	parallel();
	parallel(string raw, map<string, keyword*> types, map<string, variable*> vars, string tab);
	~parallel();

	void parse(string raw, map<string, keyword*> types, map<string, variable*> vars, string tab);
};

#endif
