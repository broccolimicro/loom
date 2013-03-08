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

void trace_space::push_back(trace t)
{
	traces.push_back(t);
}

vector<trace>::iterator trace_space::begin()
{
	return traces.begin();
}

vector<trace>::iterator trace_space::end()
{
	return traces.end();
}

trace &trace_space::operator[](int i)
{
	return traces[i];
}

state trace_space::operator()(int i)
{
	state s;

	for (vector<trace>::iterator j = traces.begin(); j != traces.end(); j++)
		s.values.push_back((*j)[i]);

	return s;
}

ostream &operator<<(ostream &os, trace_space t)
{
	vector<trace>::iterator i;
	for (i = t.begin(); i != t.end(); i++)
		os << *i << endl;

	return os;
}
