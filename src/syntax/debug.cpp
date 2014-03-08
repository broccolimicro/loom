/*
 * debug.cpp
 *
 *  Created on: Aug 13, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "debug.h"

debug::debug()
{
	_kind = "debug";
}

debug::debug(instruction *parent, sstring chp, variable_space *vars, flag_space *flags)
{
	_kind = "debug";

	this->chp = chp;
	this->flags = flags;
	this->vars = vars;
	this->parent = parent;

	expand_shortcuts();
	parse();
}

debug::~debug()
{
	_kind = "debug";
}

instruction *debug::duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert)
{
	debug *instr;

	instr 				= new debug();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags		= flags;
	instr->parent		= parent;
	instr->type			= type;

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

	instr->chp = canonical(instr->chp, vars).print(vars);

	return instr;
}

void debug::expand_shortcuts()
{
	if (chp[0] == '{')
		chp = "assert" + chp;
}

void debug::parse()
{
	int open = chp.find_first_of("{");
	int close = chp.find_first_of("}");

	type = chp.substr(0, open);
	chp = chp.substr(open+1, close - open - 1);

	if (type != "assert" && type != "require" && type != "assume" && type != "enforce")
		cerr << "Error: illegal debug function " << type << "." << endl;
}

void debug::simulate()
{

}

void debug::rewrite()
{

}

void debug::reorder()
{

}

svector<petri_index> debug::generate_states(petri_net *n, rule_space *p, svector<petri_index> f, smap<int, int> pbranch, smap<int, int> cbranch)
{
	net = n;
	prs = p;
	from = f;

	if (type == "assert")
		for (int i = 0; i < (int)f.size(); i++)
			(*n)[f[i]].assertions.push_back(canonical("~(" + chp + ")", vars));
	else if (type == "require")
		vars->requirements.push_back(canonical("~(" + chp + ")", vars));
	else if (type == "assume")
		for (int i = 0; i < (int)f.size(); i++)
			(*n)[f[i]].assumptions = (*n)[f[i]].assumptions >> canonical(chp, vars);
	else if (type == "enforce")
		vars->enforcements = vars->enforcements >> canonical(chp, vars);

	return f;
}

void debug::generate_class_requirements()
{
	if (type == "require")
		vars->requirements.push_back(canonical("~(" + chp + ")", vars));
	else if (type == "enforce")
		vars->enforcements = vars->enforcements >> canonical(chp, vars);
	else
		cerr << "Error: Illegal debug function type " << type << "." << endl;
}

void debug::print_hse(sstring t, ostream *fout)
{
	(*fout) << type << "{" << chp << "}";
}
