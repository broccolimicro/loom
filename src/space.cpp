/*
 * space.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "space.h"
#include "state.h"
#include "common.h"


int state_space::size()
{
	return states.size();
}

state state_space::operator[](int i)
{
	return states[i];
}


void state_space::push_back(state s)
{
	states.push_back(s);
}

int trace_space::size()
{
	return traces.size();
}

void trace_space::assign(int i, trace t)
{
	if (i >= (int)traces.size())
		traces.resize(i+1, trace());
	traces[i] = t;
}

trace trace_space::operator[](int i)
{
	return traces[i];
}
