#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "guard.h"

guard::guard()
{
	_kind = "guard";
}

guard::guard(string chp, map<string, keyword*> types, vspace *vars, string tab, int verbosity)
{
	this->_kind		= "guard";
	this->chp		= chp;
	this->tab		= tab;
	this->verbosity = verbosity;
	this->vars		= vars;

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
	this->vars		= g.vars;
	this->tab		= g.tab;
	this->verbosity	= g.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *guard::duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	guard *instr;

	instr 				= new guard();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;

	map<string, string>::iterator i, j;
	size_t k = 0, min, curr;
	while (k != instr->chp.npos)
	{
		j = convert.end();
		min = instr->chp.length();
		curr = 0;
		for (i = convert.begin(); i != convert.end(); i++)
		{
			curr = find_name(instr->chp, i->first, k);
			if (curr < min)
			{
				min = curr;
				j = i;
			}
		}

		if (j != convert.end())
		{
			instr->chp.replace(min, j->first.length(), j->second);
			k = min + j->second.length();
		}
		else
			k = instr->chp.npos;
	}

	instr->chp = strip(demorgan(instr->chp, false));

	return instr;
}

void guard::expand_shortcuts()
{
}

void guard::parse(map<string, keyword*> types)
{
	chp = strip(demorgan(chp, false));
	if (verbosity >= VERB_PARSE)
		cout << tab << "Guard:\t" + chp << endl;
}

int guard::generate_states(state_space *space, graph *trans, int init)
{
	cout << tab << "Guard " << chp << endl;

	map<string, variable>::iterator vi;
	state si, so;

	uid = space->size();
	si = (*space)[init];
	so = solve(chp, vars, tab, verbosity);
	space->push_back(si && so);
	if (CHP_EDGE)
		trans->insert_edge(init, uid, chp+"->");
	else
		trans->insert_edge(init, uid, "Guard");
	return uid;
}

void guard::generate_prs()
{


	print_prs();
}

state solve(string raw, vspace *vars, string tab, int verbosity)
{
	int id;
	state outcomes;
	string::iterator i, j;
	int depth;

	if (vars->global.size() != 0)
		outcomes.assign(vars->global.size()-1, value("?"), value("?"));

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
			outcomes = ~solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

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

	id = vars->get_uid(raw);
	if (id != -1)
		outcomes.assign(id, value("1"), value("?"));
	else if (raw == "1")
		for (map<string, variable>::iterator vi = vars->global.begin(); vi != vars->global.end(); vi++)
			outcomes.assign(vi->second.uid, value("X"), value("?"));
	else
		cout << "Error: Undefined variable " << raw << "." << endl;

	if (verbosity >= VERB_PARSE)
		cout << tab << outcomes << endl;

	return outcomes;
}

void guard::print_hse()
{
	cout << chp;
}
