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
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			blockstr = chp.substr(j-chp.begin(), i-j);
			k = blockstr.find("->") + blockstr.begin();
			guardstr = blockstr.substr(0, k-blockstr.begin());
			blockstr = blockstr.substr(k-blockstr.begin()+2);

			instrs.push_back(pair<block*, guard*>(new block( blockstr, types, global, label, tab+"\t", verbosity), new guard(guardstr, types, global, label, tab+"\t", verbosity)));
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
	state s;
	int count = 0;

	while (!done && count++ < 5)
	{
		cout << "Loop Iteration " << count << ": " << (*space)[next] << endl;
		for (vi = global->begin(); vi != global->end(); vi++)
			s.assign(vi->second.uid, value("_"));

		for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
		{
			guardresult = instr_iter->second->generate_states(space, trans, next);
			trans->insert_edge(next, guardresult);		//Tie init to each of the states from guards
			state_catcher.push_back(instr_iter->first->generate_states(space, trans, guardresult));
			trans->insert_edge(guardresult, state_catcher.back());		//Tie init to each of the states from guards
			cout << "Unioning " << s << " and " << (*space)[state_catcher.back()] << endl;
			s = s || (*space)[state_catcher.back()];
		}

		cout << "Result " << s << endl;
		uid.push_back(space->size());

		space->push_back(s);

		next = uid.back();

		for (int i = 0; i < state_catcher.size(); i++)
			trans->insert_edge(state_catcher[i], next);

		done = subset((*space)[init], (*space)[next]);
		for (int i = 0; i < uid.size()-1; i++)
			done = done || subset((*space)[uid[i]], (*space)[next]);
	}

	s = (*space)[init];
	for (int i = 0; i < uid.size(); i++)
	{
		s = s || (*space)[uid[i]];
		trans->insert_edge(uid[i], space->size());
	}

	cout << "Final Result " << s << endl;

	uid.push_back(space->size());
	space->push_back(s);
	next = uid.back();

	return next;
}

void loop::generate_prs()
{


}
