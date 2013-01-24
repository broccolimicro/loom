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

conditional::conditional(string uid, string chp, map<string, keyword*> *types, map<string, variable*> globals, string tab, int verbosity)
{
	_kind = "conditional";
	this->uid = uid;
	this->chp = chp.substr(1, raw.length()-2);
	this->tab = tab;
	this->verbosity = verbosity;
	this->global = globals;

	clear();
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
// [G -> S]
void conditional::parse(map<string, keyword*> *types)
{
	char nid = 'a';

	type = unknown;
	string expr, eval;
	bool guarded = true;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Conditional:\t" << chp << endl;

	map<string, state> guardresult, temp;

	map<string, block*>::iterator ii, ij;
	map<string, state>::iterator si, sj;
	string::iterator i, j, k;
	string::reverse_iterator ri, rj, rk;

	//Check for the shorthand [var] and replace it with [var -> skip]
	// TODO CHECK DEPTH!!!
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
			if (verbosity >= VERB_PARSE)
				cout << tab << "Choice\n";
			if (type == unknown)
				type = choice;
			else if (type == mutex)
				cout << "Error: A conditional can either be mutually exclusive or choice, but not both." << endl;

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
				// TODO I don't think that we can correctly make this assumption.
				// This assumes that if a variable has an X value as a result of the guard,
				// then we take the variable's previous value.
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
			guardresult = init;
		}
		else if (!guarded && depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '[' && *(i+1) == ']') || i == chp.end()))
		{
			if (verbosity >= VERB_PARSE)
				cout << tab << "Mutex\n";
			if (type == unknown)
				type = mutex;
			else if (type == choice)
				cout << "Error: A conditional can either be mutually exclusive or choice, but not both." << endl;

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
				// TODO I don't think that we can correctly make this assumption.
				// This assumes that if a variable has an X value as a result of the guard,
				// then we take the variable's previous value.
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
			j = i+2;
			guarded = true;
			guardresult = init;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}

	// Determine the resulting state of the conditional by
	// merging the states of the guarded blocks.
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
				result.insert(pair<string, state>(si->first, si->second));
		}
	}

	variable *v;
	list<rule>::iterator rui;
	list<state>::iterator sli;
	map<string, state>::iterator vi;
	map<string, space>::iterator vj;
	rule ru;

	for (vi = init.begin(); vi != init.end(); vi++)
	{
		space s;
		s.var = vi->first;
		s.states.push_back(vi->second);
		states.insert(pair<string, space>(s.var, s));
	}

	for (vi = result.begin(); vi != result.end(); vi++)
	{
		if ((vj = states.find(vi->first)) != states.end())
			vj->second.states.push_back(vi->second);
		else
		{
			space s;
			s.var = vi->first;
			s.states.push_back(state("X", false));
			s.states.push_back(vi->second);
			states.insert(pair<string, space>(s.var, s));
		}
	}

	int highest_state_name = 0;
	int bi1;
	bool first;
	for (ii = instrs.begin(); ii != instrs.end(); ii++)
	{
		// add a state variable per guarded block
		v = new variable("int<1>" + this->uid + "_" + to_string(highest_state_name++), tab, verbosity);
		this->local.insert(pair<string, variable*>(v->name, v));
		this->global.insert(pair<string, variable*>(v->name, v));

		if (ii->second->rules.size() > 0)
		{
			space s = ii->second->rules.begin()->left;
			s.var = v->name;
			for (sli = s.states.begin(); sli != s.states.end(); sli++)
			{
				sli->data = "1";
				sli->prs = false;
			}

			for (rui = ii->second->rules.begin(); rui != ii->second->rules.end(); rui++)
			{
				ru = *rui;
				ru.left &= s;
				rules.push_back(ru);
			}

			ru.clear(0);
			ru.right = s;
			ru.right.var += "+";
			first = true;
			for (vj = ii->second->states.begin(); vj != ii->second->states.end(); vj++)
			{
				// Expand multibit variables into their single bit constituents
				for (bi1 = 0; bi1 < global.find(vj->first)->second->width; bi1++)
				{
					space t = vj->second[bi1];

					if (t.states.front().data != "X")
					{
						if (t.states.front().data == "0")
							t = ~t;

						if (first)
							ru.left = t;
						else
							ru.left = t & ru.left;

						first = false;
					}
				}
			}

			rules.push_back(ru);

			ru.clear(0);
			ru.right = s;
			ru.right.var += "-";

			first = true;
			for (vj = ii->second->states.begin(); vj != ii->second->states.end(); vj++)
			{
				// Expand multibit variables into their single bit constituents
				for (bi1 = 0; bi1 < global.find(vj->first)->second->width; bi1++)
				{
					space t = vj->second[bi1];

					if (t.states.back().data != "X")
					{
						if (t.states.back().data == "0")
							t = ~t;

						if (first)
							ru.left = t;
						else
							ru.left = t & ru.left;

						first = false;
					}
				}
			}

			rules.push_back(ru);
		}
		v = NULL;
	}

	// TODO create a state variable per guarded block whose production rule is the guard.
	// TODO a possible optimization would be to check to make sure that we need one first. If we don't, then we must already have one that works, add the guard to it's condition.
	// TODO condition all production rules of the guarded blocks on their designated state variable.

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "Result:\t";

		for (si = result.begin(); si != result.end(); si++)
			cout << "{" << si->first << " = " << si->second << "} ";
		cout << endl;
	}

	if (verbosity >= VERB_STATEVAR)
	{
		for (rui = rules.begin(); rui != rules.end(); rui++)
			cout << tab << *rui << endl;
		if (rules.size() > 0)
			cout << endl;
	}
}

