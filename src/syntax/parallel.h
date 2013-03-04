/*
 * parallel.h
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */


#ifndef parallel_h
#define parallel_h

#include "block.h"

struct parallel : block
{
	parallel();
	parallel(string chp, vspace *vars, string tab, int verbosity);
	~parallel();

	int uid;					// indexes into the state in the state space

	parallel &operator=(parallel p);

	instruction *duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	int generate_states(graph *trans, int init);
	void generate_prs();
	void generate_statevars();
	// void handshaking_reshuffle();
	void bubble_reshuffle();

	void print_hse();

	void push(instruction *i);
};

#endif
