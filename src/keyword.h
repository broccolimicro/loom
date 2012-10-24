/*
 * keyword.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
 */

#include "common.h"

#ifndef keyword_h
#define keyword_h

/* This structure represents the basic data types. The only
 * basic data type currently defined is 'int' which represents
 * an n-bit integer. The only thing that we need to keep track
 * of these basic data types is its name.
 */
struct keyword
{
	keyword()
	{
		name = "";
	}
	keyword(string n)
	{
		name = n;
	}
	~keyword()
	{
		name = "";
	}

	string name;

	keyword &operator=(keyword k)
	{
		name = k.name;
		return *this;
	}
};

#endif
