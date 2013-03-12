/*
 * sspace.h
 *
 *  Created on: Jan 26, 2013
 *      Author: nbingham
 */

#ifndef sspace_h
#define sspace_h

#include "../common.h"
#include "state.h"
#include "trace.h"

struct state_space
{
	// Instruction indexed using uid
	vector<state> states;

	size_t size();
	size_t width();
	void push_back(state s);

	vector<state>::iterator begin();
	vector<state>::iterator end();

	state &operator[](int i);
	trace operator()(int i);

};

ostream &operator<<(ostream &os, state_space s);

#endif
