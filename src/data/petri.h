/*
 * petri.h
 *
 *  Created on: May 9, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "canonical.h"
#include "path_space.h"
#include "../flag_space.h"

#ifndef petri_h
#define petri_h

struct instruction;
struct variable_space;
struct rule_space;
struct petri_net;

struct petri_index
{
	petri_index();
	petri_index(int idx, bool place);
	~petri_index();

	int data;

	bool is_place() const;
	bool is_trans() const;
	sstring name() const;
	int idx() const;

	//operator int() const;

	petri_index &operator=(petri_index i);
	petri_index &operator--();
	petri_index &operator++();
	petri_index &operator--(int);
	petri_index &operator++(int);
};

bool operator==(petri_index i, petri_index j);
bool operator!=(petri_index i, petri_index j);
bool operator<(petri_index i, int j);
ostream &operator<<(ostream &os, petri_index i);
bool operator>(petri_index i, petri_index j);
bool operator<(petri_index i, petri_index j);
bool operator>=(petri_index i, petri_index j);
bool operator<=(petri_index i, petri_index j);
petri_index operator+(petri_index i, int j);
petri_index operator-(petri_index i, int j);

typedef pair<petri_index, petri_index> petri_arc;

/**
 * A state is a collection of concurrent places that form
 * a complete graph cut.
 */
struct petri_state
{
	petri_state();
	petri_state(petri_net *net, svector<petri_index> init, bool backward = false);
	~petri_state();

	svector<petri_index> state;

	int count(int j);
	void merge(int j);

	bool is_state();

	petri_state &operator=(petri_state s);
};

ostream &operator<<(ostream &os, petri_state s);
bool operator==(petri_state s1, petri_state s2);
bool operator!=(petri_state s1, petri_state s2);
bool operator<(petri_state s1, petri_state s2);
bool operator>(petri_state s1, petri_state s2);
bool operator<=(petri_state s1, petri_state s2);
bool operator>=(petri_state s1, petri_state s2);

struct petri_node
{
	petri_node();
	petri_node(canonical index, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	~petri_node();

	instruction *owner;
	map<petri_state, canonical> observed;
	smap<int, int> pbranch;
	smap<int, int> cbranch;
	bool active;

	bool definitely_vacuous;
	bool possibly_vacuous;
	bool definitely_invacuous;

	canonical index;
	canonical positive;	// Negative sense variables are hidden
	canonical negative;	// Positive sense variables are hidden

	canonical assumptions;
	svector<canonical> assertions;

	pair<int, int> sense_count();
};

struct petri_net
{
	petri_net();
	~petri_net();

	svector<petri_node> S;
	svector<petri_node> T;
	svector<petri_index> M0;
	svector<petri_arc> arcs;

	variable_space *vars;
	rule_space *prs;
	flag_space *flags;
	int pbranch_count;
	int cbranch_count;

	smap<petri_index, list<svector<petri_index> > > conflicts;
	smap<petri_index, list<svector<petri_index> > > indistinguishable;
	smap<petri_index, list<svector<petri_index> > > positive_conflicts;
	smap<petri_index, list<svector<petri_index> > > positive_indistinguishable;
	smap<petri_index, list<svector<petri_index> > > negative_conflicts;
	smap<petri_index, list<svector<petri_index> > > negative_indistinguishable;
	int max_indistinguishables;
	int max_positive_indistinguishables;
	int max_negative_indistinguishables;

	smap<int, svector<int> > isochronics;

	smap<int, pair<petri_index, petri_index> > conditional_places;
	svector<int> variable_usage;

