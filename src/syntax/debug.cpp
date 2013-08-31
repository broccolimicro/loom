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

debug::debug(instruction *parent, string chp, variable_space *vars, flag_space *flags)
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

instruction *debug::duplicate(instruction *parent, variable_space *vars, map<string, string> convert)
{
	debug *instr;

	instr 				= new debug();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags		= flags;
	instr->parent		= parent;
	instr->type			= type;

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

void debug::expand_shortcuts()
{
	if (chp[0] == '{')
		chp = "assert" + chp;
}

void debug::parse()
{
	size_t open = chp.find_first_of("{");
	size_t close = chp.find_first_of("}");

	type = chp.substr(0, open);
	chp = chp.substr(open+1, close - open - 1);

	if (type != "assert" && type != "require" && type != "assume" && type != "enforce")
		cout << "Error: illegal debug function " << type << "." << endl;
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

vector<int> debug::generate_states(petri *n, rule_space *p, vector<int> f, map<int, int> pbranch, map<int, int> cbranch)
{
	net = n;
	prs = p;
	from = f;

	if (type == "assert")
		for (int i = 0; i < (int)f.size(); i++)
			(*n)[f[i]].assertions.push_back(logic("~(" + chp + ")", vars));
	else if (type == "require")
		vars->requirements.push_back(logic("~(" + chp + ")", vars));
	else if (type == "assume")
		for (int i = 0; i < (int)f.size(); i++)
			(*n)[f[i]].assumptions = (*n)[f[i]].assumptions >> logic(chp, vars);
	else if (type == "enforce")
		vars->enforcements = vars->enforcements >> logic(chp, vars);

	return f;
}

void debug::generate_class_requirements()
{
	if (type == "require")
		vars->requirements.push_back(logic("~(" + chp + ")", vars));
	else if (type == "enforce")
		vars->enforcements = vars->enforcements >> logic(chp, vars);
	else
		cout << "Error: Illegal debug function type " << type << "." << endl;
}

void debug::print_hse(string t, ostream *fout)
{
	(*fout) << type << "{" << chp << "}";
}
