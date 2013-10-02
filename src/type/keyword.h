/*
 * keyword.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../flag_space.h"

#ifndef keyword_h
#define keyword_h

/* This structure represents the basic data types. The only
 * basic data type currently defined is 'int' which represents
 * an n-bit integer. The only thing that we need to keep track
 * of these basic data types is its name.
 */
struct keyword
{
protected:
	sstring _kind;

public:

	keyword();
	keyword(sstring n);
	~keyword();

	sstring name;

	sstring kind();

	keyword &operator=(keyword k);
};

typedef smap<sstring, keyword*> type_space;

bool contains(sstring s, type_space m);

#endif
