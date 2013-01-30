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
	graph();
	graph(state_space *spaces);

	void insert_edge(int from, int to);
	void print_line(int from);
};

ostream &operator<<(ostream &os, graph g);

#endif
