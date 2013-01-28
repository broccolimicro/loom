/*
 * state.h
 *
 *  Created on: Jan 26, 2013
 *      Author: nbingham
 */

#include "value.h"

#include "common.h"

#ifndef state_h
#define state_h

struct state
{
	state();
	state(vector<value> v);
	~state();
	// Variable indexed using uid
	vector<value> values;

	void clear();
	vector<value>::iterator begin();
	vector<value>::iterator end();
	void insert(int i, value v);
	int size();

	value operator[](int i);
};


state operator+(state s1, state s2);
state operator-(state s1, state s2);
state operator*(state s1, state s2);
state operator/(state s1, state s2);

state operator+(state s1, value s2);
state operator-(state s1, value s2);
state operator*(state s1, value s2);
state operator/(state s1, value s2);

state operator+(value s1, state s2);
state operator-(value s1, state s2);
state operator*(value s1, state s2);
state operator/(value s1, state s2);

state operator-(state s);

state operator&(state s1, state s2);
state operator|(state s1, state s2);

state operator&(state s1, value s2);
state operator|(state s1, value s2);

state operator&(value s1, state s2);
state operator|(value s1, state s2);

state operator~(state s);

state operator<<(state s1, state s2);
state operator>>(state s1, state s2);

state operator<<(state s1, value s2);
state operator>>(state s1, value s2);

state operator<<(value s1, state s2);
state operator>>(value s1, state s2);

state operator<<(state s, int n);
state operator>>(state s, int n);

/*state operator<(state s1, int n);
state operator>(state s1, int n);*/

state operator==(state s1, state s2);
state operator!=(state s1, state s2);
state operator<=(state s1, state s2);
state operator>=(state s1, state s2);
state operator<(state s1, state s2);
state operator>(state s1, state s2);

state operator==(state s1, value s2);
state operator!=(state s1, value s2);
state operator<=(state s1, value s2);
state operator>=(state s1, value s2);
state operator<(state s1, value s2);
state operator>(state s1, value s2);

state operator==(value s1, state s2);
state operator!=(value s1, state s2);
state operator<=(value s1, state s2);
state operator>=(value s1, state s2);
state operator<(value s1, state s2);
state operator>(value s1, state s2);

#endif
