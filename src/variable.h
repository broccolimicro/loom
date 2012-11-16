/*
 * variable.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
 */

#include "common.h"
#include "state.h"

#ifndef variable_h
#define variable_h

/* This structure describes a variable in the chp program. We need
 * to keep track of a variables name, its type (record, keyword, process),
 * and its bit width.
 */
struct variable
{
	variable();
	variable(string n, string t, string s, uint16_t w);
	variable(string chp, string spr, string tab);
	~variable();

	string		name;		// the name of the instantiated variable
	string		type;		// the name of the type of the instantiated variable
	string		super;		// the name of the type of the record or channel that this variable belongs to (empty if not)
	uint16_t	width;		// the bit width of the instantiated variable
	bool		fixed;		// is the bit width of this variable fixed or variable?
	state		last;
	state		reset;

	variable &operator=(variable v);
	void parse(string chp, string tab);
};

ostream &operator<<(ostream &os, variable s);

#endif
