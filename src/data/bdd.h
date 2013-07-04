/*
 * bdd.h
 *
 *  Created on: May 10, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "canonical.h"

using namespace std;

#ifndef bdd_h
#define bdd_h

struct triple
{
	triple();
	triple(int i, int l, int h);
	~triple();

	int i;
	int l;
	int h;

	triple &operator=(triple t);
};

bool operator==(triple t1, triple t2);

namespace std
{
	template<>
	struct hash<triple>
	{
		inline size_t operator()(const triple &v) const
		{
			return hash_pair(v.i, hash_pair(v.l, v.h)) % 15485863;
		}
	};
}

struct bdd
{
	bdd();
	~bdd();

	vector<triple> T;
	unordered_map<triple, int> H;

	int var(int u);
	int low(int u);
	int high(int u);

	int mk(int i, int l, int h);
	int build(minterm t, int i = 0);
	vector<int> build(canonical t, int i = 0);
	int apply(int (*op)(int, int), int u1, int u2, unordered_map<pair<int, int>, int> *G);
	int apply(int (*op)(int), int u1, unordered_map<int, int> *G);
	int restrict(int u, int j, int b);
	int smooth(int u, int j);
	int smooth(int u, vector<int> j);
	void variable_list(int u, vector<int> *l);
	int transition(int u0, int u1);
	int count(int u);
	int satcount(int u);
	list<pair<int, int> > anysat(int u);
	list<list<pair<int, int> > > allsat(int u);
	int simplify(int d, int u);
	void print(int u, string tab = "");
	string expr(int u, vector<string> vars);
	string trace(int u, vector<string> vars);

	int apply_or(int u0, int u1);
	int apply_and(int u0, int u1);
	int apply_not(int u1);
	int apply_union(int u0, int u1);
	int apply_intersect(int u0, int u1);
	int apply_inverse(int u1);
};

#endif
