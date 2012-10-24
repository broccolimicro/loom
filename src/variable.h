/*
 * variable.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
 */

#include "common.h"

#ifndef variable_h
#define variable_h

/* This structure describes a variable in the chp program. We need
 * to keep track of a variables name, its type (record, keyword, process),
 * and its bit width.
 */
struct variable
{
	variable();
	variable(string n, string t, uint16_t w);
	variable(string chp);
	~variable();

	string		name;		// the name of the instantiated variable
	string		type;		// the name of the type of the instantiated variable
	uint16_t	width;		// the bit width of the instantiated variable
	string		last;

	variable &operator=(variable v);
	void parse(string chp);
};

ostream &operator<<(ostream &os, variable s);

#endif
