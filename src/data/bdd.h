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
	bdd(map<int, uint32_t> vals);
	bdd(string exp, variable_space *v);

	vector<int> vars();
	void vars(vector<int> *var_list);

	bdd refactor(vector<int> ids);

	bdd smooth(int var);
	bdd smooth(vector<int> vars);
	void extract(map<int, bdd> *result);
	map<int, bdd> extract();

	bdd pabs();
	bdd nabs();

	int satcount();
	map<int, uint32_t> anysat();
	vector<map<int, uint32_t> > allsat();

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

	string print(variable_space *v);
};

#endif
