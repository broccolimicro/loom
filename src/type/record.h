/*
 * record.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#ifndef record_h
#define record_h

#include "../common.h"
#include "../flag_space.h"
#include "../data.h"
#include "keyword.h"

/* This structure represents a structure or record. A record
 * contains a bunch of member variables that help you index
 * segments of bits within the multibit signal.
 */
struct record : keyword
{
	record();
	record(string raw, type_space *types, flag_space *flags);
	~record();

	string					chp;
	variable_space			vars;
	flag_space				*flags;

	record &operator=(record r);
	void parse(string raw);
};

ostream &operator<<(ostream &os, record s);

#endif
