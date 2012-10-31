/*
 * state.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Ned Bingham
 */

#include "common.h"

#ifndef state_h
#define state_h

struct state
{
	state();
	state(string d, bool p);
	~state();

	string data;
	bool prs;		// If this is 1, then we need to generate production rules for this state. Otherwise, we don't.

	state &operator=(state s);
	state &operator=(string s);

	state &operator+=(state s);
	state &operator-=(state s);
	state &operator*=(state s);
	state &operator/=(state s);

	state &operator&=(state s);
	state &operator|=(state s);

	state &operator<<=(int n);
	state &operator>>=(int n);
};

ostream &operator<<(ostream &os, state s);

state operator+(state s1, state s2);
state operator-(state s1, state s2);
state operator*(state s1, state s2);
state operator/(state s1, state s2);

state operator-(state s);

state operator&(state s1, state s2);
state operator|(state s1, state s2);

state operator~(state s);

state operator<<(state s, int n);
state operator>>(state s, int n);

state operator==(state s1, state s2);
state operator!=(state s1, state s2);
state operator<=(state s1, state s2);
state operator>=(state s1, state s2);
state operator<(state s1, state s2);
state operator>(state s1, state s2);

#endif
