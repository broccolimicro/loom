/*
 * variable.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */


#ifndef variable_h
#define variable_h

#include "../common.h"
#include "value.h"

/* This structure describes a variable in the chp program. We need
 * to keep track of a variables name, its type (record, keyword, process),
 * and its bit width.
 */
struct variable
{
	variable();
	variable(string name, string type, value reset, uint16_t width, bool io);
	variable(string chp, bool io, string tab, int verbosity);
	~variable();

	string		chp;
	string		name;		// the name of the instantiated variable
	string		type;		// the name of the type of the instantiated variable
	uint16_t	width;		// the bit width of the instantiated variable
	bool		fixed;		// is the bit width of this variable fixed or variable?
	value		reset;
	int			uid;
	bool		prs;		// keep track of whether or not to generate production rules
	bool		io;			// Is this variable an input variable into a process?


	list<string> inputs;

	// For debugging purposes
	string tab;
	int verbosity;

	variable &operator=(variable v);
	void parse(string chp);
};

ostream &operator<<(ostream &os, map<string, variable> g);
ostream &operator<<(ostream &os, variable s);

#endif
