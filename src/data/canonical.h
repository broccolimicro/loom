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

struct canonical
{
	canonical();
	canonical(int s);
	canonical(minterm m);
	canonical(vector<minterm> m);
	~canonical();

	vector<minterm> terms;

	int size();
	int width();
	void assign(int i, minterm t);
	void remove(int i);

	void mccluskey();

	void push_back(minterm m);
	void push_up(minterm m);
	void clear();

	bool always_0();
	bool always_1();

	string print();
	string print(vector<string> vars);

	vector<minterm>::iterator begin();
	vector<minterm>::iterator end();

	canonical &operator=(canonical c);

	canonical operator()(minterm m);
	canonical operator()(int i, uint32_t v);
	minterm operator[](int i);
	minterm operator()(int i);
};

#endif
