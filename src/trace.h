/*
 * trace.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"
#include "value.h"

#ifndef trace_h
#define trace_h

/* This structure represents a whole value
 * trace for a single variable. It contains
 * the name of the variable it represents
 * and a list of value structures.
 */
struct trace
{
	trace();
	trace(string v, vector<value> s);
	trace(string v, string s);
	trace(string s);
	~trace();

	// Instruction indexed using uid
	vector<value>	values;

	void clear();
	vector<value>::iterator begin();
	vector<value>::iterator end();

	void push_back(value v);

	trace &operator=(trace s);

	trace &operator+=(trace s);
	trace &operator-=(trace s);
	trace &operator*=(trace s);
	trace &operator/=(trace s);

	trace &operator&=(trace s);
	trace &operator|=(trace s);

	trace &operator<<=(trace s);
	trace &operator>>=(trace s);

	trace &operator+=(value s);
	trace &operator-=(value s);
	trace &operator*=(value s);
	trace &operator/=(value s);

	trace &operator&=(value s);
	trace &operator|=(value s);

	trace &operator<<=(value s);
	trace &operator>>=(value s);

	trace &operator<<=(int n);
	trace &operator>>=(int n);

	trace operator[](int i);
};

ostream &operator<<(ostream &os, trace s);

trace operator+(trace s1, trace s2);
trace operator-(trace s1, trace s2);
trace operator*(trace s1, trace s2);
trace operator/(trace s1, trace s2);

trace operator+(trace s1, value s2);
trace operator-(trace s1, value s2);
trace operator*(trace s1, value s2);
trace operator/(trace s1, value s2);

trace operator+(value s1, trace s2);
trace operator-(value s1, trace s2);
trace operator*(value s1, trace s2);
trace operator/(value s1, trace s2);

trace operator-(trace s);

trace operator&(trace s1, trace s2);
trace operator|(trace s1, trace s2);

trace operator&(trace s1, value s2);
trace operator|(trace s1, value s2);

trace operator&(value s1, trace s2);
trace operator|(value s1, trace s2);

trace operator~(trace s);

trace operator<<(trace s1, trace s2);
trace operator>>(trace s1, trace s2);

trace operator<<(trace s1, value s2);
trace operator>>(trace s1, value s2);

trace operator<<(value s1, trace s2);
trace operator>>(value s1, trace s2);

trace operator<<(trace s, int n);
trace operator>>(trace s, int n);

/*trace operator<(trace s1, int n);
trace operator>(trace s1, int n);*/

trace operator==(trace s1, trace s2);
trace operator!=(trace s1, trace s2);
trace operator<=(trace s1, trace s2);
trace operator>=(trace s1, trace s2);
trace operator<(trace s1, trace s2);
trace operator>(trace s1, trace s2);

trace operator==(trace s1, value s2);
trace operator!=(trace s1, value s2);
trace operator<=(trace s1, value s2);
trace operator>=(trace s1, value s2);
trace operator<(trace s1, value s2);
trace operator>(trace s1, value s2);

trace operator==(value s1, trace s2);
trace operator!=(value s1, trace s2);
trace operator<=(value s1, trace s2);
trace operator>=(value s1, trace s2);
trace operator<(value s1, trace s2);
trace operator>(value s1, trace s2);

int count(trace s);
int strict_count(trace s);
int delta_count(trace s);
/*trace up(trace s);
trace up(trace s, int idx);
trace down(trace s);
trace down(trace s, int idx);*/
string conflicts(trace left, trace right);

#endif
