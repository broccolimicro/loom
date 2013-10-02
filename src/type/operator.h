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
#include "../flag_space.h"
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
	operate(sstring raw, type_space *types, flag_space *flags);
	~operate();

	operate &operator=(operate p);

	void parse(sstring raw);

	void print_prs(ostream *fout, sstring prefix, svector<sstring> driven);
};

#endif
