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

guard::guard(instruction *parent, string chp, variable_space *vars, flag_space *flags)
{
	this->_kind		= "guard";
	this->chp		= chp;
	this->flags 	= flags;
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
instruction *guard::duplicate(instruction *parent, variable_space *vars, map<string, string> convert)
{
	guard *instr;

	instr 				= new guard();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags		= flags;
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

	instr->chp = canonical(instr->chp, vars).print(vars);

	return instr;
}

void guard::expand_shortcuts()
{
}

void guard::parse()
{
	// TODO Expand multi-bit guard expressions using operators
	flags->inc();
	if (flags->log_base_hse())
		(*flags->log_file) << flags->tab << "Guard:\t" + chp << endl;
	flags->dec();
}

void guard::merge()
{
	chp = canonical(chp, vars).print(vars);
}

vector<int> guard::generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch)
{
	flags->inc();
	from = f;
	net = n;
	chp = canonical(chp, vars).print(vars);

	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Guard " << chp << endl;

	uid.push_back(net->insert_transition(from, logic(chp, vars), pbranch, cbranch, this));
	flags->dec();

	return uid;
}

void guard::insert_instr(int uid, int nid, instruction *instr)
{
}

void guard::print_hse(string t, ostream *fout)
{
	(*fout) << chp;
}
