/*
 * parallel.h
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "block.h"
#include "keyword.h"

#ifndef parallel_h
#define parallel_h

struct parallel : block
{
	parallel();
	parallel(string chp, map<string, keyword*> *types, map<string, variable*> globals, string tab, int verbosity);
	~parallel();

	void expand_shortcuts();
	void parse(map<string, keyword*> *types);
	void generate_states(state_space *space, graph *trans, int init);
	void generate_prs(map<string, variable*> globals);
	void generate_statevars();
	// void handshaking_reshuffle();
	void bubble_reshuffle();
};

#endif
