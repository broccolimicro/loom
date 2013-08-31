/*
 * petri.h
 *
 *  Created on: May 9, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "matrix.h"
#include "canonical.h"
#include "bdd.h"
#include "path_space.h"
#include "../flag_space.h"

#ifndef petri_h
#define petri_h

struct instruction;
struct variable_space;
struct rule_space;

struct node
{
	node();
	node(logic index, bool active, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);
	~node();

	instruction *owner;
	map<int, logic> mutables;	// The set of variables whose value we cannot know
	vector<int> tail;		// The set of inactive states preceding an active state
	map<int, int> pbranch;
	map<int, int> cbranch;
	bool active;

	logic index;
	logic positive;	// Negative sense variables are smoothed out
	logic negative;	// Positive sense variables are smoothed out

	logic assumptions;
	vector<logic> assertions;
};

struct petri
{
	petri();
	~petri();

	variable_space *vars;
	rule_space *prs;
	flag_space *flags;
	vector<node> S;
	vector<node> T;
	matrix<int> Wp;		// <s, t> from t to s
	matrix<int> Wn;		// <s, t> from s to t
	vector<int> M0;		// Wp - Wn
	int pbranch_count;
	int cbranch_count;
	vector<pair<int, int> > arcs;

	map<int, list<vector<int> > > conflicts;
	map<int, list<vector<int> > > indistinguishable;
	map<int, list<vector<int> > > positive_conflicts;
	map<int, list<vector<int> > > positive_indistinguishable;
	map<int, list<vector<int> > > negative_conflicts;
	map<int, list<vector<int> > > negative_indistinguishable;

	map<int, pair<int, int> > conditional_places;
	vector<int> variable_usage;

	int new_transition(logic root, bool active, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);
	vector<int> new_transitions(vector<logic> root, bool active, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);
	int new_place(logic root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);

	int insert_transition(int from, logic root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);
	int insert_transition(vector<int> from, logic root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);

	void insert_sv_at(int a, logic root);
	void insert_sv_before(int from, logic root);
	void insert_sv_parallel(int from, logic root);
	void insert_sv_after(int from, logic root);

	vector<int> insert_transitions(int from, vector<logic> root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);
	vector<int> insert_transitions(vector<int> from, vector<logic> root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);

	int insert_dummy(int from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);
	int insert_dummy(vector<int> from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);

	int insert_place(int from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);
	int insert_place(vector<int> from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);

	vector<int> insert_places(vector<int> from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner);

	void remove_place(int from);
	void remove_place(vector<int> from);

	void propogate_marking_forward(int from, vector<bool> *covered);
	void propogate_marking_backward(int from, vector<bool> *covered);

	bool updateplace(int p, int i = 0);
	int update(int p, vector<bool> *covered, int i = 0, bool immune = false);
	void update();
	void check_assertions();
	void update_tail(int p);
	void connect(vector<int> from, vector<int> to);
	void connect(vector<int> from, int to);
	void connect(int from, vector<int> to);
	void connect(int from, int to);
	pair<int, int> closest_input(vector<int> from, vector<int> to, path p, int i = 0);
	pair<int, int> closest_output(vector<int> from, vector<int> to, path p, int i = 0);

	bool dead(int from);
	bool is_place(int from);
	bool is_trans(int from);
	int  index(int from);
	int  place_id(int idx);
	int  trans_id(int idx);
	logic base(vector<int> idx);
	bool connected(int from, int to);
	int psiblings(int p0, int p1);
	int csiblings(int p0, int p1);
	bool same_inputs(int p0, int p1);
	bool same_outputs(int p0, int p1);

	vector<int> duplicate_nodes(vector<int> from);
	int duplicate_node(int from);
	int merge_places(vector<int> from);
	int merge_places(int a, int b);

	int get_split_place(int merge_place, vector<bool> *covered);

	void add_conflict_pair(map<int, list<vector<int> > > *c, int i, int j);

	void gen_mutables();
	void gen_conditional_places();
	void gen_conflicts();
	void gen_bubbleless_conflicts();
	void gen_senses();
	void trim_branch_ids();
	void gen_tails();
	void gen_arcs();

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
	vector<int> input_nodes(int from);

	/**
	 * \brief	If "from" is a transition this returns it's output places, and if "from" is a place this returns it's output transitions.
	 * \sa		input_nodes()
	 */
	vector<int> output_nodes(int from);

	vector<int> input_arcs(int from);
	vector<int> output_arcs(int from);

	void get_paths(vector<int> from, vector<int> to, path_space *p);

	int arc_paths(int from, vector<int> to, vector<int> ex, path_space *t, path_space *p, int i = 0);
	void filter_path_space(path_space *p);
	void filter_path(int from, int to, path *p);
	void zero_paths(path_space *paths, int from);
	void zero_paths(path_space *paths, vector<int> from);
	void zero_ins(path_space *paths, int from);
	void zero_ins(path_space *paths, vector<int> from);
	void zero_outs(path_space *paths, int from);
	void zero_outs(path_space *paths, vector<int> from);
	vector<int> start_path(int from, vector<int> ex);
	vector<int> start_path(vector<int> from, vector<int> ex);
	vector<int> end_path(int to, vector<int> ex);
	vector<int> end_path(vector<int> to, vector<int> ex);

	node &operator[](int i);

	void print_dot(ostream *fout, string name);

	void print_mutables();
	void print_branch_ids(ostream *fout);
};

#endif
