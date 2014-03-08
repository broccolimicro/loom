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

guard::guard(instruction *parent, sstring chp, variable_space *vars, flag_space *flags)
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
instruction *guard::duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert)
{
	guard *instr;

	instr 				= new guard();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags		= flags;
	instr->parent		= parent;

	int idx;
	sstring rep;

	smap<sstring, sstring>::iterator i, j;
	int k = 0, min, curr;
	while (k != instr->chp.npos)
	{
		j = convert.end();
		min = instr->chp.length();
		curr = 0;
		for (i = convert.begin(); i != convert.end(); i++)
		{
			curr = find_name(instr->chp, i->first, k);
			if (curr < min && curr >= 0)
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

	instr->solution = canonical(instr->chp, vars);
	instr->chp = instr->solution.print(vars);

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

void guard::simulate()
{

	vars->increment_pcs(canonical(chp, vars), false);
}

void guard::rewrite()
{
	chp = canonical(chp, vars).print(vars);
}

void guard::reorder()
{

}

svector<petri_index> guard::generate_states(petri_net *n, rule_space *p, svector<petri_index> f, smap<int, int> pbranch, smap<int, int> cbranch)
{
	flags->inc();
	from = f;
	net = n;
	prs = p;
	solution = canonical(chp, vars);

	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Guard " << chp << endl;

	uid.push_back(net->push_transition(from, solution, false, pbranch, cbranch, this));
	flags->dec();

	return uid;
}

void guard::print_hse(sstring t, ostream *fout)
{
	(*fout) << chp;
}
