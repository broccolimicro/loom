/*
 * state.h
 *
 * State holds known information about every variable at a given instance of a program's execution.
 * States are used prodominantly in sspace, but sometimes as containers to hold one piece of information
 * for all variables.
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
	//Should this state be used to generate state variables from?
	vector<unsigned char> prs;
	map<int, int> branch;



	void clear();
	vector<value>::iterator begin();
	vector<value>::iterator end();
	void assign(int i, value v, value r = value("?"));
	int size();

	bool fire(int uid);
	void drive(int uid);
	void drive(int uid, value v, value r = value("?"));

	value &operator[](int i);

	state &operator=(state s);

	state &operator&=(state s);
	state &operator|=(state s);

	state &operator&=(value s);
	state &operator|=(value s);
};

state null(int s);
state full(int s);

bool is_all_x(state s1);

bool subset(state s1, state s2);
bool conflict(state s1, state s2);
bool up_conflict(state s1, state s2);
bool down_conflict(state s1, state s2);

int which_index_unneeded(state s1, state s2);
int who_weaker(state s1, state s2);

ostream &operator<<(ostream &os, state s);

state operator&(state s1, state s2);
state operator|(state s1, state s2);

state operator&(state s1, value s2);
state operator|(state s1, value s2);

state operator&(value s1, state s2);
state operator|(value s1, state s2);

state operator~(state s);

bool operator==(state s1, state s2);
bool operator!=(state s1, state s2);

state operator||(state s1, state s2);
state operator&&(state s1, state s2);
state operator!(state s);


state diff(state s1, state s2);
int diff_count(state s1, state s2);

#endif
