/*
 * variable.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"
#include "value.h"

#ifndef variable_h
#define variable_h

/* This structure describes a variable in the chp program. We need
 * to keep track of a variables name, its type (record, keyword, process),
 * and its bit width.
 */
struct variable
{
	variable();
	variable(string n, string t, value r, uint16_t w);
	variable(string raw, string tab, int verbosity);
	~variable();

	string		chp;
	string		name;		// the name of the instantiated variable
	string		type;		// the name of the type of the instantiated variable
	uint16_t	width;		// the bit width of the instantiated variable
	bool		fixed;		// is the bit width of this variable fixed or variable?
	value		reset;
	int			uid;		// unique identifier
	bool		prs;		// keep track of whether or not to generate production rules

	variable &operator=(variable v);
	void parse(string raw, string tab, int verbosity);
};

ostream &operator<<(ostream &os, variable s);

#endif
