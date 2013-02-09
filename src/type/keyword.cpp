/*
 * keyword.cpp
 *
 *  Created on: Jan 31, 2013
 *      Author: nbingham
 */

#include "keyword.h"

keyword::keyword()
{
	name = "";
	_kind = "keyword";
}
keyword::keyword(string n)
{
	name = n;
	_kind = "keyword";
}
keyword::~keyword()
{
	name = "";
	_kind = "keyword";
}

string keyword::kind()
{
	return _kind;
}

keyword &keyword::operator=(keyword k)
{
	name = k.name;
	return *this;
}

bool contains(string s, map<string, keyword*> m)
{
	map<string, keyword*>::iterator i;
	for (i = m.begin(); i != m.end(); i++)
		if (s.find(i->first) != s.npos)
			return true;

	return false;
}
