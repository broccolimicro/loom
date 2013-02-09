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
#include "../variable.h"
#include "keyword.h"

/* This structure represents a structure or record. A record
 * contains a bunch of member variables that help you index
 * segments of bits within the multibit signal.
 */
struct record : keyword
{
	record();
	record(string raw, map<string, keyword*> types, int verbosity);
	~record();

	string					chp;
	map<string, variable>	globals;	// the list of member variables that make up this record
	map<string, variable>	labels;

	record &operator=(record r);
	void parse(string raw, map<string, keyword*> types, int verbosity);
};

ostream &operator<<(ostream &os, record s);

#endif
