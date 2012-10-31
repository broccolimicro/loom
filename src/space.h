/*
 * space.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham, Nicholas Kramer
 */

#include "common.h"
#include "state.h"

#ifndef space_h
#define space_h

/* This structure represents a whole state
 * space for a single variable. It contains
 * the name of the variable it represents
 * and a list of state structures.
 */
struct space
{
	space();
	space(string v, list<state> s);
	~space();

	string			var;
	list<state>		states;

	space &operator=(space s);

	space &operator+=(space s);
	space &operator-=(space s);
	space &operator*=(space s);
	space &operator/=(space s);

	space &operator&=(space s);
	space &operator|=(space s);

	space &operator+=(state s);
	space &operator-=(state s);
	space &operator*=(state s);
	space &operator/=(state s);

	space &operator&=(state s);
	space &operator|=(state s);

	space &operator<<=(int n);
	space &operator>>=(int n);
};

ostream &operator<<(ostream &os, space s);

space operator+(space s1, space s2);
space operator-(space s1, space s2);
space operator*(space s1, space s2);
space operator/(space s1, space s2);

space operator+(space s1, state s2);
space operator-(space s1, state s2);
space operator*(space s1, state s2);
space operator/(space s1, state s2);

space operator+(state s1, space s2);
space operator-(state s1, space s2);
space operator*(state s1, space s2);
space operator/(state s1, space s2);

space operator-(space s);

space operator&(space s1, space s2);
space operator|(space s1, space s2);

space operator&(space s1, state s2);
space operator|(space s1, state s2);

space operator&(state s1, space s2);
space operator|(state s1, space s2);

space operator~(space s);

space operator<<(space s, int n);
space operator>>(space s, int n);

space operator==(space s1, space s2);
space operator!=(space s1, space s2);
space operator<=(space s1, space s2);
space operator>=(space s1, space s2);
space operator<(space s1, space s2);
space operator>(space s1, space s2);

space operator==(space s1, state s2);
space operator!=(space s1, state s2);
space operator<=(space s1, state s2);
space operator>=(space s1, state s2);
space operator<(space s1, state s2);
space operator>(space s1, state s2);

space operator==(state s1, space s2);
space operator!=(state s1, space s2);
space operator<=(state s1, space s2);
space operator>=(state s1, space s2);
space operator<(state s1, space s2);
space operator>(state s1, space s2);

#endif
