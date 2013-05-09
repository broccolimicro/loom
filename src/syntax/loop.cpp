/*
 * loop.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "loop.h"

loop::loop()
{
	_kind = "loop";
}

loop::loop(instruction *parent, string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	_kind = "loop";
	this->chp = chp.substr(2, chp.length()-3);
	this->tab = tab;
	this->verbosity = verbosity;
	this->type = unknown;
	this->vars = vars;
	this->parent = parent;

	expand_shortcuts();
	parse();
}

loop::~loop()
{
	_kind = "loop";

	instrs.clear();
}

loop &loop::operator=(loop l)
{
	this->type		= l.type;
	this->uid		= l.uid;
	this->chp		= l.chp;
	this->instrs	= l.instrs;
	this->vars		= l.vars;
	this->space		= l.space;
	this->tab		= l.tab;
	this->verbosity	= l.verbosity;
	this->parent	= l.parent;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *loop::duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	loop *instr;

	instr 				= new loop();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;
	instr->type			= this->type;
	instr->parent		= parent;

	size_t idx;
	string rep;

	map<string, string>::iterator i, j;
	size_t k = 0, min, curr;
	while (k != instr->chp.npos)
	{
		j = convert.end();
		min = instr->chp.length();
		curr = 0;
		for (i = convert.begin(); i != convert.end(); i++)
		{
			curr = find_name(instr->chp, i->first, k);
			if (curr < min)
			{
				min = curr;
				j = i;
			}
		}

		if (j != convert.end())
		{
			rep = j->second;
			instr->chp.replace(min, j->first.length(), rep);
			if (instr->chp[min + rep.length()] == '[' && instr->chp[min + rep.length()-1] == ']')
			{
				idx = instr->chp.find_first_of("]", min + rep.length()) + 1;
				rep = flatten_slice(instr->chp.substr(min, idx - min));
				instr->chp.replace(min, idx - min, rep);
			}

			k = min + rep.length();
		}
		else
			k = instr->chp.npos;
	}

	list<pair<sequential*, guard*> >::iterator l;
	for (l = instrs.begin(); l != instrs.end(); l++)
		instr->instrs.push_back(pair<sequential*, guard*>((sequential*)l->first->duplicate(instr, vars, convert, tab+"\t", verbosity), (guard*)l->second->duplicate(instr, vars, convert, tab+"\t", verbosity)));

	return instr;
}

state loop::variant()
{
	state result(value("_"), vars->global.size());

	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		result = result || i->first->variant();
		result = result || i->second->variant();
	}
	return result;
}

state loop::active_variant()
{
	state result(value("_"), vars->global.size());

	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		result = result || i->first->active_variant();
		result = result || i->second->active_variant();
	}
	return result;
}

state loop::passive_variant()
{
	state result(value("_"), vars->global.size());

	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		result = result || i->first->passive_variant();
		result = result || i->second->passive_variant();
	}
	return result;
}

void loop::expand_shortcuts()
{
	//Check for the shorthand *[S] and replace it with *[1 -> S]
	string::iterator i, j;
	int depth[3] = {0};
	for (i = chp.begin(), j = chp.begin(); i != chp.end()-1; i++)
	{
		if (*i == '(')
			depth[0]++;
		else if (*i == '[')
			depth[1]++;
		else if (*i == '{')
			depth[2]++;
		else if (*i == ')')
			depth[0]--;
		else if (*i == ']')
			depth[1]--;
		else if (*i == '}')
			depth[2]--;

		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == '-' && *(i+1) == '>'))
			return;
	}

	chp = "1->" + chp;
}

void loop::parse()
{
	string guardstr, sequentialstr;

	string::iterator i, j, k;

	//Parse instructions!
	bool guarded = true;
	int depth[3] = {0};
	for (i = chp.begin(), j = chp.begin(); i != chp.end()+1; i++)
	{
		if (*i == '(')
			depth[0]++;
		else if (*i == '[')
			depth[1]++;
		else if (*i == '{')
			depth[2]++;
		else if (*i == ')')
			depth[0]--;
		else if (*i == ']')
			depth[1]--;
		else if (*i == '}')
			depth[2]--;

		if (!guarded && depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) != '|' && *(i-1) != '|') || (i == chp.end() && type == choice)))
		{
			if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
				cout << tab << "Choice\n";
			if (type == unknown)
				type = choice;
			else if (type == mutex)
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			sequentialstr = chp.substr(j-chp.begin(), i-j);
			k = sequentialstr.find("->") + sequentialstr.begin();
			guardstr = sequentialstr.substr(0, k-sequentialstr.begin());
			sequentialstr = sequentialstr.substr(k-sequentialstr.begin()+2);

			instrs.push_back(pair<sequential*, guard*>(new sequential(this, sequentialstr, vars, tab+"\t", verbosity), new guard(this, guardstr, vars, tab+"\t", verbosity)));
			j = i+1;
			guarded = true;
		}
		else if (!guarded && depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '[' && *(i+1) == ']') || i == chp.end()))
		{
			if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
				cout << tab << "Mutex\n";
			if (type == unknown)
				type = mutex;
			else if (type == choice)
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			sequentialstr = chp.substr(j-chp.begin(), i-j);
			k = sequentialstr.find("->") + sequentialstr.begin();
			guardstr = sequentialstr.substr(0, k-sequentialstr.begin());
			sequentialstr = sequentialstr.substr(k-sequentialstr.begin()+2);

			instrs.push_back(pair<sequential*, guard*>(new sequential(this, sequentialstr, vars, tab+"\t", verbosity), new guard(this, guardstr, vars, tab+"\t", verbosity)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}
}

void loop::merge()
{
	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		i->second->merge();
		i->first->merge();
	}
}

int loop::generate_states(graph *g, int init, state filter)
{
	list<pair<sequential*, guard*> >::iterator instr_iter;
	vector<int> state_catcher;
	vector<string> chp_catcher;
	int guardresult;
	string exp = "";
	state s, v;

	if (filter.size() == 0)
		filter = null(vars->size());

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << "Loop " << chp << endl;

	space = g;
	from = init;
	v = variant();
	s = null(vars->size());
	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
		s = s || instr_iter->first->simulate_states(instr_iter->second->simulate_states(full(vars->size()), null(vars->size())), filter);

	v = filter || (v && s);
	s = g->states[init] || v;
	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		guardresult = instr_iter->second->generate_states(g, init, v);
		state_catcher.push_back(instr_iter->first->generate_states(g, guardresult, filter));

		chp_catcher.push_back(CHP_EDGE ? (instr_iter->second->chp+"->Block") : "Loop merge");

		exp += (exp != "" ? "&~(" : "~(") + instr_iter->second->chp + ")";
	}

	s = s && solve(expression(exp).simple, vars, tab, verbosity);

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << s << endl;

	uid = g->append_state(s, state_catcher, chp_catcher);

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
		g->insert_edge(uid, instr_iter->second->uid, CHP_EDGE ? (instr_iter->second->chp + "->") : "Loopback");

	return uid;
}

state loop::simulate_states(state init, state filter)
{
	list<pair<sequential*, guard*> >::iterator instr_iter;
	string exp = "";
	state s, v;

	if (filter.size() == 0)
		filter = null(vars->size());

	v = variant();
	s = null(vars->size());
	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		s = s || instr_iter->first->simulate_states(instr_iter->second->simulate_states(full(vars->size()), null(vars->size())), filter);
		exp += (exp != "" ? "&~(" : "~(") + instr_iter->second->chp + ")";
	}
	v = filter || (v && s);
	s = init || v;

	s = s && solve(expression(exp).simple, vars, tab, verbosity);

	return s;
}

void loop::generate_scribes()
{
	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		i->second->generate_scribes();
		i->first->generate_scribes();
	}
}

void loop::insert_instr(int uid, int nid, instruction *instr)
{
	instr->uid = nid;
	instr->from = uid;

	list<pair<sequential*, guard*> >::iterator i, k;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i->second->uid == uid)
		{
			i->first->instrs.front()->from = nid;
			i->first->instrs.push_front(instr);
			return;
		}
		else if (i->first->uid == uid)
		{
			i->first->instrs.push_back(instr);
			i->first->uid = nid;
			return;
		}
	}

	for (i = instrs.begin(); i != instrs.end(); i++)
		i->first->insert_instr(uid, nid, instr);
}

void loop::print_hse(string t)
{
	cout << "\n" << t << "*[";
	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin() && type == mutex)
			cout << "\n" << t << "[]";
		else if (i != instrs.begin() && type == choice)
			cout << "\n" << t << "|";
		if (instrs.size() > 1 || i->second->chp != "1")
		{
			i->second->print_hse(t + "\t");
			cout << " ->\t";
		}
		else
			cout << "\t";
		i->first->print_hse(t + "\t");
	}
	cout << endl << t << "]";
}
