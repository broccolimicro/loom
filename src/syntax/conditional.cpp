/*
 * conditional.cpp
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

#include "conditional.h"

conditional::conditional()
{
	_kind = "conditional";
	type = unknown;
}

conditional::conditional(string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	_kind = "conditional";
	type = unknown;
	this->chp = chp.substr(1, chp.length()-2);
	this->tab = tab;
	this->verbosity = verbosity;
	this->vars = vars;

	expand_shortcuts();
	parse();
	simplify();
}
conditional::~conditional()
{
	_kind = "conditional";
	type = unknown;

	list<pair<block*, guard*> >::iterator i;
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

conditional &conditional::operator=(conditional c)
{
	this->type		= c.type;
	this->uid		= c.uid;
	this->chp		= c.chp;
	this->instrs	= c.instrs;
	this->vars		= c.vars;
	this->tab		= c.tab;
	this->verbosity	= c.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *conditional::duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	conditional *instr;

	instr 				= new conditional();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;
	instr->type			= this->type;

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
		instr->instrs.push_back(pair<block*, guard*>((block*)l->first->duplicate(vars, convert, tab+"\t", verbosity), (guard*)l->second->duplicate(vars, convert, tab+"\t", verbosity)));

	return instr;
}

void conditional::expand_shortcuts()
{
	//Check for the shorthand [var] and replace it with [var -> skip]
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

	chp += "->skip";
}
// [G -> S]
void conditional::parse()
{
	string::iterator i, j, k;
	string guardstr, blockstr;
	bool guarded = true;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Conditional:\t" << chp << endl;

	//Parse instructions!
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
				cout << "Error: A conditional can either be mutually exclusive or choice, but not both." << endl;

			blockstr = chp.substr(j-chp.begin(), i-j);
			k = blockstr.find("->") + blockstr.begin();
			guardstr = blockstr.substr(0, k-blockstr.begin());
			blockstr = blockstr.substr(k-blockstr.begin()+2);

			instrs.push_back(pair<block*, guard*>(new block(blockstr, vars, tab+"\t", verbosity), new guard(guardstr, vars, tab+"\t", verbosity)));
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
				cout << "Error: A conditional can either be mutually exclusive or choice, but not both." << endl;

			blockstr = chp.substr(j-chp.begin(), i-j);
			k = blockstr.find("->") + blockstr.begin();
			guardstr = blockstr.substr(0, k-blockstr.begin());
			blockstr = blockstr.substr(k-blockstr.begin()+2);

			instrs.push_back(pair<block*, guard*>(new block(blockstr, vars, tab+"\t", verbosity), new guard(guardstr, vars, tab+"\t", verbosity)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}
}

void conditional::simplify()
{
	list<pair<block*, guard*> >::iterator i, j;
	list<instruction*>::iterator ii;
	block *copy;
	block *nblock;
	guard *nguard;
	conditional *front;
	list<pair<block*, guard*> > add;

	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i->first->instrs.size() > 0 && i->first->instrs.front()->kind() == "conditional")
		{
			cout << "Simplifying" << endl;
			front = (conditional*)i->first->instrs.front();
			i->first->instrs.pop_front();

			j = front->instrs.begin();
			for (j++; j != front->instrs.end(); j++)
			{
				nblock = j->first;
				copy = (block*)i->first->duplicate(vars, map<string, string>(), tab, verbosity);
				for (ii = copy->instrs.begin(); ii != copy->instrs.end(); ii++)
					nblock->instrs.push_back(*ii);
				copy->instrs.clear();
				delete copy;
				nguard = j->second;
				nguard->chp = "(" + i->second->chp + ")&(" + nguard->chp + ")";
				add.push_back(pair<block*, guard*>(nblock, nguard));
			}
			j = front->instrs.begin();

			i->second->chp = "(" + i->second->chp + ")&(" + j->second->chp + ")";
			for (ii = j->first->instrs.begin(); ii != j->first->instrs.end(); ii++)
				i->first->instrs.push_front(*ii);
			j->first->instrs.clear();
			delete j->first;
			delete j->second;
			front->instrs.clear();
			delete front;
		}
	}

	for (i = add.begin(); i != add.end(); i++)
		instrs.push_back(*i);
	add.clear();
}

int conditional::generate_states(graph *g, int init)
{
	space = g;
	from = init;
	cout << tab << "Conditional " << chp << endl;

	list<pair<block*, guard*> >::iterator instr_iter;
	map<string, variable>::iterator vi;
	int guard_result = init;
	vector<int> state_catcher;
	vector<string> chp_catcher;
	state s;
	bool first = true;

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		guard_result = instr_iter->second->generate_states(g, init);

		state_catcher.push_back(instr_iter->first->generate_states(g, guard_result));
		if (CHP_EDGE)
			chp_catcher.push_back(instr_iter->first->chp);
		else
			chp_catcher.push_back("Conditional Merge");
		if (first)
		{
			s = g->states[state_catcher.back()];
			first = false;
		}
		else
			s = s || g->states[state_catcher.back()];
	}

	if (state_catcher.size() > 1)
	{
		uid = g->states.size();
		g->append_state(s, state_catcher, chp_catcher);
	}
	else
		uid = state_catcher.back();

	return uid;
}

void conditional::generate_scribes()
{
	list<pair<block*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		i->second->generate_scribes();
		i->first->generate_scribes();
	}
}

void conditional::print_hse()
{
	if (instrs.size() > 1)
		cout << "\n" << tab;
	cout << "[";
	if (instrs.size() > 1)
		cout << "\t";

	list<pair<block*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin() && type == mutex)
		{
			if (instrs.size() > 1)
				cout << "\n" << tab;
			cout << "[]";
			if (instrs.size() > 1)
				cout << "\t";
		}
		else if (i != instrs.begin() && type == choice)
		{
			if (instrs.size() > 1)
				cout << "\n" << tab;
			cout << "|";
			if (instrs.size() > 1)
				cout << "\t";
		}
		i->second->print_hse();
		if (i->first->instrs.size() > 0)
		{
			cout << " -> ";
			i->first->print_hse();
		}
	}
	if (instrs.size() > 1)
		cout << "\n" << tab;
	cout << "]";
}
