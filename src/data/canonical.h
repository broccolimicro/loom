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
	canonical(string s, vector<string> vars);
	~canonical();

	vector<minterm> terms;

	vector<minterm>::iterator begin();
	vector<minterm>::iterator end();

	int size();
	int width();
	void assign(int i, minterm t);
	void remove(int i);

	void push_back(minterm m);
	void push_up(minterm m);
	void clear();

	bool always_0();
	bool always_1();

	void mccluskey();

	canonical restrict(int j, uint32_t b);
	canonical smooth(int j);
	canonical smooth(vector<int> j);
	void extract(map<int, uint32_t> *result);

	canonical get_pos();
	canonical get_neg();

	string print();
	string print(vector<string> vars);

	canonical &operator=(canonical c);

	canonical operator()(minterm m);
	canonical operator()(int i, uint32_t v);
	minterm operator[](int i);
	minterm operator()(int i);
};

canonical operator|(canonical c1, canonical c2);
canonical operator&(canonical c1, canonical c2);
canonical operator~(canonical c);

canonical operator+(canonical s, minterm t);

#endif
