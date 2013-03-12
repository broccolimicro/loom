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
	graph();
	~graph();

	state_space states;
	trace_space traces;

	trace_space delta;
	trace_space up;
	trace_space down;


	vector<vector<int> > up_firings;
	vector<vector<int> > down_firings;
	map<int, vector<int> > up_conflicts;
	map<int, vector<int> > down_conflicts;

	// From				  , To
	// Instruction indexed, Instruction indexed
	vector<vector<int> > edges;
	// Strings that caused given transition
	vector<vector<string> > transitions;

	void insert(state s, vector<int> from, vector<string> chp = vector<string>());
	void insert(state s, int from = -1, string chp = "");
	void insert_edge(int from, int to, string chp);

	void gen_conflicts();
	void gen_traces();
	void gen_deltas();

	int size();
	int width();

	void print_up();
	void print_down();
	void print_dot();
	void print_delta();
};

ostream &operator<<(ostream &os, graph g);
ostream &operator>>(ostream &os, graph g);

#endif
