/*
 * loop.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "loop.h"
#include "common.h"

loop::loop()
{
	_kind = "loop";
}

loop::loop(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity)
{
	clear();

	_kind = "loop";
	this->chp = chp.substr(2, chp.length()-3);
	this->tab = tab;
	this->verbosity = verbosity;
	this->type = unknown;
	this->global = globals;
	this->label = label;

	expand_shortcuts();
	parse(types);
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
	this->rules		= l.rules;
	this->global	= l.global;
	this->label		= l.label;
	this->tab		= l.tab;
	this->verbosity	= l.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *loop::duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert, string tab, int verbosity)
{
	loop *instr;

	instr 				= new loop();
	instr->chp			= this->chp;
	instr->global		= globals;
	instr->label		= labels;
	instr->tab			= tab;
	instr->verbosity	= verbosity;
	instr->type			= this->type;

	map<string, string>::iterator i, j;
	size_t k;
	for (i = convert.begin(); i != convert.end(); i++)
		while ((k = find_name(instr->chp, i->first, k+1)) != instr->chp.npos)
			instr->chp.replace(k, i->first.length(), i->second);

	list<pair<block*, guard*> >::iterator l;
	for (l = instrs.begin(); l != instrs.end(); l++)
		instr->instrs.push_back(pair<block*, guard*>((block*)l->first->duplicate(globals, labels, convert, tab+"\t", verbosity), (guard*)l->second->duplicate(globals, labels, convert, tab+"\t", verbosity)));

	return instr;
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

void loop::parse(map<string, keyword*> types)
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

			instrs.push_back(pair<block*, guard*>(new block(blockstr, types, global, label, tab+"\t", verbosity), new guard(guardstr, types, global, label, tab+"\t", verbosity)));
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

			instrs.push_back(pair<block*, guard*>(new block(blockstr, types, global, label, tab+"\t", verbosity), new guard(guardstr, types, global, label, tab+"\t", verbosity)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}
}

int loop::generate_states(state_space *space, graph *trans, int init)
{
	cout << tab << "Loop " << chp << endl;

	list<pair<block*, guard*> >::iterator instr_iter;
	map<string, variable>::iterator vi;
	int guardresult = -1;
	vector<int> state_catcher;
	int next = init;
	bool done = false;
	bool sub = false;
	bool first = true;
	state s;
	int count = 0;
	int i = 0;

	while (!done && count++ < 5)
	{
		cout << tab << "Loop Iteration " << count << ": " << (*space)[next] << endl;

		first = true;
		for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
		{
			guardresult = instr_iter->second->generate_states(space, trans, next);
			state_catcher.push_back(instr_iter->first->generate_states(space, trans, guardresult));
			if (first)
			{
				s = (*space)[state_catcher.back()];
				first = false;
			}
			else
				s = s || (*space)[state_catcher.back()];
		}

		cout << tab << "Result " << s << endl;
		uid.push_back(space->size());

		space->push_back(s);

		next = uid.back();

		for (i = 0, instr_iter = instrs.begin(); i < (int)state_catcher.size() && instr_iter != instrs.end(); i++, instr_iter++)
			trans->insert_edge(state_catcher[i], next, instr_iter->second->chp + "->Block");
		state_catcher.clear();

		done = subset((*space)[init], (*space)[next]);

		if (done)
			trans->insert_edge(next, init,"Loop");
		for (int i = 0; i < (int)uid.size()-1; i++)
		{
			sub = subset((*space)[uid[i]], (*space)[next]);
			done = done || sub;
			if (sub && uid[i] != next)
				trans->insert_edge(next, uid[i], "Loop");
		}
	}

	s = (*space)[init];

	for (int i = 0; i < (int)uid.size(); i++)
		s = s || (*space)[uid[i]];

	state temp;
	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		temp = solve("~("+instr_iter->second->chp+")", instr_iter->second->global, instr_iter->second->tab, instr_iter->second->verbosity);
		cout << "Intersecting " << s << " and " << temp;
		s = s && temp;
		cout << " to get " << s << endl;
	}
	cout << tab << "Final Result " << s << endl;

	uid.push_back(space->size());
	space->push_back(s);
	trans->insert_edge(next, uid.back(), "Loop");

	next = uid.back();

	return next;
}

void loop::generate_prs()
{


}

void loop::print_hse()
{
	cout << "\n*[\t";
	list<pair<block*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin() && type == mutex)
			cout << "\n[]\t";
		else if (i != instrs.begin() && type == choice)
			cout << "\n|\t";
		i->second->print_hse();
		cout << "\t->\t";
		i->first->print_hse();
	}
	cout << "\n]";
}
