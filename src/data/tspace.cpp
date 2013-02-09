/*
 * tspace.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: nbingham
 */

#include "tspace.h"

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
