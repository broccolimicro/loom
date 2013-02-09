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

struct trace_space
{
	// Variable indexed using uid
	vector<trace> traces;

	int size();
	void assign(int i, trace t);

	trace operator[](int i);
};

#endif
