/*
 * petri.h
 *
 *  Created on: May 9, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "matrix.h"
#include "bdd.h"
#include "pspace.h"

#ifndef petri_h
#define petri_h

struct instruction;
struct vspace;

struct node
{
	node();
	node(int index, bool active, vector<int> mutables, map<int, int> branch, instruction *owner);
	~node();

	instruction *owner;
	vector<int> mutables;	// The set of variables whose value we cannot know
	vector<int> tail;		// The set of inactive states preceding an active state
	map<int, int> branch;
	bool active;
	int index;
};

struct petri
{
	petri();
	~petri();

	bdd values;
	vspace *vars;
	vector<node> S;
	vector<node> T;
	matrix<int> Wp;		// <s, t> from t to s
	matrix<int> Wn;		// <s, t> from s to t
	vector<int> M0;		// Wp - Wn
	int branch_count;

	map<int, list<vector<int> > > conflicts;
	map<int, list<vector<int> > > indistinguishable;

	int new_transition(int root, bool active, map<int, int> branch, instruction *owner);
	vector<int> new_transitions(vector<int> root, bool active, map<int, int> branch, instruction *owner);
	int new_place(int root, vector<int> mutables, map<int, int> branch, instruction *owner);

	int insert_transition(int from, int root, map<int, int> branch, instruction *owner);
	int insert_transition(vector<int> from, int root, map<int, int> branch, instruction *owner);

	void insert_sv_before(int from, int root);
	void insert_sv_parallel(int from, int root);
	void insert_sv_after(int from, int root);

	vector<int> insert_transitions(int from, vector<int> root, map<int, int> branch, instruction *owner);
	vector<int> insert_transitions(vector<int> from, vector<int> root, map<int, int> branch, instruction *owner);

	int insert_dummy(int from, map<int, int> branch, instruction *owner);
	int insert_dummy(vector<int> from, map<int, int> branch, instruction *owner);

	int insert_place(int from, vector<int> mutables, map<int, int> branch, instruction *owner);
	int insert_place(vector<int> from, vector<int> mutables, map<int, int> branch, instruction *owner);

	vector<int> insert_places(vector<int> from, vector<int> mutables, map<int, int> branch, instruction *owner);

	void remove_place(int from);
	void remove_place(vector<int> from);

	void update(int p);
	void update_tail(int p);
	void connect(vector<int> from, vector<int> to);
	void connect(vector<int> from, int to);
	void connect(int from, vector<int> to);
	void connect(int from, int to);

	bool dead(int from);
	bool is_place(int from);
	bool is_trans(int from);
	int  index(int from);
	int  place_id(int idx);
	int  trans_id(int idx);
	int base(vector<int> idx);
	bool connected(int from, int to);

	vector<int> duplicate(vector<int> from);

	void gen_conflicts();

	vector<int> input_arcs(int from);
	vector<int> output_arcs(int from);

	void trim();
	void tails();

	path_space get_paths(int t1, int t2, path p);
	path_space get_paths(int t1, vector<int> t2, path p);
	path_space get_paths(int t1, int t2, vector<int> ex, path p);
	path_space get_paths(int t1, vector<int> t2, vector<int> ex, path p);
	path_space get_paths(vector<int> t1, int t2, path p);
	path_space get_paths(vector<int> t1, vector<int> t2, path p);
	path restrict_path(path p, vector<int> implicants);

	node &operator[](int i);

	void print_dot(ostream *fout, string name);
	void print_petrify(string name);
};

#endif
