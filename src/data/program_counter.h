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
struct node;

typedef pair<int, int> unode;
typedef pair<unode, unode> uarc;

struct umap
{
	umap();
	umap(const umap &um);
	~umap();

	list<uarc> arcs;
	list<unode> begin;
	list<pair<unode, minterm> > end;
	svector<int> sids;
	svector<pair<int, int> > tids;
	svector<int> iac;

	list<uarc> input_arcs(unode n);
	list<uarc> output_arcs(unode n);
	list<unode> input_nodes(unode n);
	list<unode> output_nodes(unode n);
	bool path_contains(unode n);
	int nid(int i);

	umap &operator=(umap um);
};

struct remote_program_counter
{
	remote_program_counter();
	remote_program_counter(const remote_program_counter &pc);
	remote_program_counter(string name, petri *from, petri *to);
	remote_program_counter(int index, petri *net);
	~remote_program_counter();

	string name;
	petri* net;

	svector<pair<int, int> > forward_factors;
	svector<pair<int, int> > reverse_factors;
	svector<int>			 hidden_factors;

	umap index;

	logic &net_index(int i);
	bool is_place(int i);
	bool is_trans(int i);
	bool is_active(int i);
	bool is_satisfied(int i, logic s);
	bool is_vacuous(int i, logic s);
	bool is_one(int i);

	void roll_to(unode idx);

	int count(unode n);

	minterm firings();
	logic waits(unode n);

	remote_program_counter &operator=(remote_program_counter pc);
};

bool operator==(remote_program_counter pc1, remote_program_counter pc2);

struct program_environment
{
	program_environment();
	program_environment(const remote_program_counter &pc);
	program_environment(const program_environment &env);
	~program_environment();

	list<remote_program_counter> pcs;

	minterm firings();

	program_environment &operator=(program_environment env);
};

bool operator==(program_environment env1, program_environment env2);

struct program_counter
{
	program_counter();
	program_counter(const program_counter &pc);
	program_counter(int index, petri *net);
	~program_counter();

	int last;
	int index;
	petri *net;
	bool waiting;

	list<program_environment> envs;
	list<pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > > > splits;

	bool is_trans();
	bool is_place();
	bool is_active();
	string node_name();

	bool end_is_ready(logic s, list<program_environment>::iterator &env, list<remote_program_counter>::iterator &pc, list<pair<unode, minterm> >::iterator &idx, logic &state);
	bool begin_is_ready(logic s, list<program_environment>::iterator &env, list<remote_program_counter>::iterator &pc, list<unode>::iterator &idx, list<pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > > >::iterator &split, list<pair<list<program_environment>::iterator, unode> >::iterator &prgm, logic &state);

	void simulate(logic s);
	logic mute(logic s);

	int count();
	void merge();

	program_counter &operator=(program_counter pc);
};

struct program_execution
{
	program_execution();
	program_execution(const program_execution &exe);
	program_execution(petri *net);
	~program_execution();

	petri *net;
	svector<logic> states;
	svector<logic> final;

	list<program_counter> pcs;
	bool deadlock;
	bool done;

	int count(list<program_counter>::iterator i);
	void merge(list<program_counter>::iterator i);

	logic calculate_state(list<program_counter>::iterator i);

	program_execution &operator=(program_execution exe);
};

struct program_execution_space
{
	program_execution_space();
	~program_execution_space();

	svector<logic> final;
	list<program_execution> execs;

	void simulate();
};

#endif
