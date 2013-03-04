/*
 * tspace.h
 *
 *  Created on: Feb 9, 2013
 *      Author: nbingham
 */

#ifndef tspace_h
#define tspace_h

#include "../common.h"
#include "trace.h"
#include "state.h"

struct trace_space
{
	// Variable indexed using uid
	vector<trace> traces;

	int size();
	void assign(int i, trace t);
	void push_back(trace t);

	vector<trace>::iterator begin();
	vector<trace>::iterator end();

	trace operator[](int i);
	state operator()(int i);
};

#endif
