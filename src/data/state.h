/*
  * state.h
 *
 *  Created on: Jan 26, 2013
 *      Author: nbingham
 */

#include "value.h"
#include "../common.h"

#ifndef state_h
#define state_h

struct state
{
	state();
	state(vector<value> v);
	state(value v, int c);
	~state();
	// Variable indexed using uid
	vector<value> values;
	//Optional tag field to 'label' states.
	//In diff spaces, the tag marks the 'from' state.
	//In an implicants list (see rule) it is the state that must cause a fire.
	int tag;
	//Should this state be used to generate state variables from?
	bool prs;
	void clear();
	vector<value>::iterator begin();
	vector<value>::iterator end();
	void assign(int i, value v, value r = value("?"));
	int size();

	value &operator[](int i);

	state &operator=(state s);

	state &operator+=(state s);
	state &operator-=(state s);
	state &operator*=(state s);
	state &operator/=(state s);

	state &operator&=(state s);
	state &operator|=(state s);

	state &operator<<=(state s);
	state &operator>>=(state s);

	state &operator+=(value s);
	state &operator-=(value s);
	state &operator*=(value s);
	state &operator/=(value s);

	state &operator&=(value s);
	state &operator|=(value s);

	state &operator<<=(value s);
	state &operator>>=(value s);

	state &operator<<=(int n);
	state &operator>>=(int n);
};

bool is_all_x(state s1);

bool subset(state s1, state s2);
bool up_subset(state s1, state s2);
bool down_subset(state s1, state s2);

int which_index_unneeded(state s1, state s2);
int who_weaker(state s1, state s2);

ostream &operator<<(ostream &os, state s);

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

bool operator==(state s1, state s2);
bool operator!=(state s1, state s2);
state operator<=(state s1, state s2);
state operator>=(state s1, state s2);
state operator<(state s1, state s2);
state operator>(state s1, state s2);

state operator||(state s1, state s2);
state operator&&(state s1, state s2);
state operator!(state s);

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


state diff(state s1, state s2);
#endif
