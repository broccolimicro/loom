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
	clear();

	_kind = "conditional";
	this->uid = uid;
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
void conditional::parse(map<string, keyword*> *types)
{
	char nid = 'a';

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
				cout << "Error: A conditional can either be mutually exclusive or choice, but not both." << endl;

			blockstr = chp.substr(j-chp.begin(), i-j);
			k = blockstr.find("->") + blockstr.begin();
			guardstr = blockstr.substr(0, k-blockstr.begin());
			blockstr = blockstr.substr(k-blockstr.begin()+2);



			instrs.insert(pair<string, block*>(guardstr, new block(uid + nid++, blockstr, types, global, tab+"\t", verbosity)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}

	// TODO create a state variable per guarded block whose production rule is the guard.
	// TODO a possible optimization would be to check to make sure that we need one first. If we don't, then we must already have one that works, add the guard to it's condition.
	// TODO condition all production rules of the guarded blocks on their designated state variable.

/*	if (verbosity >= VERB_PARSE)
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
	}*/

}
//i->second.states.back()

/*	map<string, block*>::iterator ii, ij;
	map<string, state>::iterator si, sj;
	list<rule>::iterator rui;
	string::reverse_iterator ri, rj, rk;
*/

/*if (verbosity >= VERB_PARSE)
			{
				cout << tab << "Before\n";
				for (si = guardresult.begin(); si != guardresult.end(); si++)
					cout << tab << si->first << " -> " << si->second << endl;

				cout << tab << "Guard\n";
			}

			temp = guard(guardstr, vars]);
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
			*/

// Determine the resulting state of the conditional by
// merging the states of the guarded blocks.


/*for (ii = instrs.begin(); ii != instrs.end(); ii++)
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
}*/

/*
for (ii = instrs.begin(); ii != instrs.end(); ii++)
	{
		for (si = ii->second->states.begin(); si != ii->second->states.end(); si++)
		{
			if ((sj = states.find(si->first)) != states.end())
			{
				sj->second = sj->second || si->second;
				sj->second.prs = false;
			}
			else
				states.push_back(pair<string, state>(si->first, si->second.back()));
		}
	}
*/

/*
 if (verbosity >= VERB_PARSE)
			{
				cout << tab << "Before\n";
				for (si = guardresult.begin(); si != guardresult.end(); si++)
					cout << tab << si->first << " -> " << si->second << endl;

				cout << tab << "Guard\n";
			}
			temp = guard(guardstr, global);
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
 */

void conditional::generate_states(state init)
{
/*


	map<string, state>::iterator vi;
	map<string, space>::iterator vj;
	map<string, space>::iterator iter;


	for (vi = init.begin(); vi != init.end(); vi++)
	{
		space s;
		s.var = vi->first;
		s.states.push_back(vi->second);
		states.insert(pair<string, space>(s.var, s));
	}



//	for (iter = states.begin(); iter != states.end(); iter++)
//	{
		iter->second.states.back();
//	}

	for (iter = states.begin(); iter != states.end(); iter++)
	{
		//if ((vj = states.find(vi->first)) != states.end())
		//	vj->second.states.push_back(vi->second);
		if((vj = states.find(iter->first)) != states.end())
			vj->second.states.push_back(iter->second.states.back());
		else
		{
			space s;
			//s.var = vi->first;
			s.var = iter->first;
			s.states.push_back(state("X", false));
			//s.states.push_back(vi->second);
			s.states.push_back(iter->second.states.back());
			states.insert(pair<string, space>(s.var, s));
		}
	}*/

	map<string, block*>::iterator instr_iter;
	block *instr;

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = instr_iter->second;
		instr->generate_states(state());
	}
}

void conditional::generate_prs(map<string, variable*> globals){

	/*map<string, block*>::iterator ii;
	variable *v;
	list<state>::iterator sli;
	list<rule>::iterator rui;
	map<string, space>::iterator vj;
	rule ru;

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
	}*/



}

state guard(string raw,  map<string, variable*> vars, string tab, int verbosity)
{
	map<string, variable*>::iterator vi;
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

	vi = vars.find(raw);
	if (vi != vars.end())
		outcomes.insert(vi->second->uid, value("1"));

	for (ai = 0; ai != outcomes.size(); ai++)
	{
		if (verbosity >= VERB_PARSE)
			cout << tab << ai << ": " << outcomes[ai] << endl;
	}

	return outcomes;
}

