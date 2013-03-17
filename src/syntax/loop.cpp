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

	list<pair<block*, guard*> >::iterator l;
	for (l = instrs.begin(); l != instrs.end(); l++)
		instr->instrs.push_back(pair<block*, guard*>((block*)l->first->duplicate(instr, vars, convert, tab+"\t", verbosity), (guard*)l->second->duplicate(instr, vars, convert, tab+"\t", verbosity)));

	return instr;
}

state loop::variant()
{
	state result(value("_"), vars->global.size());

	list<pair<block*, guard*> >::iterator i;
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

	list<pair<block*, guard*> >::iterator i;
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

	list<pair<block*, guard*> >::iterator i;
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
	string guardstr, blockstr;

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
			if (verbosity >= VERB_PARSE)
				cout << tab << "Choice\n";
			if (type == unknown)
				type = choice;
			else if (type == mutex)
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			blockstr = chp.substr(j-chp.begin(), i-j);
			k = blockstr.find("->") + blockstr.begin();
			guardstr = blockstr.substr(0, k-blockstr.begin());
			blockstr = blockstr.substr(k-blockstr.begin()+2);

			instrs.push_back(pair<block*, guard*>(new block(this, blockstr, vars, tab+"\t", verbosity), new guard(this, guardstr, vars, tab+"\t", verbosity)));
			j = i+1;
			guarded = true;
		}
		else if (!guarded && depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '[' && *(i+1) == ']') || i == chp.end()))
		{
			if (verbosity >= VERB_PARSE)
				cout << tab << "Mutex\n";
			if (type == unknown)
				type = mutex;
			else if (type == choice)
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			blockstr = chp.substr(j-chp.begin(), i-j);
			k = blockstr.find("->") + blockstr.begin();
			guardstr = blockstr.substr(0, k-blockstr.begin());
			blockstr = blockstr.substr(k-blockstr.begin()+2);

			instrs.push_back(pair<block*, guard*>(new block(this, blockstr, vars, tab+"\t", verbosity), new guard(this, guardstr, vars, tab+"\t", verbosity)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}
}

int loop::generate_states(graph *g, int init, state filter)
{
	space = g;
	from = init;
	cout << tab << "Loop " << chp << endl;

	list<pair<block*, guard*> >::iterator instr_iter;
	map<string, variable>::iterator vi;

	vector<int> state_catcher;
	vector<string> chp_catcher;

	int guardresult;
	string exp = "";
	state s, v;

	v = filter || variant();
	s = g->states[init] || v;

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		guardresult = instr_iter->second->generate_states(g, init, v);
		state_catcher.push_back(instr_iter->first->generate_states(g, guardresult, filter));
		if(CHP_EDGE)
			chp_catcher.push_back(instr_iter->second->chp+"->Block");
		else
			chp_catcher.push_back("Loop merge");

		if (exp != "")
			exp += "&";

		exp += "~(" + instr_iter->second->chp + ")";
	}

	s = s && solve(expression(exp).simple, vars, tab, verbosity);

	cout << tab << "Result " << s << endl;
	uid = g->states.size();

	g->append_state(s, state_catcher, chp_catcher);
	g->insert_edge(uid, init, "Loop");

	return uid;
}

void loop::generate_scribes()
{
	list<pair<block*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		i->second->generate_scribes();
		i->first->generate_scribes();
	}
}

void loop::print_hse()
{
	cout << "\n" << tab << "*[";
	list<pair<block*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin() && type == mutex)
			cout << "\n" << tab << "[]";
		else if (i != instrs.begin() && type == choice)
			cout << "\n" << tab << "|";
		i->second->print_hse();
		cout << " ->\t";
		i->first->print_hse();
	}
	cout << "\n" << tab << "]";
}
