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

loop::loop(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity)
{
	_kind = "loop";
	parse(id, raw, types, vars, init, tab, verbosity);
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

void loop::parse(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity)
{
	map<string, state> next_init;
	map<string, state>::iterator si, sj;

	chp = raw.substr(2, raw.length()-3);
	uid = id;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Loop Preparse:\t" << chp << endl;

	// TODO we should repeat this step until no deltas in the initial conditions are detected
	pass(chp, types, vars, init, tab, VERB_SUPPRESS);

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
}

void loop::pass(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity)
{
	result.clear();
	local.clear();
	global.clear();
	instrs.clear();
	states.clear();
	waits.clear();
	changes.clear();
	rules.clear();

	global = vars;						//The variables this block uses.
	type = unknown;
	string expr, eval;
	bool guarded;
	char nid = 'a';

	map<string, block*>::iterator ii;
	map<string, state>::iterator si, sj;
	string::iterator i, j, k;

	map<string, state> guardresult, temp;
	string::reverse_iterator ri, rj, rk;

	guardresult = init;

	//Check for the shorthand *[S] and replace it with *[1 -> S]
	int depth[3] = {0};
	guarded = false;
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
		{
			guarded = true;
			break;
		}
	}

	if(!guarded)
		chp = "1->" + chp;

	//Parse instructions!
	guarded = true;
	depth[0] = 0;
	depth[1] = 0;
	depth[2] = 0;
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

			eval = chp.substr(j-chp.begin(), i-j);
			k = eval.find("->") + eval.begin();
			expr = eval.substr(0, k-eval.begin());
			eval = eval.substr(k-eval.begin()+2);

			if (verbosity >= VERB_PARSE)
			{
				cout << tab << "Before\n";
				for (si = guardresult.begin(); si != guardresult.end(); si++)
					cout << tab << si->first << " -> " << si->second << endl;

				cout << tab << "Guard\n";
			}
			temp = guard(expr, vars, tab+"\t", verbosity);
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
			}

			instrs.insert(pair<string, block*>(expr, new block(uid + nid++, eval, types, global, guardresult, tab+"\t", verbosity)));
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

			eval = chp.substr(j-chp.begin(), i-j);
			k = eval.find("->") + eval.begin();
			expr = eval.substr(0, k-eval.begin());
			eval = eval.substr(k-eval.begin()+2);

			if (verbosity >= VERB_PARSE)
			{
				cout << tab << "Before\n";
				for (si = guardresult.begin(); si != guardresult.end(); si++)
					cout << tab << si->first << " -> " << si->second << endl;

				cout << tab << "Guard\n";
			}
			temp = guard(expr, vars, tab+"\t", verbosity);
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
			}

			instrs.insert(pair<string, block*>(expr, new block(uid + nid++, eval, types, global, guardresult, tab+"\t", verbosity)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}

	for (ii = instrs.begin(); ii != instrs.end(); ii++)
	{
		for (si = ii->second->result.begin(); si != ii->second->result.end(); si++)
		{
			if ((sj = result.find(si->first)) != result.end())
				sj->second = sj->second || si->second;
			else
				result.insert(pair<string, state>(si->first, si->second));
		}
	}
}
