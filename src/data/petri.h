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
#include "program_counter.h"

#ifndef petri_h
#define petri_h

struct instruction;
struct variable_space;
struct rule_space;

struct node
{
	node();
	node(logic index, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	~node();

	instruction *owner;
	svector<int> tail;		// The set of inactive states preceding an active state
	smap<int, int> pbranch;
	smap<int, int> cbranch;
	bool active;

	bool definitely_vacuous;
	bool possibly_vacuous;
	bool definitely_invacuous;

	logic index;
	logic positive;	// Negative sense variables are hideed out
	logic negative;	// Positive sense variables are hideed out

	logic tail_index;

	logic assumptions;
	svector<logic> assertions;

	bool is_in_tail(int idx);
	void add_to_tail(int idx);
	void add_to_tail(svector<int> idx);

	pair<int, int> sense_count();
};

struct petri
{
	petri();
	~petri();

	variable_space *vars;
	rule_space *prs;
	flag_space *flags;
	program_execution_space env;
	svector<node> S;
	svector<node> T;
	svector<int> M0;
	int pbranch_count;
	int cbranch_count;
	svector<pair<int, int> > arcs;

	smap<int, list<svector<int> > > conflicts;
	smap<int, list<svector<int> > > indistinguishable;
	smap<int, list<svector<int> > > positive_conflicts;
	smap<int, list<svector<int> > > positive_indistinguishable;
	smap<int, list<svector<int> > > negative_conflicts;
	smap<int, list<svector<int> > > negative_indistinguishable;
	int max_indistinguishables;
	int max_positive_indistinguishables;
	int max_negative_indistinguishables;

	smap<int, svector<int> > isochronics;

	smap<int, pair<int, int> > conditional_places;
	svector<int> variable_usage;

	int new_transition(logic root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	svector<int> new_transitions(svector<logic> root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	int new_place(logic root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);

	int insert_transition(int from, logic root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	int insert_transition(svector<int> from, logic root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);

	void insert_sv_at(int a, logic root);
	void insert_sv_parallel(int from, logic root);

	svector<int> insert_transitions(int from, svector<logic> root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	svector<int> insert_transitions(svector<int> from, svector<logic> root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);

	int insert_dummy(int from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	int insert_dummy(svector<int> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);

	int insert_place(int from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);
	int insert_place(svector<int> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);

	svector<int> insert_places(svector<int> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner);

	void remove_place(int from);
	void remove_place(svector<int> from);
	void remove_trans(int from);
	void remove_trans(svector<int> from);

	void update();
	void check_assertions();
	void connect(svector<int> from, svector<int> to);
	void connect(svector<int> from, int to);
	void connect(int from, svector<int> to);
	void connect(int from, int to);
	pair<int, int> closest_input(svector<int> from, svector<int> to, path p, int i = 0);
	pair<int, int> closest_output(svector<int> from, svector<int> to, path p, int i = 0);

	bool dead(int from);
	bool is_place(int from);
	bool is_trans(int from);
	int  index(int from);
	int  place_id(int idx);
	int  trans_id(int idx);
	sstring node_name(int idx);
	logic base(svector<int> idx);
	bool connected(int from, int to);
	int psiblings(int p0, int p1);
	int csiblings(int p0, int p1);
	bool same_inputs(int p0, int p1);
	bool same_outputs(int p0, int p1);

	svector<int> duplicate_nodes(svector<int> from);
	int duplicate_node(int from);
	int merge_places(svector<int> from);
	int merge_places(int a, int b);

	int get_split_place(int merge_place, svector<bool> *covered);

	void add_conflict_pair(smap<int, list<svector<int> > > *c, int i, int j);

	void gen_mutables();
	void gen_conditional_places();
	bool are_sibling_guards(int i, int j);
	void gen_conflicts();
	void gen_bubbleless_conflicts();
	void gen_senses();
	void trim_branch_ids();
	void gen_tails();
	smap<pair<int, int>, pair<bool, bool> > gen_isochronics();
	logic apply_debug(int pc);

	logic get_effective_state_encoding(int place, int observer);

	/**
	 * \brief	Removes vacuous pbranches, unreachable places, and dangling, vacuous, and impossible transitions.
	 * \sa		merge_conflicts() and zip()
	 */
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

	/**
	 * \brief	If "from" is a transition this returns it's input places, and if "from" is a place this returns it's input transitions.
	 * \sa		output_nodes()
	 */
	svector<int> input_nodes(int from);
	svector<int> input_nodes(svector<int> from);

	/**
	 * \brief	If "from" is a transition this returns it's output places, and if "from" is a place this returns it's output transitions.
	 * \sa		input_nodes()
	 */
	svector<int> output_nodes(int from);
	svector<int> output_nodes(svector<int> from);

	svector<int> input_arcs(int from);
	svector<int> input_arcs(svector<int> from);
	svector<int> output_arcs(int from);
	svector<int> output_arcs(svector<int> from);

	svector<int> increment_arcs(svector<int> a);
	svector<int> decrement_arcs(svector<int> a);

	pair<int, int> get_input_sense_count(int idx);
	pair<int, int> get_input_sense_count(svector<int> idx);

	void get_paths(svector<int> from, svector<int> to, path_space *p);

	svector<int> arc_paths(int from, svector<int> to, svector<int> ex, path_space *t, path_space *p, int i = 0);
	void filter_path_space(path_space *p);
	void filter_path(int from, int to, path *p);
	void zero_paths(path_space *paths, int from);
	void zero_paths(path_space *paths, svector<int> from);
	void zero_ins(path_space *paths, int from);
	void zero_ins(path_space *paths, svector<int> from);
	void zero_outs(path_space *paths, int from);
	void zero_outs(path_space *paths, svector<int> from);
	svector<int> start_path(int from, svector<int> ex);
	svector<int> start_path(svector<int> from, svector<int> ex);
	svector<int> end_path(int to, svector<int> ex);
	svector<int> end_path(svector<int> to, svector<int> ex);

	node &operator[](int i);

	void print_dot(ostream *fout, sstring name);

	void print_mutables();
	void print_branch_ids(ostream *fout);

	void print_conflicts(ostream &fout, string name);
	void print_indistinguishables(ostream &fout, string name);
	void print_positive_conflicts(ostream &fout, string name);
	void print_positive_indistinguishables(ostream &fout, string name);
	void print_negative_conflicts(ostream &fout, string name);
	void print_negative_indistinguishables(ostream &fout, string name);
};

#endif
