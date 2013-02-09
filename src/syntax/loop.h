/*
 * loop.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */


#ifndef loop_h
#define loop_h

#include "conditional.h"

struct loop : conditional
{
	loop();
	loop(string raw, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity);
	~loop();

	vector<int> uid;

	loop &operator=(loop l);

	instruction *duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse(map<string, keyword*> types);
	int generate_states(state_space *space, graph *trans, int init);
	void generate_prs();
	void generate_statevars();
	// void handshaking_reshuffle();
	void bubble_reshuffle();

	void print_hse();
};

#endif