	// Functions for adding, removing, and connecting nodes
	petri_index put_place(canonical root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	petri_index put_transition(canonical root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	svector<petri_index> put_places(svector<canonical> root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	svector<petri_index> put_transitions(svector<canonical> root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	void cut(petri_index node);
	void cut(svector<petri_index> nodes);
	svector<petri_index> connect(svector<petri_index> from, svector<petri_index> to);
	petri_index connect(svector<petri_index> from, petri_index to);
	svector<petri_index> connect(petri_index from, svector<petri_index> to);
	petri_index connect(petri_index from, petri_index to);
	petri_index push_transition(petri_index from, canonical root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	petri_index push_transition(svector<petri_index> from, canonical root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	petri_index push_transition(petri_index from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	petri_index push_transition(svector<petri_index> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	svector<petri_index> push_transitions(petri_index from, svector<canonical> root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	svector<petri_index> push_transitions(svector<petri_index> from, svector<canonical> root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	petri_index push_place(petri_index from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	petri_index push_place(svector<petri_index> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	svector<petri_index> push_places(svector<petri_index> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	void pinch_forward(petri_index n);
	void pinch_backward(petri_index n);
	void insert(int a, canonical root, bool active);
	void insert_alongside(petri_index from, petri_index to, canonical root, bool active, instruction *owner);
	petri_index duplicate(petri_index n);
	svector<petri_index> duplicate(svector<petri_index> n);
	petri_index merge(petri_index n0, petri_index n1);
	petri_index merge(svector<petri_index> n);

	// Functions for traversing the network
	svector<petri_index> next(petri_index n);
	svector<petri_index> next(svector<petri_index> n);
	svector<petri_index> prev(petri_index n);
	svector<petri_index> prev(svector<petri_index> n);
	svector<int> outgoing(petri_index n);
	svector<int> outgoing(svector<petri_index> n);
	svector<int> incoming(petri_index n);
	svector<int> incoming(svector<petri_index> n);
	svector<int> next_arc(int n);
	svector<int> next_arc(svector<int> n);
	svector<int> prev_arc(int n);
	svector<int> prev_arc(svector<int> n);

	// Connectivity and sibling checks
	bool is_floating(petri_index n);
	bool are_connected(petri_index n0, petri_index n1);
	bool have_same_source(petri_index n0, petri_index n1);
	bool have_same_dest(petri_index n0, petri_index n1);
	int are_parallel_siblings(petri_index p0, petri_index p1);
	int are_conditional_siblings(petri_index p0, petri_index p1);

	// Accessor functions
	petri_node &operator[](petri_index i);
	petri_node &at(petri_index i);

	// Functions for handling paths
	pair<int, int> closest_input(svector<int> from, svector<int> to);
	pair<int, int> closest_output(svector<int> from, svector<int> to);
	void get_paths(svector<int> from, svector<int> to, path_space *result);
	void zero_paths(path_space *paths, petri_index from);
	void zero_paths(path_space *paths, svector<petri_index> from);
	void zero_ins(path_space *paths, petri_index from);
	void zero_ins(path_space *paths, svector<petri_index> from);
	void zero_outs(path_space *paths, petri_index from);
	void zero_outs(path_space *paths, svector<petri_index> from);
	svector<int> start_path(int from, svector<int> ex);
	svector<int> start_path(svector<int> from, svector<int> ex);
	svector<petri_index> end_path(petri_index to, svector<petri_index> ex);
	svector<petri_index> end_path(svector<petri_index> to, svector<petri_index> ex);


	void expand();
	void generate_observed();
	void check_assertions();



	canonical base(svector<int> idx);



	petri_index get_split_place(petri_index merge_place, svector<bool> *covered);

	void add_conflict_pair(smap<petri_index, list<svector<petri_index> > > *c, petri_index i, petri_index j);

	void gen_conditional_places();
	bool are_sibling_guards(petri_index i, petri_index j);
	void gen_conflicts();
	void gen_bubbleless_conflicts();
	void gen_senses();
	void trim_branch_ids();

	smap<pair<int, int>, pair<bool, bool> > gen_isochronics();
	canonical apply_debug(int pc);

	canonical get_effective_place_encoding(petri_index place, petri_index observer);
	canonical get_effective_state_encoding(petri_state state, petri_state observer);

	/**
	 * \brief	Removes vacuous pbranches, unreachable places, and dangling, vacuous, and impossible transitions.
	 * \sa		merge_conflicts() and zip()
	 */
	void compact();
	bool trim();

	/**
	 * \brief	Groups places that have indistinguishable state encodings, then merges each group into one place.
	 * \pre		This function assumes that it is given a petri-net in which every transition
	 * 			has a single input place and a single output place which are only connected
	 * 			to that transition.
	 * \sa		trim() and zip()
	 */
	void merge_conflicts();

	/**
	 * \brief	Groups places and groups transitions based upon their neighborhood, then merges each group into one place or transition.
	 * \sa		trim() and merge_conflicts()
	 */
	void zip();

	pair<int, int> get_input_sense_count(petri_index idx);
	pair<int, int> get_input_sense_count(svector<petri_index> idx);



	void print_dot(ostream *fout, sstring name);
	void print_branch_ids(ostream *fout);
	void print_conflicts(ostream &fout, string name);
	void print_indistinguishables(ostream &fout, string name);
	void print_positive_conflicts(ostream &fout, string name);
	void print_positive_indistinguishables(ostream &fout, string name);
	void print_negative_conflicts(ostream &fout, string name);
	void print_negative_indistinguishables(ostream &fout, string name);
};

#endif
