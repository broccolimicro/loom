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
	_kind = "composition";
	flags = NULL;
}

composition::~composition()
{
	parent = NULL;
	vars = NULL;
	net = NULL;
	chp = "";
	_kind = "composition";
	flags = NULL;

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
}

void composition::init(string chp, variable_space *vars, flag_space *flags)
{
	clear();

	this->chp = chp;
	this->flags = flags;
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

instruction *composition::expand_assignment(string chp)
{
	parallel *p = new parallel(this, "", vars, flags);
	assignment *a = new assignment(p, chp, vars, flags);
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
			if (result.second != NULL)
			{
				result.second->parent = p;
				p->push(result.second);
			}
			remove.push_back(*i);
		}
		else if (v != NULL)
		{
			p->push(new condition(p, "[" + i->second + "->" + i->first + "+[]~" + i->second + "->" + i->first + "-]", vars, flags));
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
	sequential *s = new sequential(this, "", vars, flags);
	parallel *p = new parallel(s, "", vars, flags);
	condition *a = new condition(s, chp, vars, flags);
	pair<string, instruction*> result;
	list<pair<sequential*, guard*> >::iterator i;

	for (i = a->instrs.begin(); i != a->instrs.end(); i++)
	{
		if (i->second->chp.find_first_of("&|~^=<>/+-*?#()") != string::npos)
		{
			result = a->expand_guard(i->second->chp);
			i->second->chp = result.first;
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
	sequential *s = new sequential(this, "", vars, flags);
	parallel *p = new parallel(s, "", vars, flags);
	loop *a = new loop(s, chp, vars, flags);
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
