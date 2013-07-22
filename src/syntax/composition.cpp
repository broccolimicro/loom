/*
 * composition.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: nbingham
 */

#include "composition.h"
#include "parallel.h"
#include "sequential.h"
#include "assignment.h"
#include "condition.h"
#include "loop.h"

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

instruction *composition::expand_assignment(string chp)
{
	parallel *p = new parallel(this, "", vars, tab, verbosity);
	assignment *a = new assignment(p, chp, vars, tab, verbosity);
	pair<string, instruction*> result;
	list<pair<string, string> >::iterator i;
	list<pair<string, string> > remove;
	variable *v;

	for (i = a->expr.begin(); i != a->expr.end(); i++)
	{
		v = vars->find(i->second);
		if (i->second.find_first_of("&|~^=<>/+-*?#()") != i->second.npos)
		{
			result = expand_expression(i->second, i->first);
			i->second = result.first;
			result.second->parent = p;
			p->push(result.second);
			remove.push_back(*i);
		}
		else if (v != NULL)
		{
			p->push(new condition(p, "[" + i->second + "->" + i->first + "+[]~" + i->second + "->" + i->first + "-]", vars, tab, verbosity));
			remove.push_back(*i);
		}
	}

	for (i = remove.begin(); i != remove.end(); i++)
		a->expr.remove(*i);

	if (p->instrs.size() == 0)
	{
		delete p;
		a->parent = this;
		return a;
	}

	if (a->expr.size() > 0)
		p->push(a);
	else
		delete a;

	return p;
}

instruction *composition::expand_condition(string chp)
{
	sequential *s = new sequential(this, "", vars, tab, verbosity);
	parallel *p = new parallel(s, "", vars, tab, verbosity);
	condition *a = new condition(s, chp, vars, tab, verbosity);
	pair<string, instruction*> result;
	list<pair<sequential*, guard*> >::iterator i;

	for (i = a->instrs.begin(); i != a->instrs.end(); i++)
	{
		cout << "Expanding Guard: " << i->second->chp << endl;
		if (i->second->chp.find_first_of("&|~^=<>/+-*?#()") != string::npos)
		{
			result = a->expand_guard(i->second->chp);
			i->second->chp = result.first;
			cout << "Result: " << result.first << endl;
			if (result.second != NULL)
			{
				result.second->parent = p;
				p->push(result.second);
			}
		}
	}

	if (p->instrs.size() == 0)
	{
		delete p;
		delete s;
		a->parent = this;
		return a;
	}

	s->push(p);
	s->push(a);

	return s;
}

instruction *composition::expand_loop(string chp)
{
	sequential *s = new sequential(this, "", vars, tab, verbosity);
	parallel *p = new parallel(s, "", vars, tab, verbosity);
	loop *a = new loop(s, chp, vars, tab, verbosity);
	pair<string, instruction*> result;
	list<pair<sequential*, guard*> >::iterator i;

	for (i = a->instrs.begin(); i != a->instrs.end(); i++)
		if (i->second->chp.find_first_of("&|~^=<>/+-*?#()") != string::npos)
		{
			result = a->expand_guard(i->second->chp);
			i->second->chp = result.first;
			result.second->parent = p;
			p->push(result.second);
		}

	if (p->instrs.size() == 0)
	{
		delete p;
		delete s;
		a->parent = this;
		return a;
	}

	s->push(p);
	s->push(a);

	return s;
}
