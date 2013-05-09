/*
 * process.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#ifndef operator_h
#define operator_h

#include "../common.h"
#include "../data.h"
#include "keyword.h"
#include "process.h"

/* This structure represents a process. Processes act in parallel
 * with each other and can only communicate with other processes using
 * channels. We need to keep track of the sequential that defines this process and
 * the input and output signals. The final element in this structure is
 * a list of production rules that are the result of the compilation.
 */
struct operate : process
{
	operate();
	operate(string raw, map<string, keyword*> *types, int verbosity);
	~operate();

	operate &operator=(operate p);

	void parse(string raw, int verbosity);
};

#endif
