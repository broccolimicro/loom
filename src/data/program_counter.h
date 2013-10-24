/*
 * program_counter.h
 *
 *  Created on: Sep 16, 2013
 *      Author: nbingham
 */

#ifndef program_counter_h
#define program_counter_h

#include "../common.h"
#include "canonical.h"

struct petri;

struct program_counter
{
	program_counter();
	program_counter(sstring name, petri *from, petri *to, int pc);
	program_counter(program_counter l, int p);
	~program_counter();

	sstring name;
	petri *net;

	int pc;
	svector<minterm>		 firings;
	svector<int>			 indices;
	svector<pair<int, int> > arcs;
	svector<int>			 begin;

	svector<pair<int, int> > forward_factors;
	svector<pair<int, int> > reverse_factors;
	svector<int>			 hidden;

	minterm mute();
	logic translate(logic state);
	void update(logic state);
	void reset();

	logic &index();
	bool is_active();
	bool is_place();
	bool is_trans();
};

struct program_counter_space
{
	program_counter_space();
	~program_counter_space();

	svector<program_counter> pcs;

	void simulate(petri *net, logic state, int i = 0);
	logic mute(logic state);
	void reset();

	void increment(int i);
	int check_merges(int j);
};

#endif
