/*
 * control.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: nbingham
 */

#include "control.h"
#include "sequential.h"
#include "parallel.h"

control::control()
{
	_kind = "control";
	type = unknown;
}

control::~control()
{
	_kind = "control";
	type = unknown;
	chp = "";

	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i->first != NULL)
			delete i->first;
		if (i->second != NULL)
			delete i->second;
		i->first = NULL;
		i->second = NULL;
	}

	instrs.clear();
}

void control::clear()
{
	chp = "";

	list<pair<sequential*, guard*> >::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (j->first != NULL)
			delete j->first;
		if (j->second != NULL)
			delete j->second;
		j->first = NULL;
		j->second = NULL;
	}

	instrs.clear();
}

pair<string, instruction*> control::expand_guard(string chp)
{
	return expand_expression(chp, "");
}
