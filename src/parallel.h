/*
 * parallel.h
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "block.h"
#include "keyword.h"

#ifndef parallel_h
#define parallel_h

struct parallel : block
{
	parallel();
	parallel(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
	~parallel();

	void parse(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
};

#endif
