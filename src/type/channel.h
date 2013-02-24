/*
 * channel.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#ifndef channel_h
#define channel_h

#include "../common.h"
#include "../data.h"
#include "keyword.h"
#include "record.h"
#include "operator.h"

/* This structure represents a structure or channel. A channel
 * contains a bunch of member variables that help you index
 * segments of bits within the multibit signal.
 */
/* Requirements:
 * All variables are boolean valued
 * There are no communication actions
 * All assignment statements have only constant expressions, only true or false
 */

struct channel : record
{
	channel();
	channel(string chp, map<string, keyword*> *types, int verbosity);
	~channel();

	operate send;
	operate recv;
	operate probe;

	channel &operator=(channel r);
	void parse(string chp, int verbosity);
};

ostream &operator<<(ostream &os, channel s);

#endif
