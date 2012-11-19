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

conditional::conditional(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab)
{
	_kind = "conditional";
	parse(raw, types, vars, init, tab);
}

conditional::~conditional()
{
	_kind = "conditional";
	type = unknown;

	map<string, instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	instrs.clear();
}
// [G -> S]
void conditional::parse(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab)
{
	result.clear();
	local.clear();
	global.clear();
	instrs.clear();
	states.clear();

	chp = raw.substr(1, raw.length()-2);
	global = vars;						//The variables this block uses.
	type = unknown;
	string expr, eval;
	bool guarded = true;

	cout << tab << "Conditional:\t" << chp << endl;

	map<string, state> guardresult, temp;

	map<string, instruction*>::iterator ii;
	map<string, state>::iterator si, sj;
	string::iterator i, j, k;
	string::reverse_iterator ri, rj, rk;

	guardresult = init;

	//Check for the shorthand [var] and replace it with [var -> skip]
	if(chp.find("->") == chp.npos)
		chp = chp + "->skip";

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
				cout << "Error: A conditional can either be mutually exclusive or choice, but not both." << endl;

			eval = chp.substr(j-chp.begin(), i-j);
			k = eval.find("->") + eval.begin();
			expr = eval.substr(0, k-eval.begin());
			eval = eval.substr(k-eval.begin()+2);

			cout << "Before\n";
			for (si = guardresult.begin(); si != guardresult.end(); si++)
				cout << si->first << " -> " << si->second << endl;

			cout << "Guard\n";
			temp = guard(expr, vars, tab+"\t");
			for (si = temp.begin(); si != temp.end(); si++)
			{
				if ((sj = guardresult.find(si->first)) == guardresult.end())
					guardresult.insert(pair<string, state>(si->first, si->second));
				else
					for (ri = si->second.data.rbegin(), rj = sj->second.data.rbegin(); ri != si->second.data.rend() && rj != sj->second.data.rend(); ri++, rj++)
						if (*ri != 'X')
							*rj = *ri;

				cout << si->first << " -> " << si->second << endl;
			}

			cout << "After\n";
			for (si = guardresult.begin(); si != guardresult.end(); si++)
				cout << si->first << " -> " << si->second << endl;

			instrs.insert(pair<string, instruction*>(expr, new block(eval, types, global, guardresult, tab+"\t")));
			j = i+1;
			guarded = true;
			guardresult = init;
		}
		else if (!guarded && depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '[' && *(i+1) == ']') || i == chp.end()))
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

			cout << tab << "Before\n";
			for (si = guardresult.begin(); si != guardresult.end(); si++)
				cout << tab << si->first << " -> " << si->second << endl;

			cout << tab << "Guard\n";
			temp = guard(expr, vars, tab+"\t");
			for (si = temp.begin(); si != temp.end(); si++)
			{
				if ((sj = guardresult.find(si->first)) == guardresult.end())
				{
					si->second.prs = false;
					guardresult.insert(pair<string, state>(si->first, si->second));
				}
				else
					for (ri = si->second.data.rbegin(), rj = sj->second.data.rbegin(); ri != si->second.data.rend() && rj != sj->second.data.rend(); ri++, rj++)
						if (*ri != 'X')
							*rj = *ri;
			}

			cout << tab << "After\n";
			for (si = guardresult.begin(); si != guardresult.end(); si++)
				cout << tab << si->first << " -> " << si->second << endl;

			instrs.insert(pair<string, instruction*>(expr, new block(eval, types, global, guardresult, tab+"\t")));
			j = i+2;
			guarded = true;
			guardresult = init;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}

	for (ii = instrs.begin(); ii != instrs.end(); ii++)
	{
		for (si = ii->second->result.begin(); si != ii->second->result.end(); si++)
		{
			if ((sj = result.find(si->first)) != result.end())
			{
				sj->second = sj->second || si->second;
				sj->second.prs = false;
			}
			else
			{
				si->second.prs = false;
				result.insert(pair<string, state>(si->first, si->second));
			}
		}
	}

	cout << tab << "Result:\t";

	for (si = result.begin(); si != result.end(); si++)
		cout << "{" << si->first << " = " << si->second << "} ";
	cout << endl;
}

