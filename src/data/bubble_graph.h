/*
 * bubble_graph.h
 *
 *  Created on: Oct 9, 2013
 *      Author: nbingham
 */

#include "variable_space.h"

#ifndef bubble_graph_h
#define bubble_graph_h

struct bubble_graph
{
	smap<pair<int, int>, pair<bool, bool> > arcs;

	void print(ostream &fout, variable_space *v);
};

#endif
