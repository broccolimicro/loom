/*
 * canonical.h
 *
 *  Created on: May 11, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "minterm.h"

#ifndef canonical_h
#define canonical_h

/**
 * This structure stores the canonical form of a binary boolean expression (a sum of minterms).
 */
struct canonical
{
	canonical();
	canonical(const canonical &c);
	canonical(int s);
	canonical(minterm m);
	canonical(svector<minterm> m);
	canonical(sstring s, variable_space *vars);
	canonical(int var, uint32_t val);
	canonical(smap<int, uint32_t> vals);
	~canonical();

	svector<minterm> terms;

	svector<minterm>::iterator begin();
	svector<minterm>::iterator end();

	// INTERNAL FUNCTIONS
	int size();
	void assign(int i, minterm t);
	void remove(int i);

	void push_back(minterm m);
	void push_up(minterm m);
	void clear();

	void mccluskey();
	minterm mask();

	// EXTERNAL FUNCTIONS
	svector<int> vars();
	void vars(svector<int> *var_list);

	canonical refactor(svector<int> ids);
	canonical refactor(svector<pair<int, int> > ids);

	canonical hide(int var);
	canonical hide(svector<int> vars);
	canonical restrict(canonical r);
	void extract(smap<int, canonical> *result);
	smap<int, canonical> extract();
	uint32_t val(int uid);

	canonical pabs();
	canonical nabs();

	int satcount();
	smap<int, uint32_t> anysat();
	svector<smap<int, uint32_t> > allsat();

	canonical &operator=(canonical c);
	canonical &operator=(minterm t);
	canonical &operator=(uint32_t c);

	canonical &operator|=(canonical c);
	canonical &operator&=(canonical c);
	canonical &operator^=(canonical c);

	canonical &operator|=(uint32_t c);
	canonical &operator&=(uint32_t c);

	canonical operator()(int i, uint32_t v);
	canonical operator[](int i);

	canonical operator|(canonical c);
	canonical operator&(canonical c);
	canonical operator~();
	canonical operator^(canonical c);
	canonical operator&&(canonical c);

	canonical operator|(uint32_t c);
	canonical operator&(uint32_t c);

	bool operator==(canonical c);
	bool operator!=(canonical c);

	bool operator==(minterm c);
	bool operator!=(minterm c);

	bool operator==(uint32_t c);
	bool operator!=(uint32_t c);

	bool constant();


	canonical operator>>(canonical t);

	sstring print(variable_space *v, sstring prefix = "");
	sstring print_assign(variable_space *v, sstring prefix = "");
	sstring print_with_quotes(variable_space *v, sstring prefix = "");
};

bool is_mutex(canonical *c0, canonical *c1);
bool is_mutex(canonical c0, canonical c1);
bool is_mutex(minterm *m0, canonical *c1);
bool is_mutex(canonical *c0, minterm *m1);
bool is_mutex(canonical *c0, canonical *c1, canonical *c2);
bool is_mutex(canonical *c0, canonical *c1, canonical *c2, canonical *c3);
bool mergible(canonical *c0, canonical *c1);
bool mergible(minterm c0, minterm c1);

#endif
