/*
 * minterm.h
 *
 *  Created on: May 11, 2013
 *      Author: nbingham
 */

#include "../common.h"

#ifndef minterm_h
#define minterm_h

struct canonical;
struct variable_space;

struct minterm
{
	minterm();
	minterm(string str);
	~minterm();

	vector<uint32_t> values;
	int size;


	// INTERNAL FUNCTIONS
	uint32_t get(int uid);
	uint32_t val(int uid);
	void set(int uid, uint32_t v);
	void resize(int s, uint32_t r = 0xFFFFFFFF);
	void clear();

	void sv_union(int uid, uint32_t v);
	void sv_intersect(int uid, uint32_t v);
	void sv_invert(int uid);
	void sv_or(int uid, uint32_t v);
	void sv_and(int uid, uint32_t v);
	void sv_not(int uid);

	bool subset(minterm s);
	bool conflict(minterm s);

	int diff_count(minterm s);
	pair<int, int> xdiff_count(minterm s);

	minterm xoutnulls();

	minterm mask();
	minterm inverse();

	void push_back(uint32_t v);

	vector<minterm> expand(vector<int> uids);

	// EXTERNAL FUNCTIONS
	minterm(uint32_t val);
	minterm(int var, uint32_t val);
	minterm(map<int, uint32_t> vals);
	minterm(string exp, variable_space *vars);

	vector<int> vars();
	void vars(vector<int> *var_list);

	minterm refactor(vector<int> ids);

	minterm hide(int var);
	minterm hide(vector<int> vars);
	void extract(map<int, minterm> *result);
	map<int, minterm> extract();

	minterm pabs();
	minterm nabs();

	int satcount();
	map<int, uint32_t> anysat();
	vector<map<int, uint32_t> > allsat();

	minterm &operator=(minterm s);
	minterm &operator=(uint32_t s);

	minterm &operator&=(minterm s);
	minterm &operator|=(minterm s);

	minterm &operator&=(uint32_t s);
	minterm &operator|=(uint32_t s);

	minterm operator()(int i, uint32_t v);
	minterm operator[](int i);

	minterm operator&(minterm s);
	minterm operator|(minterm s);
	canonical operator~();

	minterm operator|(uint32_t s);
	minterm operator&(uint32_t s);

	bool constant();

	minterm operator>>(minterm t);

	string print(variable_space *vars, string prefix = "");
	string print_assign(variable_space *vars, string prefix = "");
	string print_with_quotes(variable_space *vars, string prefix = "");
};

bool operator==(minterm s1, minterm s2);
bool operator!=(minterm s1, minterm s2);

bool operator==(minterm s1, uint32_t s2);
bool operator!=(minterm s1, uint32_t s2);

bool operator<(minterm s1, minterm s2);
bool operator>(minterm s1, minterm s2);
bool operator<=(minterm s1, minterm s2);
bool operator>=(minterm s1, minterm s2);

#endif
