/*
 * value.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"

#ifndef value_h
#define value_h

/* This structure contains a single value for a single variable.
 * It also flags whether or not this value should generate a
 * production rule.
 */
struct value
{
	value();
	value(string d);
	~value();

	string data;

	string::iterator begin();
	string::iterator end();

	value &operator=(value s);
	value &operator=(string s);

	value &operator+=(value s);
	value &operator-=(value s);
	value &operator*=(value s);
	value &operator/=(value s);

	value &operator&=(value s);
	value &operator|=(value s);

	value &operator<<=(value s);
	value &operator>>=(value s);

	value &operator<<=(int n);
	value &operator>>=(int n);

	value operator[](size_t i);
};

bool subset(value s1, value s2);
bool up_subset(value s1, value s2);
bool down_subset(value s1, value s2);

ostream &operator<<(ostream &os, value s);

value operator+(value s1, value s2);
value operator-(value s1, value s2);
value operator*(value s1, value s2);
value operator/(value s1, value s2);

value operator-(value s);

value operator&(value s1, value s2);
value operator|(value s1, value s2);

value operator~(value s);

value operator<<(value s, int n);
value operator>>(value s, int n);

value operator<<(value s1, value s2);
value operator>>(value s1, value s2);

value operator==(value s1, value s2);
value operator!=(value s1, value s2);
value operator<=(value s1, value s2);
value operator>=(value s1, value s2);
value operator<(value s1, value s2);
value operator>(value s1, value s2);

value operator||(value s1, value s2);
value operator&&(value s1, value s2);
value operator!(value s);

value diff(value a, value b);

#endif
