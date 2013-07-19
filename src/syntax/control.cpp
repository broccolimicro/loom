/*
 * control.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: nbingham
 */

#include "control.h"

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

vector<int> control::variant()
{
	vector<int> result;
	vector<int> temp;

	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		temp = i->first->variant();
		result.insert(result.end(), temp.begin(), temp.end());
		temp = i->second->variant();
		result.insert(result.end(), temp.begin(), temp.end());
	}

	return unique(&result);
}

vector<int> control::active_variant()
{
	vector<int> result;
	vector<int> temp;

	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		temp = i->first->active_variant();
		result.insert(result.end(), temp.begin(), temp.end());
		temp = i->second->active_variant();
		result.insert(result.end(), temp.begin(), temp.end());
	}

	return unique(&result);
}

vector<int> control::passive_variant()
{
	vector<int> result;
	vector<int> temp;

	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		temp = i->first->passive_variant();
		result.insert(result.end(), temp.begin(), temp.end());
		temp = i->second->passive_variant();
		result.insert(result.end(), temp.begin(), temp.end());
	}

	return unique(&result);
}
