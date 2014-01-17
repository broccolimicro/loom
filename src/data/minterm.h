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

#define v_ 0x00000000
#define v0 0x55555555
#define v1 0xAAAAAAAA
#define vX 0xFFFFFFFF

uint32_t itom(int v);
int mtoi(uint32_t v);
uint32_t vidx(int v);
uint32_t vmsk(int v);

struct minterm
{
	minterm();
	minterm(const minterm &m);
	minterm(sstring str);
	~minterm();

	svector<uint32_t> values;
	uint32_t default_value;
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

	svector<minterm> expand(svector<int> uids);

	// EXTERNAL FUNCTIONS
	minterm(uint32_t val);
	minterm(int var, uint32_t val);
	minterm(smap<int, uint32_t> vals);
	minterm(sstring exp, variable_space *vars);

	svector<int> vars();
	void vars(svector<int> *var_list);

	minterm refactor(svector<int> ids);
	minterm refactor(svector<pair<int, int> > ids);

	minterm hide(int var);
	minterm hide(svector<int> vars);
	void extract(smap<int, minterm> *result);
	smap<int, minterm> extract();

	minterm pabs();
	minterm nabs();

	int satcount();
	smap<int, uint32_t> anysat();
	svector<smap<int, uint32_t> > allsat();

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

	sstring print(variable_space *vars, sstring prefix = "");
	sstring print_assign(variable_space *vars, sstring prefix = "");
	sstring print_with_quotes(variable_space *vars, sstring prefix = "");
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
