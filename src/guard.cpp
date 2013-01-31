#include "guard.h"

guard::guard()
{
	_kind = "guard";
}

guard::guard(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity)
{
	this->_kind		= "guard";
	this->chp		= chp;
	this->tab		= tab;
	this->verbosity = verbosity;
	this->global	= globals;
	this->label		= label;

	expand_shortcuts();
	parse(types);
}

guard::~guard()
{
	_kind = "guard";
}

void guard::expand_shortcuts()
{
}

void guard::parse(map<string, keyword*> types)
{
	if (verbosity >= VERB_PARSE)
		cout << tab << "Guard:\t" + chp << endl;
}

int guard::generate_states(state_space *space, graph *trans, int init)
{
	map<string, variable>::iterator vi;
	int i, j;

	cout << tab << "Guard " << chp << endl;

	uid = space->size();

	state si, so;

	for (vi = global->begin(); vi != global->end(); vi++)
		si.assign(vi->second.uid, value("X"));

	if (init != -1)
		for (i = 0; i < (*space)[init].size(); i++)
			si.assign(i, (*space)[init][i]);

	so = solve(chp, global, tab, verbosity);

	space->push_back(si && so);
	if (init != -1)
		trans->insert_edge(init, uid);

	cout << tab << si << endl;

	return uid;
}

void guard::generate_prs()
{


	print_prs();
}

state solve(string raw,  map<string, variable> *vars, string tab, int verbosity)
{
	map<string, variable>::iterator vi;
	state outcomes;
	state a, b;
	int ai, bi;
	string::iterator i, j;
	value temp;
	int depth;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Solve: " << raw << endl;

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
			a = solve(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity);
			b = solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			outcomes = (a || b);

			if (verbosity >= VERB_PARSE)
				cout << tab << outcomes << endl;

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
			a = solve(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity);
			b = solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			outcomes = (a && b);

			if (verbosity >= VERB_PARSE)
				cout << tab << outcomes << endl;

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
			b = solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			for (bi = 0; bi != b.size(); bi++)
				outcomes.assign(bi, ~b[bi]);

			if (verbosity >= VERB_PARSE)
				cout << tab << outcomes << endl;

			return outcomes;
		}
	}

	unsigned long s = raw.find_first_of("(");
	unsigned long e = raw.find_last_of(")");
	if (s != raw.npos && e != raw.npos)
	{
		a = solve(raw.substr(s+1, e-s-1), vars, tab+"\t", verbosity);

		outcomes = a;

		if (verbosity >= VERB_PARSE)
			cout << tab << outcomes << endl;

		return outcomes;
	}

	vi = vars->find(raw);
	if (vi != vars->end())
		outcomes.assign(vi->second.uid, value("1"));

	if (verbosity >= VERB_PARSE)
		cout << tab << outcomes << endl;

	return outcomes;
}

