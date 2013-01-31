/*
 * graph.h
 *
 */

#include "common.h"
#include "space.h"
#ifndef graph_h
#define graph_h

struct graph
{
	// From				  , To
	// Instruction indexed, Instruction indexed
	vector<vector<int> > edges;
	// Strings that caused given transition
	vector<vector<string> > transitions;
	graph();
	graph(state_space *spaces);

	void insert_edge(int from, int to, string chp);
	void print_line(int from);
	void print_line_with_trans(int from);
};

ostream &operator<<(ostream &os, graph g);

#endif
