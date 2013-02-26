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

state state_space::operator[](int i)
{
	return states[i];
}

void state_space::push_back(state s)
{
	states.push_back(s);
}
