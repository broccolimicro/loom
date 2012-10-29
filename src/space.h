/*
 * space.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham, Nicholas Kramer
 */

#include "common.h"

#ifndef space_h
#define space_h

struct space
{
	space();
	space(string v, list<string> s);
	~space();

	string			var;
	list<string>	states;

	space &operator=(space s);
};

ostream &operator<<(ostream &os, space s);
space operator==(space s1, string s2);
space operator==(string s1, space s2);

#endif
