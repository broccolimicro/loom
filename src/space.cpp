/*
 * space.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "space.h"
#include "common.h"

space::space()
{
	var = "";
}

space::space(string v, list<string> s)
{
	var = v;
	states = s;
}

space::~space()
{
	var = "";
}

space &space::operator=(space s)
{
	var = s.var;
	states = s.states;
	return *this;
}

ostream &operator<<(ostream &os, space s)
{
    os << s.var << " : ";
    list<string>::iterator i;
    for (i = s.states.begin(); i != s.states.end(); i++)
    	os << *i << " ";

    return os;
}

space operator==(space s1, string s2)
{
	space result;
	result.var =  s1.var + " == " + s2;

	list<string>::iterator i;
	string::iterator j, k;
	string state = "i";

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		state = "i";
		for (j = i->begin()+1, k = s2.begin(); j != i->end() && k != s2.end(); j++, k++)
		{
			if (*j == 'X' || *k == 'X')
				state += "X";
			else if (*j == *k)
				state += "1";
			else
				state += "0";
		}

		result.states.push_back(state);
	}

	return result;
}

space operator==(string s1, space s2)
{
	space result;
	result.var =  s1 + " == " + s2.var;

	list<string>::iterator i;
	string::iterator j, k;
	string state = "i";

	for (i = s2.states.begin(); i != s2.states.end(); i++)
	{
		state = "i";
		for (j = i->begin()+1, k = s1.begin(); j != i->end() && k != s1.end(); j++, k++)
		{
			if (*j == 'X' || *k == 'X')
				state += "X";
			else if (*j == *k)
				state += "1";
			else
				state += "0";
		}

		result.states.push_back(state);
	}

	return result;
}
