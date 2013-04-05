/*
 * space.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "sspace.h"


//Returns the size (number of states) of the state space
size_t state_space::size()
{
	return states.size();
}

//Returns the width (number of variables) of the state space
size_t state_space::width()
{
	if (states.size() > 0)
		return states[0].size();
	return 0;
}

//Appends a state to the end of the space
void state_space::push_back(state s)
{
	states.push_back(s);
}

//Returns an iterator (pointer) to the first element in the state space
vector<state>::iterator state_space::begin()
{
	return states.begin();
}

//Returns an iterator (pointer) to the last element in the state space
vector<state>::iterator state_space::end()
{
	return states.end();
}

//(Call using var[i]) Returns the ith state in the state space
state &state_space::operator[](int i)
{
	return states[i];
}

//(Call using var(i) ) Return's the ith element in every state of the space
//In other words, the value of every variable with the uid i in the state space (i.e. the trace)
trace state_space::operator()(int i)
{
	trace ret;
	vector<state>::iterator s;

	for (s = states.begin(); s != states.end(); s++)
		ret.push_back((*s)[i]);

	return ret;
}

//Print the state space to the ostream. e.g. cout << var << endl;
ostream &operator<<(ostream &os, state_space s)
{
	vector<state>::iterator i;
	for (i = s.begin(); i != s.end(); i++)
		os << *i << endl;
	return os;
}
