/*
 * graph.h
 *
 *  Created on: Jan 26, 2013
 *      Author: nbingham
 */

#include "common.h"

#ifndef graph_h
#define graph_h

struct graph
{
	// From				  , To
	// Instruction indexed, Instruction indexed
	vector<vector<int> > edges;
};

#endif
