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
	canonical(int s);
	canonical(minterm m);
	canonical(vector<minterm> m);
	canonical(string s, variable_space *vars);
	canonical(int var, uint32_t val);
	canonical(map<int, uint32_t> vals);
	~canonical();

	vector<minterm> terms;

	vector<minterm>::iterator begin();
	vector<minterm>::iterator end();

	// INTERNAL FUNCTIONS
	int size();
	int width();
	void assign(int i, minterm t);
	void remove(int i);

	void push_back(minterm m);
	void push_up(minterm m);
	void clear();

	void mccluskey();
	minterm mask();

	// EXTERNAL FUNCTIONS
	vector<int> vars();
	void vars(vector<int> *var_list);

	canonical refactor(vector<int> ids);

	canonical smooth(int var);
	canonical smooth(vector<int> vars);
	void extract(map<int, canonical> *result);
	map<int, canonical> extract();

	canonical pabs();
	canonical nabs();

	int satcount();
	map<int, uint32_t> anysat();
	vector<map<int, uint32_t> > allsat();

	canonical &operator=(canonical c);
	canonical &operator=(minterm t);
	canonical &operator=(uint32_t c);

	canonical &operator|=(canonical c);
	canonical &operator&=(canonical c);

	canonical &operator|=(uint32_t c);
	canonical &operator&=(uint32_t c);

	canonical operator()(int i, uint32_t v);
	canonical operator[](int i);

	canonical operator|(canonical c);
	canonical operator&(canonical c);
	canonical operator~();

	canonical operator|(uint32_t c);
	canonical operator&(uint32_t c);

	bool operator==(canonical c);
	bool operator!=(canonical c);

	bool operator==(uint32_t c);
	bool operator!=(uint32_t c);

	bool constant();

	canonical operator>>(canonical t);

	string print(variable_space *v, string prefix = "");
};



#endif
