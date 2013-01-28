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

conditional::conditional(string chp, map<string, keyword*> types, map<string, variable> *globals, string tab, int verbosity)
{
	clear();

	_kind = "conditional";
	this->chp = chp.substr(1, chp.length()-2);
	this->tab = tab;
	this->verbosity = verbosity;
	this->global = globals;
	type = unknown;

	expand_shortcuts();
	parse(types);
}
conditional::~conditional()
{
	_kind = "conditional";
	type = unknown;

	map<string, block*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	instrs.clear();
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

			instrs.insert(pair<string, block*>(guardstr, new block( blockstr, types, global, tab+"\t", verbosity)));
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



			instrs.insert(pair<string, block*>(guardstr, new block( blockstr, types, global, tab+"\t", verbosity)));
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

void conditional::generate_states(state_space *space, graph *trans, int init)
{
	map<string, block*>::iterator instr_iter;
	block *instr;

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = instr_iter->second;
		instr->generate_states(space, trans, init);
	}
}

void conditional::generate_prs(map<string, variable> *globals)
{
}

state guard(string raw,  map<string, variable> *vars, string tab, int verbosity)
{
	map<string, variable>::iterator vi;
	state outcomes;
	state a, b;
	int ai, bi;
	string::iterator i, j;
	value temp;
	int depth;

	if (verbosity >= VERB_PARSE)
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
			a = guard(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity);
			b = guard(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			for (ai = 0; ai != a.size(); ai++)
			{
				if (ai >= b.size())
				{
					temp = a[ai] || (~a[ai]);
					outcomes.insert(ai, temp);
				}
				else
					outcomes.insert(ai, a[ai]);
			}

			for (bi = 0; bi != b.size(); bi++)
			{
				if (bi >= a.size())
				{
					temp = b[bi] || (~b[bi]);
					outcomes.insert(bi, temp);
				}
				else
				{
					a[bi] = a[bi] || b[bi];
				}
			}

			for (ai = 0; ai != outcomes.size(); ai++)
			{
				if (verbosity >= VERB_PARSE)
					cout << tab << ai << ": " << outcomes[ai] << endl;
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

		if (depth == 0 && *i == '&')
		{
			a = guard(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity);
			b = guard(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			outcomes = a;

			for (bi = 0; bi != b.size(); bi++)
			{
				if (bi >= outcomes.size())
					outcomes.insert(bi, b[bi]);
				else
					outcomes[bi] = outcomes[bi] && b[bi];
			}

			for (ai = 0; ai != outcomes.size(); ai++)
			{
				if (verbosity >= VERB_PARSE)
					cout << tab << ai << ": " << outcomes[ai] << endl;
			}

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

	// TODO: Add support for the following operators {<<,>>,+,-,*,/}
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
			b = guard(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			for (bi = 0; bi != b.size(); bi++)
				outcomes.insert(bi, ~b[bi]);

			for (ai = 0; ai != outcomes.size(); ai++)
			{
				if (verbosity >= VERB_PARSE)
					cout << tab << ai << ": " << outcomes[ai] << endl;
			}

			return outcomes;
		}
	}

	unsigned long s = raw.find_first_of("(");
	unsigned long e = raw.find_last_of(")");
	if (s != raw.npos && e != raw.npos)
	{
		a = guard(raw.substr(s+1, e-s-1), vars, tab+"\t", verbosity);

		outcomes = a;

		for (ai = 0; ai != outcomes.size(); ai++)
		{
			if (verbosity >= VERB_PARSE)
				cout << tab << ai << ": " << outcomes[ai] << endl;
		}

		return outcomes;
	}

	vi = vars->find(raw);
	if (vi != vars->end())
		outcomes.insert(vi->second.uid, value("1"));

	for (ai = 0; ai != outcomes.size(); ai++)
	{
		if (verbosity >= VERB_PARSE)
			cout << tab << ai << ": " << outcomes[ai] << endl;
	}

	return outcomes;
}

