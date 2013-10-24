/*
 * process.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#ifndef process_h
#define process_h

#include "../common.h"
#include "../data.h"
#include "../syntax.h"
#include "../flag_space.h"
#include "keyword.h"

/* This structure represents a process. Processes act in parallel
 * with each other and can only communicate with other processes using
 * channels. We need to keep track of the sequential that defines this process and
 * the input and output signals. The final element in this structure is
 * a list of production rules that are the result of the compilation.
 */
struct process : keyword
{
	process();
	process(sstring raw, type_space *types, flag_space *flags);
	~process();

	sstring					chp;	// the raw process definition
	parallel				def;	// the chp that defined this process
	rule_space				prs;
	variable_space			vars;
	list<sstring>			args;
	petri					net;
	flag_space				*flags;

	bool 					is_inline;

	process &operator=(process p);

	void parse(sstring raw);
	void simulate();
	void rewrite();
	void project();
	void decompose();
	void reshuffle();

	void generate_states();
	void trim_states();
	void direct_bubble_reshuffle();
	svector<pair<svector<int>, bool> > reshuffle_algorithm(smap<pair<int, int>, pair<bool, bool> >::iterator idx, bool forward, smap<pair<int, int>, pair<bool, bool> > *net, svector<int> cycle, svector<bool> *inverted);
	bool insert_state_vars();
	bool insert_bubbleless_state_vars();
	void generate_prs();
	void generate_bubbleless_prs();
	void bubble_reshuffle();
	void factor_prs();



	void generate_paths(pair<int, int> *up_sense_count, svector<int> up_start, path_space *up_paths, pair<int, int> *down_sense_count, svector<int> down_start,  path_space *down_paths);
	void generate_positive_paths(pair<int, int> *up_sense_count, svector<int> up_start, path_space *up_paths, pair<int, int> *down_sense_count, svector<int> down_start,  path_space *down_paths);
	void generate_negative_paths(pair<int, int> *up_sense_count, svector<int> up_start, path_space *up_paths, pair<int, int> *down_sense_count, svector<int> down_start,  path_space *down_paths);
	void remove_invalid_split_points(pair<int, int> up_sense_count, svector<int> up_start, path_space *up_paths, pair<int, int> down_sense_count, svector<int> down_start, path_space *down_paths);
	svector<int> choose_split_points(path_space *paths, bool up, bool down);

	void print_hse(ostream *fout = &cout);
	void print_dot(ostream *fout = &cout);
	void print_prs(ostream *fout = &cout);
};

#endif
