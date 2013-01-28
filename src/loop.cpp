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

loop::loop(string uid, string chp, map<string, keyword*> *types, map<string, variable*> globals, string tab, int verbosity)
{
	clear();

	_kind = "loop";
	this->uid = uid;
	this->chp = chp.substr(2, chp.length()-3);
	this->tab = tab;
	this->verbosity = verbosity;
	this->global = globals;
	this->type = unknown;

	expand_shortcuts();
	parse(types);
}

loop::~loop()
{
	_kind = "loop";

	map<string, block*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

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

void loop::parse(map<string, keyword*> *types)
{
	string guardstr, blockstr;
	char nid = 'a';

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

			instrs.insert(pair<string, block*>(guardstr, new block(uid + nid++, blockstr, types, global, tab+"\t", verbosity)));
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

			instrs.insert(pair<string, block*>(guardstr, new block(uid + nid++, blockstr, types, global, tab+"\t", verbosity)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}
}


void loop::generate_states(state init)
{

	map<string, block*>::iterator instr_iter;
	block *instr;

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = instr_iter->second;
		instr->generate_states(state());
	}
}

void loop::generate_prs(map<string, variable*> globals)
{


}
/*map<string, block*>::iterator ii;
map<string, state>::iterator si, sj;

map<string, state> temp;
string::reverse_iterator ri, rj, rk;*/

/*for (ii = instrs.begin(); ii != instrs.end(); ii++)
{
	for (si = ii->second->result.begin(); si != ii->second->result.end(); si++)
	{
		if ((sj = result.find(si->first)) != result.end())
			sj->second = sj->second || si->second;
		else
			result.insert(pair<string, state>(si->first, si->second));
	}
}
*/


/*if (verbosity >= VERB_PARSE)
{
	cout << tab << "Before\n";
	for (si = guardresult.begin(); si != guardresult.end(); si++)
		cout << tab << si->first << " -> " << si->second << endl;

	cout << tab << "Guard\n";
}
temp = guard(guardstr, vars, tab+"\t", verbosity);
for (si = temp.begin(); si != temp.end(); si++)
{
	if ((sj = guardresult.find(si->first)) == guardresult.end())
		guardresult.insert(pair<string, state>(si->first, si->second));
	else
		for (ri = si->second.data.rbegin(), rj = sj->second.data.rbegin(); ri != si->second.data.rend() && rj != sj->second.data.rend(); ri++, rj++)
			if (*ri != 'X')
				*rj = *ri;

	if (verbosity >= VERB_PARSE)
		cout << tab << si->first << " -> " << si->second << endl;
}

if (verbosity >= VERB_PARSE)
{
	cout << tab << "After\n";
	for (si = guardresult.begin(); si != guardresult.end(); si++)
		cout << tab << si->first << " -> " << si->second << endl;
}*/

/*if (verbosity >= VERB_PARSE)
{
	cout << tab << "Before\n";
	for (si = guardresult.begin(); si != guardresult.end(); si++)
		cout << tab << si->first << " -> " << si->second << endl;

	cout << tab << "Guard\n";
}
temp = guard(guardstr, vars, tab+"\t", verbosity);
for (si = temp.begin(); si != temp.end(); si++)
{
	if ((sj = guardresult.find(si->first)) == guardresult.end())
		guardresult.insert(pair<string, state>(si->first, si->second));
	else
		for (ri = si->second.data.rbegin(), rj = sj->second.data.rbegin(); ri != si->second.data.rend() && rj != sj->second.data.rend(); ri++, rj++)
			if (*ri != 'X')
				*rj = *ri;

	if (verbosity >= VERB_PARSE)
		cout << si->first << " -> " << si->second << endl;
}

if (verbosity >= VERB_PARSE)
{
	cout << tab << "After\n";
	for (si = guardresult.begin(); si != guardresult.end(); si++)
		cout << tab << si->first << " -> " << si->second << endl;
}*/



/*
	map<string, state> next_init;
	map<string, state>::iterator si, sj;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Loop Preparse:\t" << chp << endl;

	// TODO we should repeat this step until no deltas in the initial conditions are detected
	pass(types);

	next_init.insert(init.begin(), init.end());

	for (si = result.begin(); si != result.end(); si++)
	{
		sj = next_init.find(si->first);
		if (sj == next_init.end())
		{
			next_init.insert(pair<string, state>(si->first, state(si->second.data, false)));
			if (verbosity >= VERB_PARSE)
				cout << tab << "Loop Vars: " << si->first << " " << si->second << endl;
		}
		else
		{
			sj->second = sj->second || si->second;
			sj->second.prs = false;
			if (verbosity >= VERB_PARSE)
				cout << tab << "Loop Vars: " << sj->first << " " << sj->second << " " << si->second << endl;
		}
	}

	map<string, variable*>::iterator i;
	for (i = local.begin(); i != local.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	local.clear();

	map<string, block*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (j->second != NULL)
			delete j->second;
		j->second = NULL;
	}

	instrs.clear();

	if (verbosity >= VERB_PARSE)
		cout << tab << "Loop Parse:\t" << chp << endl;

	pass(chp, types, vars, next_init, tab, verbosity);

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "Result:\t";
		for (si = result.begin(); si != result.end(); si++)
			cout << "{" << si->first << " = " << si->second << "} ";
		cout << endl;
	}
}*/
