/* sspace.h
 *
 * sspace is the data structure that holds a collection (the whole space)
 * of all the states of a data set. The most common use is in graph, which
 * contains a state space for the whole program. In this example, the state
 * space represents every state which the program can find itself in. Another
 * common use is a 'diff space', which is the dual of a state space graph;
 * every edge becomes a state that marks the difference between the two sides of
 * the edge.
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

	void remove(int i);
};

ostream &operator<<(ostream &os, state_space s);

#endif
