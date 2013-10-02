/*
 * bdd_package.h
 *
 *  Created on: Jul 31, 2013
 *      Author: nbingham
 */

#include "minterm.h"
#include "canonical.h"
#include "triple.h"

#ifndef bdd_package_h
#define bdd_package_h

/**
 * This is a package for Reduced Ordered Binary Decision Diagrams. It can store
 * many boolean expressions at once, optimizing for memory and access time. To
 * get started, look at the build(), apply(), pruint32_t(), and expr() functions.
 */
struct bdd_package
{
	friend struct bdd;

	bdd_package();
	~bdd_package();

private:
	svector<triple> T;
	unordered_map<triple, uint32_t> H;

	void vars(uint32_t u, svector<int> *var_list);

	uint32_t mk(int i, uint32_t l, uint32_t h);
	uint32_t build(smap<int, uint32_t> t);
	uint32_t build(sstring exp, variable_space *V, int i = 0);
	uint32_t build(minterm t);
	uint32_t build(canonical t, int i = 0);

	uint32_t apply(uint32_t (*op)(uint32_t, uint32_t), uint32_t u1, uint32_t u2, unordered_map<pair<uint32_t, uint32_t>, uint32_t> *G);
	uint32_t apply(uint32_t (*op)(uint32_t), uint32_t u1, unordered_map<uint32_t, uint32_t> *G);

	uint32_t invert(uint32_t u);
	uint32_t simplify(uint32_t d, uint32_t u);
	uint32_t restrict(uint32_t u, int j, uint32_t b);

	int count(uint32_t u);
	smap<int, uint32_t> anysat(uint32_t u);
	svector<smap<int, uint32_t> > allsat(uint32_t u);

	void print(uint32_t u, sstring tab = "");
};

#endif

