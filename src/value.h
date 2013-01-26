/*
 * value.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"

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
	value(string d, bool p);
	~value();

	string data;

	/* If this is 1, then we need to generate
	 * production rules for this value. Otherwise,
	 * we don't.
	 */
	bool prs;	// TODO Move to variable

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

value operator!(value s);
value operator||(value s1, value s2);
value operator&&(value s1, value s2);

#endif
