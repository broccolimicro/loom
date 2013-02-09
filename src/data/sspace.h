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

struct state_space
{
	// Instruction indexed using uid
	vector<state> states;

	int size();
	void push_back(state s);
//	state_space delta_space_gen(state_space spaces, graph trans);

	state operator[](int i);

};

#endif
