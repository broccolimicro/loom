/*
 * composition.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: nbingham
 */

#include "composition.h"

composition::composition()
{
	parent = NULL;
	vars = NULL;
	net = NULL;
	chp = "";
	tab = "";
	_kind = "composition";
	verbosity = VERB_NONE;
}

composition::~composition()
{
	parent = NULL;
	vars = NULL;
	net = NULL;
	chp = "";
	tab = "";
	_kind = "composition";
	verbosity = VERB_NONE;

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
}

void composition::init(string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->vars = vars;

	expand_shortcuts();
	parse();
}

void composition::clear()
{
	chp = "";

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
}

vector<int> composition::variant()
{
	vector<int> result;
	vector<int> temp;

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		temp = (*i)->variant();
		result.insert(result.end(), temp.begin(), temp.end());
	}

	return unique(&result);
}

vector<int> composition::active_variant()
{
	vector<int> result;
	vector<int> temp;

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		temp = (*i)->active_variant();
		result.insert(result.end(), temp.begin(), temp.end());
	}

	return unique(&result);
}

vector<int> composition::passive_variant()
{
	vector<int> result;
	vector<int> temp;

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		temp = (*i)->passive_variant();
		result.insert(result.end(), temp.begin(), temp.end());
	}

	return unique(&result);
}
