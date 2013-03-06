/*
 * space.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "sspace.h"

size_t state_space::size()
{
	return states.size();
}

void state_space::push_back(state s)
{
	states.push_back(s);
}

vector<state>::iterator state_space::begin()
{
	return states.begin();
}

vector<state>::iterator state_space::end()
{
	return states.end();
}

state state_space::operator[](int i)
{
	return states[i];
}

trace state_space::operator()(int i)
{
	trace ret;
	vector<state>::iterator s;

	for (s = states.begin(); s != states.end(); s++)
		ret.push_back((*s)[i]);

	return ret;
}

ostream &operator<<(ostream &os, state_space s)
{
	vector<state>::iterator i;
	for (i = s.begin(); i != s.end(); i++)
		os << *i << endl;
	return os;
}
