/*
 * bdd.h
 *
 *  Created on: May 10, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "variable_space.h"
#include "canonical.h"
#include "triple.h"

#ifndef bdd_h
#define bdd_h

/**
 * This is a package for Reduced Ordered Binary Decision Diagrams. It can store
 * many boolean expressions at once, optimizing for memory and access time. To
 * get started, look at the build(), apply(), print(), and expr() functions.
 */
struct bdd
{
	bdd();
	~bdd();

	vector<triple> T;
	unordered_map<triple, int> H;

	int var(int u);
	int low(int u);
	int high(int u);
	void allvars(int u, vector<int> *l);

	int mk(int i, int l, int h);
	int build(string e, variable_space *vars, int i = 0);
	int build(minterm t, int i = 0);
	int build(list<pair<int, int> > t);
	vector<int> build(canonical t, int i = 0);

	int apply(int (*op)(int, int), int u1, int u2, unordered_map<pair<int, int>, int> *G);
	int apply(int (*op)(int), int u1, unordered_map<int, int> *G);
	int apply_or(int u0, int u1);
	int apply_and(int u0, int u1);
	int apply_not(int u1);

	int invert(int u);
	int simplify(int d, int u);
	int transition(int u0, int u1);
	int get_pos(int u);
	int get_neg(int u);

	int restrict(int u, int j, int b);
	int smooth(int u, int j);
	int smooth(int u, vector<int> j);
	int extract(int u, int j);
	void extract(int u, map<int, int> *result);

	int count(int u);
	int satcount(int u);
	list<pair<int, int> > anysat(int u);
	list<list<pair<int, int> > > allsat(int u);

	void print(int u, string tab = "");
	string expr(int u, vector<string> vars);
	string trace(int u, vector<string> vars);
};

#endif
