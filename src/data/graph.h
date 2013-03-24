/*
 * graph.h
 *
 */

#include "../common.h"
#include "tspace.h"
#include "sspace.h"
#include "pspace.h"

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
	vector<vector<int> > up_firings_transpose;
	vector<vector<int> > down_firings;
	vector<vector<int> > down_firings_transpose;
	vector<vector<int> > up_conflicts;
	vector<vector<int> > down_conflicts;

	// From				  , To
	// Instruction indexed, Instruction indexed
	vector<vector<int> > front_edges;
	vector<vector<int> > back_edges;
	// Strings that caused given transition
	vector<vector<string> > transitions;

	void append_state(state s, vector<int> from, vector<string> chp = vector<string>());
	void append_state(state s, int from = -1, string chp = "");
	int insert_state(state s, int from);
	int duplicate_state(int from);
	void insert_edge(int from, int to, string chp);
	path_space get_paths(int from, int to, path p);
	void	   get_trace(int from, int up, int down, trace *result);
	void	   get_trace(int from, vector<bool> *up, vector<bool> *down, trace *result);

	void set_trace(int uid, trace t);

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
