/*
 * graph.h
 *
 */

#include "../common.h"
#include "tspace.h"
#include "sspace.h"

#ifndef graph_h
#define graph_h

struct graph
{
	state_space states;
	trace_space traces;

	// From				  , To
	// Instruction indexed, Instruction indexed
	vector<vector<int> > edges;
	// Strings that caused given transition
	vector<vector<string> > transitions;
	graph();

	void insert_edge(int from, int to, string chp);
	void push_back(state s);
	void push_back(trace t);

	int size();
	int width();

	/*
	void print_line(int from, graph *trans);
	void print_line_dot(int from, state_space *spaces, graph *trans);
	void print_line_with_trans(int from, graph *trans);
	*/
};

ostream &operator<<(ostream &os, graph g);

#endif
