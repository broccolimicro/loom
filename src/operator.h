/*
 * process.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "keyword.h"
#include "block.h"
#include "common.h"
#include "process.h"

#ifndef operator_h
#define operator_h

/* This structure represents a process. Processes act in parallel
 * with each other and can only communicate with other processes using
 * channels. We need to keep track of the block that defines this process and
 * the input and output signals. The final element in this structure is
 * a list of production rules that are the result of the compilation.
 */
struct operate : process
{
	operate();
	operate(string raw, map<string, keyword*> types, map<string, variable> vars, int verbosity);
	~operate();

	operate &operator=(operate p);

	void parse(string raw, map<string, keyword*> types, map<string, variable> vars, int verbosity);
};

#endif
