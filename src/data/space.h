/*
 * space.h
 *
 *  Created on: Jan 26, 2013
 *      Author: nbingham
 */

#include "state.h"
#include "trace.h"
#include "graph.h"

#ifndef space_h
#define space_h

struct state_space
{
	// Instruction indexed using uid
	vector<state> states;

	int size();
	void push_back(state s);
//	state_space delta_space_gen(state_space spaces, graph trans);

	state operator[](int i);

};

struct trace_space
{
	// Variable indexed using uid
	vector<trace> traces;

	int size();
	void assign(int i, trace t);

	trace operator[](int i);
};

#endif
