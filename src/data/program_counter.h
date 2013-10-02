/*
 * program_counter.h
 *
 *  Created on: Sep 16, 2013
 *      Author: nbingham
 */

#ifndef program_counter_h
#define program_counter_h

#include "canonical.h"

struct petri;

struct program_counter
{
	program_counter();
	program_counter(petri *net, int idx);
	~program_counter();

	petri *net;
	int index;
	svector<int> last;
	logic delta;
};

/**
 * This represents one particular combination of paths through the
 * state space. All program counters must be able to make progress
 * for there not to be deadlock. Program counters are added to a space
 * upon a parallel split and removed upon a parallel merge. Different
 * spaces are created to handle conditional splits, where each space
 * represents a different choice for the path taken.
 */
struct program_counter_space
{
	program_counter_space();
	program_counter_space(petri *net, int idx);
	~program_counter_space();

	svector<program_counter> pcs;
	svector<bool> cov;

	void check_size(int s);

	program_counter_space &operator=(program_counter_space s);
};

#endif