map<string, state> guard(string raw,  map<string, variable*> vars, string tab, int verbosity)
{
	map<string, state> outcomes;
	map<string, state> a, b;
	map<string, state>::iterator ai, bi;
	string::iterator i, j;
	state temp;
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

			for (ai = a.begin(); ai != a.end(); ai++)
			{
				if (b.find(ai->first) == b.end())
				{
					temp = ai->second || (~ai->second);
					outcomes.insert(pair<string, state>(ai->first, temp));
				}
				else
					outcomes.insert(pair<string, state>(ai->first, ai->second));
			}

			for (bi = b.begin(); bi != b.end(); bi++)
			{
				if (a.find(bi->first) == a.end())
				{
					temp = bi->second || (~bi->second);
					outcomes.insert(pair<string, state>(bi->first, temp));
				}
				else
				{
					ai = outcomes.find(bi->first);
					ai->second = ai->second || bi->second;
				}
			}

			for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
			{
				ai->second.prs = false;
				if (verbosity >= VERB_PARSE)
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

		if (depth == 0 && *i == '&')
		{
			a = guard(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity);
			b = guard(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			outcomes.insert(a.begin(), a.end());

			for (bi = b.begin(); bi != b.end(); bi++)
			{
				ai = outcomes.find(bi->first);
				if (ai == outcomes.end())
					outcomes.insert(pair<string, state>(bi->first, bi->second));
				else
					ai->second = ai->second && bi->second;
			}

			for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
			{
				ai->second.prs = false;
				if (verbosity >= VERB_PARSE)
					cout << tab << ai->first << ": " << ai->second << endl;
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

			for (bi = b.begin(); bi != b.end(); bi++)
				outcomes.insert(pair<string, state>(bi->first, ~bi->second));

			for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
			{
				ai->second.prs = false;
				if (verbosity >= VERB_PARSE)
					cout << tab << ai->first << ": " << ai->second << endl;
			}

			return outcomes;
		}
	}

	unsigned long s = raw.find_first_of("(");
	unsigned long e = raw.find_last_of(")");
	if (s != raw.npos && e != raw.npos)
	{
		a = guard(raw.substr(s+1, e-s-1), vars, tab+"\t", verbosity);

		outcomes.insert(a.begin(), a.end());

		for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
		{
			ai->second.prs = false;
			if (verbosity >= VERB_PARSE)
				cout << tab << ai->first << ": " << ai->second << endl;
		}

		return outcomes;
	}

	if (vars.find(raw) != vars.end())
		outcomes.insert(pair<string, state>(raw, state("1", false)));

	for (ai = outcomes.begin(); ai != outcomes.end(); ai++)
	{
		ai->second.prs = false;
		if (verbosity >= VERB_PARSE)
			cout << tab << ai->first << ": " << ai->second << endl;
	}

	return outcomes;
}
