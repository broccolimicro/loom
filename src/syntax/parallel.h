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

#include "sequential.h"

struct parallel : sequential
{
	parallel();
	parallel(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~parallel();

	parallel &operator=(parallel p);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	minterm variant();
	minterm active_variant();
	minterm passive_variant();

	void expand_shortcuts();
	void parse();
	void merge();
	pids generate_states(petri *n, pids f, bids b, minterm filter);
	place simulate_states(place init, minterm filter);

	void branch_place_set(pid from, bid id);
	void branch_trans_set(pid from, bid id);
	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t = "", ostream *fout = &cout);

	void push(instruction *i);
};

#endif
