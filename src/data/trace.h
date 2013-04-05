/*
 * trace.h
 *
 * Trace contains the known values of given variable for every 'instant' in the program's execution.
 * One way to think about a trace is as the columns of a state space. Trace is most often used in the
 * context of a trace space, which contains a trace for every variable in the program. Trace is a good tool
 * for capturing how a single variable behaves over the whole execution of the program.
 */

#include "../common.h"
#include "value.h"

#ifndef trace_h
#define trace_h

struct trace
{
	trace();
	trace(value v, int s);
	trace(vector<value> v);
	trace(string s);
	~trace();

	// Instruction indexed using uid
	vector<value>	values;

	void clear();
	int size();
	void assign(int i, value v, value r = value("X"));
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

	trace operator()(int i);
	value &operator[](int i);
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
trace operator||(trace s1, trace s2);
trace operator&&(trace s1, trace s2);

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
trace conflicts(trace left, trace right);
int conflict_count(trace proposed, trace desired);

#endif
