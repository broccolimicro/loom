/*
 * loop.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "loop.h"
#include "common.h"

loop::loop()
{
	_kind = "loop";
}

loop::loop(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab)
{
	_kind = "loop";
	parse(raw, types, vars, init, tab);
}

loop::~loop()
{
	_kind = "loop";

	map<string, instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	instrs.clear();
}

void loop::parse(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab)
{
	result.clear();
	local.clear();
	global.clear();
	instrs.clear();
	states.clear();

	chp = raw.substr(2, raw.length()-3);
	global = vars;						//The variables this block uses.
	type = unknown;
	string expr, eval;
	bool guarded = true;

	cout << tab << "Loop:\t" << chp << endl;

	map<string, instruction*>::iterator ii;
	map<string, state>::iterator si, sj;
	string::iterator i, j, k;

	//Check for the shorthand *[S] and replace it with *[1 -> S]
	if(chp.find("->") == chp.npos){
		cout << tab <<"Expanding " << chp;
		chp = "1->" + chp;
		cout << " to " << chp << endl;
	}

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
			cout << tab << "Choice\n";
			if (type == unknown)
				type = choice;
			else if (type == mutex)
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			eval = chp.substr(j-chp.begin(), i-j);
			k = eval.find("->") + eval.begin();
			expr = eval.substr(0, k-eval.begin());
			eval = eval.substr(k-eval.begin()+2);

			instrs.insert(pair<string, instruction*>(expr, new block(eval, types, global, guard(expr, vars, tab+"\t"), tab+"\t")));
			j = i+1;
			guarded = true;
		}
		else if (!guarded && depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '[' && *(i+1) == ']') || i == chp.end()))
		{
			cout << tab << "Mutex\n";
			if (type == unknown)
				type = mutex;
			else if (type == choice)
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			eval = chp.substr(j-chp.begin(), i-j);
			k = eval.find("->") + eval.begin();
			expr = eval.substr(0, k-eval.begin());
			eval = eval.substr(k-eval.begin()+2);

			instrs.insert(pair<string, instruction*>(expr, new block(eval, types, global, guard(expr, vars, tab+"\t"), tab+"\t")));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
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

	cout << tab << "Result:\t";

	for (si = result.begin(); si != result.end(); si++)
		cout << "{" << si->first << " = " << si->second << "} ";
	cout << endl;
}
