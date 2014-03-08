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
#include "petri.h"

struct remote_program_counter;

/**
 * This elaborator assumes nothing. It clears all of the previously known
 * state encodings before starting and then systematically explores the
 * entire state space in order to elaborate it and suffers horribly
 * from state space explosion. Some actions have been taken to try
 * and account for this. For example, the algorithm does a depth first
 * search keeping the total storage needed to a minimum. It does a form
 * of dynamic programing, ignoring cases that it has seen before. It
 * schedules case in which the environment can move to be considered first
 * because cases where the main program moves first generally
 * only produce a strict subset of the state encodings produced by the cases
 * where the environment moves first.
 */
struct program_counter
{
	program_counter();
	program_counter(string name, bool elaborate, petri_index index, petri_net *net);
	~program_counter();

	string name;
	petri_net *net;
	petri_index index;
	minterm state;
	bool done;
	bool elaborate;

	bool is_active();
	bool is_active(petri_index i);
	bool is_satisfied();
	bool is_satisfied(petri_index i);
	bool next_has_active_or_satisfied();
	svector<petri_index> output_nodes();
	svector<petri_index> input_nodes();
	canonical &predicate();

	void apply(minterm term);
	void set(minterm term);
};

bool operator==(program_counter p1, program_counter p2);

struct remote_petri_index : petri_index
{
	remote_petri_index();
	remote_petri_index(int idx, int iter, bool place);
	remote_petri_index(petri_index i, int iter);
	~remote_petri_index();

	int iteration;
};

bool operator==(remote_petri_index i1, remote_petri_index i2);

typedef pair<remote_petri_index, remote_petri_index> remote_petri_arc;

struct remote_program_counter
{
	remote_program_counter();
	remote_program_counter(string name, petri_net *net);
	~remote_program_counter();

	string name;
	petri_net* net;
	svector<remote_petri_index> begin;
	svector<remote_petri_index> end;
	svector<remote_petri_arc> arcs;
	svector<int> place_iteration;
	svector<pair<int, int> > trans_iteration;
	svector<int> input_size;

	svector<remote_petri_arc> input_arcs(remote_petri_index n);
	svector<remote_petri_arc> output_arcs(remote_petri_index n);
	svector<remote_petri_index> input_nodes(remote_petri_index n);
	svector<remote_petri_index> output_nodes(remote_petri_index n);

	bool is_active(petri_index i);
	bool is_satisfied(petri_index i, minterm state);
	bool is_vacuous(petri_index i, minterm state);
	bool next_has_active_or_satisfied(remote_petri_index i, minterm state, svector<petri_index> &outgoing);
	bool is_one(petri_index i);

	int nid(petri_index idx);

	void roll_to(remote_petri_index idx);

	int count(int n);
	void merge(int n);

	minterm firings();
	canonical waits(remote_petri_index n);

	remote_program_counter &operator=(remote_program_counter pc);
};

ostream &operator<<(ostream &os, remote_petri_index i);

struct program_execution
{
	program_execution();
	program_execution(const program_execution &exec);
	~program_execution();

	svector<program_counter> pcs;
	svector<remote_program_counter> rpcs;
	bool deadlock;

	int count(int pci);
	int merge(int pci);
	bool done();

	void init_pcs(string name, petri_net *net, bool elaborate);
	void init_rpcs(string name, petri_net *net);
};

struct program_execution_space
{
	svector<program_execution> execs;
	svector<petri_net*> nets;
	map<pair<pair<string, petri_net*>, pair<string, petri_net*> >, svector<pair<int, int> > > translations;

	void duplicate_execution(program_execution *exec_in, program_execution **exec_out);
	void duplicate_counter(program_execution *exec_in, int pc_in, int &pc_out);
	bool remote_end_ready(program_execution *exec, int &rpc, int &idx, svector<petri_index> &outgoing);
	bool remote_begin_ready(program_execution *exec, int &rpc, int &idx);
	void full_elaborate();
	void reset();
	void gen_translation(string name0, petri_net *net0, string name1, petri_net *net1);
	minterm translate(string name0, petri_net *net0, minterm t, string name1, petri_net *net1);
};

#endif
