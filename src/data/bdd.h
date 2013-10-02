/*
 * bdd.h
 *
 *  Created on: May 10, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "minterm.h"
#include "canonical.h"

#ifndef bdd_h
#define bdd_h

struct variable_space;

struct bdd
{
	bdd();
	bdd(minterm t);
	bdd(canonical c);
	~bdd();

	uint32_t idx;

	int var();
	bdd low();
	bdd high();

	bdd(uint32_t val);
	bdd(int var, uint32_t val);
	bdd(smap<int, uint32_t> vals);
	bdd(sstring exp, variable_space *v);

	svector<int> vars();
	void vars(svector<int> *var_list);

	bdd refactor(svector<int> ids);

	bdd hide(int var);
	bdd hide(svector<int> vars);
	void extract(smap<int, bdd> *result);
	smap<int, bdd> extract();

	bdd pabs();
	bdd nabs();

	int satcount();
	smap<int, uint32_t> anysat();
	svector<smap<int, uint32_t> > allsat();

	bdd &operator=(bdd b);
	bdd &operator=(uint32_t b);

	bdd &operator|=(bdd b);
	bdd &operator&=(bdd b);

	bdd &operator|=(uint32_t b);
	bdd &operator&=(uint32_t b);

	bdd operator()(int var, uint32_t val);
	bdd operator[](int var);

	bdd operator|(bdd b);
	bdd operator&(bdd b);
	bdd operator~();

	bdd operator|(uint32_t b);
	bdd operator&(uint32_t b);

	bool operator==(bdd b);
	bool operator!=(bdd b);

	bool operator==(uint32_t b);
	bool operator!=(uint32_t b);

	bool constant();

	bdd operator>>(bdd b);

	sstring print(variable_space *v);
};

#endif
