/*
 * minterm.h
 *
 *  Created on: May 11, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "vspace.h"

#ifndef minterm_h
#define minterm_h

#define v_ 0x00000000
#define v0 0x55555555
#define v1 0xAAAAAAAA
#define vX 0xFFFFFFFF

struct minterm
{
	minterm();
	minterm(string str);
	minterm(string str, vspace *vars);
	minterm(string str, vector<string> vars);
	minterm(int s, uint32_t v);
	minterm(int s, int i, uint32_t v);
	~minterm();

	vector<uint32_t> values;
	int size;

	void clear();
	vector<uint32_t>::iterator begin();
	vector<uint32_t>::iterator end();
	void inelastic_set(int uid, uint32_t v);
	void elastic_set(int uid, uint32_t v, uint32_t r = 0);

	void sv_union(int uid, uint32_t v);
	void sv_intersect(int uid, uint32_t v);
	void sv_invert(int uid);
	void sv_or(int uid, uint32_t v);
	void sv_and(int uid, uint32_t v);
	void sv_not(int uid);

	bool always_0();
	bool always_1();

	vector<int> variable_list();

	uint32_t operator[](int i);
	minterm operator()(int i, uint32_t v);

	minterm &operator=(minterm s);

	minterm &operator&=(minterm s);
	minterm &operator|=(minterm s);

	void push_back(uint32_t v);

	string print_expr();
	string print_expr(vector<string> vars);
	string print_trace();
};

minterm nullv(int s);
minterm fullv(int s);

bool all_(minterm s);
bool all0(minterm s);
bool all1(minterm s);
bool allX(minterm s);

bool has_(minterm s);

bool subset(minterm s1, minterm s2);
bool conflict(minterm s1, minterm s2);
bool up_conflict(minterm s1, minterm s2);
bool down_conflict(minterm s1, minterm s2);

minterm operator~(minterm s);
minterm operator&(minterm s1, minterm s2);
minterm operator|(minterm s1, minterm s2);

minterm operator!(minterm s);
minterm operator||(minterm s1, minterm s2);
minterm operator&&(minterm s1, minterm s2);

bool operator==(minterm s1, minterm s2);
bool operator!=(minterm s1, minterm s2);

int diff_count(minterm s1, minterm s2);

minterm project(minterm m0, minterm m1);

#endif