map<string, state> guard(string raw,  map<string, variable*> vars, string tab)
{
	map<string, state> outcomes;
	map<string, state> a, b;
	map<string, state>::iterator ai, bi;
	string::iterator i, j;
	state temp;
	int depth;

	cout << tab << "Guard: " << raw << endl;

	//Parse instructions!
	depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && *i == '|')
		{
			a = guard(raw.substr(j-raw.begin(), i-j), vars, tab+"\t");
			b = guard(raw.substr(i+1-raw.begin()), vars, tab+"\t");

			for (ai = a.begin(); ai != a.end(); ai++)
			{
				if (b.find(ai->first) == b.end())
				{
					temp = ai->second || (~ai->second);
					temp.prs = false;
					outcomes.insert(pair<string, state>(ai->first, temp));
				}
				else
				{
					ai->second.prs = false;
					outcomes.insert(pair<string, state>(ai->first, ai->second));
				}
			}

			for (bi = b.begin(); bi != b.end(); bi++)
			{
				if (a.find(bi->first) == a.end())
				{
					temp = bi->second || (~bi->second);
					temp.prs = false;
					outcomes.insert(pair<string, state>(bi->first, temp));
				}
				else
				{
					ai = outcomes.find(bi->first);
					ai->second = ai->second || bi->second;
					ai->second.prs = false;
				}
			}

			for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
				cout << tab << ai->first << ": " << ai->second << endl;

			return outcomes;
		}
	}

	depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && *i == '&')
		{
			a = guard(raw.substr(j-raw.begin(), i-j), vars, tab+"\t");
			b = guard(raw.substr(i+1-raw.begin()), vars, tab+"\t");

			outcomes.insert(a.begin(), a.end());

			for (bi = b.begin(); bi != b.end(); bi++)
			{
				ai = outcomes.find(bi->first);
				if (ai == outcomes.end())
				{
					bi->second.prs = false;
					outcomes.insert(pair<string, state>(bi->first, bi->second));
				}
				else
				{
					ai->second = ai->second && bi->second;
					ai->second.prs = false;
				}
			}

			for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
				cout << tab << ai->first << ": " << ai->second << endl;

			return outcomes;
		}
	}

	/*depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && *i == '=')
		{
			if (*(i-1) == '=')
			{
				a = guard(raw.substr(j-raw.begin(), i-j-1), tab+"\t");
				b = guard(raw.substr(i+1-raw.begin()), tab+"\t");

				for (bi = b.begin(); bi != b.end(); bi++)
					outcomes.insert(pair<string, state>(bi->first, bi->second || ~(bi->second)));

				for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
					cout << tab << ai->first << ": " << ai->second << endl;
			}
			else if (*(i-1) == '~')
			{
				a = guard(raw.substr(j-raw.begin(), i-j-1), tab+"\t");
				b = guard(raw.substr(i+1-raw.begin()), tab+"\t");

				for (bi = b.begin(); bi != b.end(); bi++)
					outcomes.insert(pair<string, state>(bi->first, bi->second || ~(bi->second)));

				for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
					cout << tab << ai->first << ": " << ai->second << endl;
			}

			return outcomes;
		}
	}

	depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && ((*i == '<' && *(i+1) != '<') || (*i == '>' && *(i+1) != '>')))
		{
			if (*(i+1) == '=')
			{
				a = guard(raw.substr(j-raw.begin(), i-j), tab+"\t");
				b = guard(raw.substr(i+2-raw.begin()), tab+"\t");

				for (bi = b.begin(); bi != b.end(); bi++)
					outcomes.insert(pair<string, state>(bi->first, bi->second || ~(bi->second)));

				for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
					cout << tab << ai->first << ": " << ai->second << endl;
			}
			else
			{
				a = guard(raw.substr(j-raw.begin(), i-j), tab+"\t");
				b = guard(raw.substr(i+1-raw.begin()), tab+"\t");

				for (bi = b.begin(); bi != b.end(); bi++)
					outcomes.insert(pair<string, state>(bi->first, bi->second || ~(bi->second)));

				for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
					cout << tab << ai->first << ": " << ai->second << endl;
			}

			return outcomes;
		}
	}*/

	// TODO: << >> + - * /
	// For shifting, reverse the shift. For add, you subtract and visa versa. For multiple, you divide and visa versa

	depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && *i == '~')
		{
			b = guard(raw.substr(i+1-raw.begin()), vars, tab+"\t");

			for (bi = b.begin(); bi != b.end(); bi++)
			{
				bi->second = ~bi->second;
				bi->second.prs = false;
				outcomes.insert(pair<string, state>(bi->first, bi->second));
			}

			for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
				cout << tab << ai->first << ": " << ai->second << endl;

			return outcomes;
		}
	}

	unsigned long s = raw.find_first_of("(");
	unsigned long e = raw.find_last_of(")");
	if (s != raw.npos && e != raw.npos)
	{
		a = guard(raw.substr(s+1, e-s-1), vars, tab+"\t");

		outcomes.insert(a.begin(), a.end());

		for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
			cout << tab << ai->first << ": " << ai->second << endl;

		return outcomes;
	}

	if (vars.find(raw) != vars.end())
		outcomes.insert(pair<string, state>(raw, state("1", false)));

	for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
		cout << tab << ai->first << ": " << ai->second << endl;

	return outcomes;
}
