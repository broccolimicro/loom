/*
 * space.h
 *
 *  Created on: Jan 26, 2013
 *      Author: nbingham
 */

#include "state.h"
#include "trace.h"

#ifndef space_h
#define space_h

struct state_space
{
	// Instruction indexed using uid
	vector<state> states;

	int size();
};

struct trace_space
{
	// Variable indexed using uid
	vector<trace> traces;

	int size();
};

#endif
