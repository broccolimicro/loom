/*
 * tspace.h
 *
 * tspace, or trace space, is a collection of traces for every variable in a program.
 * A tspace can be thought of as the transpose of a state space. The primary tspace for
 * a program is held in graph.
 */

#ifndef tspace_h
#define tspace_h

#include "../common.h"
#include "trace.h"
#include "state.h"

struct trace_space
{
	trace_space();
	trace_space(int s);
	~trace_space();

	// Variable indexed using uid
	vector<trace> traces;

	int size();
	void assign(int i, trace t);
	void push_back(trace t);

	vector<trace>::iterator begin();
	vector<trace>::iterator end();

	trace &operator[](int i);
	state operator()(int i);
};

ostream &operator<<(ostream &os, trace_space t);

#endif
