#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "guard.h"
#include "assignment.h"

guard::guard()
{
	_kind = "guard";
}

guard::guard(instruction *parent, string chp, vspace *vars, string tab, int verbosity)
{
	this->_kind		= "guard";
	this->chp		= chp;
	this->tab		= tab;
	this->verbosity = verbosity;
	this->vars		= vars;
	this->parent	= parent;

	expand_shortcuts();
	parse();
}

guard::~guard()
{
	_kind = "guard";
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *guard::duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	guard *instr;

	instr 				= new guard();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;
	instr->parent		= parent;

	size_t idx;
	string rep;

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
			rep = j->second;
			instr->chp.replace(min, j->first.length(), rep);
			if (instr->chp[min + rep.length()] == '[' && instr->chp[min + rep.length()-1] == ']')
			{
				idx = instr->chp.find_first_of("]", min + rep.length()) + 1;
				rep = flatten_slice(instr->chp.substr(min, idx - min));
				instr->chp.replace(min, idx - min, rep);
			}

			k = min + rep.length();
		}
		else
			k = instr->chp.npos;
	}

	instr->chp = canonical(instr->chp, vars).print(vars->get_names());

	return instr;
}

vector<int> guard::variant()
{
	return extract_ids(chp, vars);
}

vector<int> guard::active_variant()
{
	return vector<int>();
}

vector<int> guard::passive_variant()
{
	return extract_ids(chp, vars);
}

void guard::expand_shortcuts()
{
}

void guard::parse()
{
	// TODO Expand multi-bit guard expressions using operators
	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
		cout << tab << "Guard:\t" + chp << endl;
}

void guard::merge()
{
	chp = canonical(chp, vars).print(vars->get_names());
}

vector<int> guard::generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch)
{
	from = f;
	net = n;
	chp = canonical(chp, vars).print(vars->get_names());

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << "Guard " << chp << endl;

	uid.push_back(net->insert_transition(from, net->values.build(chp, vars), pbranch, cbranch, this));

	return uid;
}

void guard::insert_instr(int uid, int nid, instruction *instr)
{
}

void guard::print_hse(string t, ostream *fout)
{
	(*fout) << chp;
}
