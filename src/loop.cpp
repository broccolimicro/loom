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

loop::loop(string raw, map<string, keyword*> types, map<string, variable*> vars, string tab)
{
	_kind = "loop";
	parse(raw, types, vars, tab);
}

loop::~loop()
{
	_kind = "loop";
}

void loop::parse(string raw, map<string, keyword*> types, map<string, variable*> vars, string tab)
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

	cout << tab << "Loop:\t" << chp << endl;

	string::iterator i, j, k;

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

		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) != '|' && *(i-1) != '|') || (i == chp.end() && type == choice)))
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

			instrs.insert(pair<string, instruction>(expr, block(eval, types, global,  map<string, state>(), tab+"\t")));
			j = i+1;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '[' && *(i+1) == ']') || i == chp.end()))
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

			instrs.insert(pair<string, instruction>(expr, block(eval, types, global,  map<string, state>(), tab+"\t")));
			j = i+2;
		}
	}
}
