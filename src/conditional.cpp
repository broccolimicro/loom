/*
 * conditional.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "conditional.h"
#include "common.h"

conditional::conditional()
{
	_kind = "conditional";
	type = unknown;
}

conditional::conditional(string raw, map<string, variable*> svars, string tab)
{
	_kind = "conditional";
	parse(raw, svars, tab);
}

conditional::~conditional()
{
	_kind = "conditional";
	type = unknown;
}

void conditional::parse(string raw, map<string, variable*> svars, string tab)
{
	chp = raw.substr(1, raw.length()-2);
	global = svars;						//The variables this block uses.
	type = unknown;
	string expr, eval;

	cout << tab << "Conditional:\t" << chp << endl;

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
				cout << "Error: A conditional can either be mutually exclusive or choice, but not both." << endl;

			eval = chp.substr(j-chp.begin(), i-j);
			k = eval.find("->") + eval.begin();
			expr = eval.substr(0, k-eval.begin());
			eval = eval.substr(k-eval.begin()+2);

			instrs.insert(pair<string, instruction>(expr, block(eval, global, tab+"\t")));
			j = i+1;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '[' && *(i+1) == ']') || i == chp.end()))
		{
			cout << tab << "Mutex\n";
			if (type == unknown)
				type = mutex;
			else if (type == choice)
				cout << "Error: A conditional can either be mutually exclusive or choice, but not both." << endl;

			eval = chp.substr(j-chp.begin(), i-j);
			k = eval.find("->") + eval.begin();
			expr = eval.substr(0, k-eval.begin());
			eval = eval.substr(k-eval.begin()+2);

			instrs.insert(pair<string, instruction>(expr, block(eval, global, tab+"\t")));
			j = i+2;
		}
	}
}
