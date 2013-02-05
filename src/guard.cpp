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

guard &guard::operator=(guard g)
{
	this->uid		= g.uid;
	this->chp		= g.chp;
	this->rules		= g.rules;
	this->global	= g.global;
	this->label		= g.label;
	this->tab		= g.tab;
	this->verbosity	= g.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *guard::duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert, string tab, int verbosity)
{
	guard *instr;

	instr 				= new guard();
	instr->chp			= this->chp;
	instr->global		= globals;
	instr->label		= labels;
	instr->tab			= tab;
	instr->verbosity	= verbosity;

	map<string, string>::iterator i;
	size_t k;
	for (i = convert.begin(); i != convert.end(); i++)
		while ((k = find_name(instr->chp, i->first)) != instr->chp.npos)
			instr->chp.replace(k, i->first.length(), i->second);

	return instr;
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
	state si, so;
	int i;

	cout << tab << "Guard " << chp << endl;

	uid = space->size();

	si = (*space)[init];

	so = solve(chp, global, tab, verbosity);

	space->push_back(si && so);
	trans->insert_edge(init, uid, chp+"->");

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
	string::iterator i, j;
	int depth;

	if (vars->size() != 0)
		outcomes.assign(vars->rbegin()->second.uid, value("?"), value("?"));

	if (verbosity >= VERB_PARSE)
		cout << tab << "Solve: " << raw << endl;

	depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && *i == '|')
		{
			outcomes = (solve(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity) ||
						solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity));

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
			outcomes = (solve(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity) &&
						solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity));

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
			outcomes = !solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			if (verbosity >= VERB_PARSE)
				cout << tab << outcomes << endl;

			return outcomes;
		}
	}

	unsigned long s = raw.find_first_of("(");
	unsigned long e = raw.find_last_of(")");
	if (s != raw.npos && e != raw.npos)
	{
		outcomes = solve(raw.substr(s+1, e-s-1), vars, tab+"\t", verbosity);

		if (verbosity >= VERB_PARSE)
			cout << tab << outcomes << endl;

		return outcomes;
	}

	vi = vars->find(raw);
	if (vi != vars->end())
		outcomes.assign(vi->second.uid, value("1"), value("?"));
	else if (raw == "1")
		for (vi = vars->begin(); vi != vars->end(); vi++)
			outcomes.assign(vi->second.uid, value("X"), value("?"));
	else
		cout << "Error: Undefined variable " << raw << "." << endl;

	if (verbosity >= VERB_PARSE)
		cout << tab << outcomes << endl;

	return outcomes;
}

