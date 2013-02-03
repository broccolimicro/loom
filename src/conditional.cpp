/*
 * conditional.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "conditional.h"
#include "common.h"

conditional::conditional()
{
	_kind = "conditional";
	type = unknown;
}

conditional::conditional(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity)
{
	clear();

	_kind = "conditional";
	this->chp = chp.substr(1, chp.length()-2);
	this->tab = tab;
	this->verbosity = verbosity;
	this->global = globals;
	type = unknown;
	this->label = label;

	expand_shortcuts();
	parse(types);
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
	this->rules		= c.rules;
	this->global	= c.global;
	this->label		= c.label;
	this->tab		= c.tab;
	this->verbosity	= c.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 * TODO Check to make sure that this actually works as specified
 */
instruction *conditional::duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert)
{
	conditional *instr;

	instr 				= new conditional();
	instr->chp			= this->chp;
	instr->global		= globals;
	instr->label		= labels;
	instr->tab			= this->tab;
	instr->verbosity	= this->verbosity;
	instr->type			= this->type;

	map<string, string>::iterator i, j;
	size_t k;
	for (i = convert.begin(); i != convert.end(); i++)
		while ((k = find_name(instr->chp, i->first)) != instr->chp.npos)
			instr->chp.replace(k, i->first.length(), i->second);

	list<pair<block*, guard*> >::iterator l;
	for (l = instrs.begin(); l != instrs.end(); l++)
		instr->instrs.push_back(pair<block*, guard*>((block*)l->first->duplicate(globals, labels, convert), (guard*)l->second->duplicate(globals, labels, convert)));

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
void conditional::parse(map<string, keyword*> types)
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

			instrs.push_back(pair<block*, guard*>(new block( blockstr, types, global, label, tab+"\t", verbosity), new guard(guardstr, types, global, label, tab+"\t", verbosity)));
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

			instrs.push_back(pair<block*, guard*>(new block(blockstr, types, global, label, tab+"\t", verbosity), new guard(guardstr, types, global, label, tab+"\t", verbosity)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}

	// TODO create a state variable per guarded block whose production rule is the guard.
	// TODO a possible optimization would be to check to make sure that we need one first. If we don't, then we must already have one that works, add the guard to it's condition.
	// TODO condition all production rules of the guarded blocks on their designated state variable.
}

int conditional::generate_states(state_space *space, graph *trans, int init)
{
	cout << tab << "Conditional " << chp << endl;

	list<pair<block*, guard*> >::iterator instr_iter;
	map<string, variable>::iterator vi;
	int guard_result = -1;
	vector<int> state_catcher;
	state s;

	for (vi = global->begin(); vi != global->end(); vi++)
		s.assign(vi->second.uid, value("_"));

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		guard_result = instr_iter->second->generate_states(space, trans, init);

		state_catcher.push_back(instr_iter->first->generate_states(space, trans, guard_result));
		s = s || (*space)[state_catcher.back()];
	}

	uid = space->size();
	space->push_back(s);

	for (int i = 0; i < (int)state_catcher.size(); i++)
		trans->insert_edge(state_catcher[i], uid, chp);

	return uid;
}

void conditional::generate_prs()
{
}

