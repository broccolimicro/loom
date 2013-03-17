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

guard &guard::operator=(guard g)
{
	this->uid		= g.uid;
	this->chp		= g.chp;
	this->vars		= g.vars;
	this->space		= g.space;
	this->tab		= g.tab;
	this->verbosity	= g.verbosity;
	this->parent	= g.parent;
	return *this;
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

	instr->chp = expression(instr->chp).simple;

	return instr;
}

state guard::variant()
{
	return estimate(chp, vars);
}

void guard::expand_shortcuts()
{
}

void guard::parse()
{
	// TODO Expand multi-bit guard expressions using operators
	chp = expression(chp).simple;
	if (verbosity >= VERB_PARSE)
		cout << tab << "Guard:\t" + chp << endl;
}

int guard::generate_states(graph *g, int init, state filter)
{
	space = g;
	from = init;
	cout << tab << "Guard " << chp << endl;

	map<string, variable>::iterator vi;
	state s, temp;

	bool prs = g->states[init].prs;
	int tag = g->states[init].tag;
	g->states[init] = g->states[init] || estimate("~(" + chp + ")", vars);
	g->states[init].prs = prs;
	g->states[init].tag = tag;

	uid = g->states.size();
	solution = solve(chp, vars, tab, verbosity);
	s = (g->states[init] || filter) && solution;

	if(CHP_EDGE)
		g->append_state(s, init, chp + "->");
	else
		g->append_state(s, init, "Guard");

	return uid;
}

void guard::generate_scribes()
{
	if (chp.find_first_of("|") != chp.npos)
	{
		int vi = vars->insert(variable("("+chp+")", "int", value("X"), 1, false));

		if (vi != -1)
			space->traces.push_back(evaluate(chp, vars, space->traces.traces));
		else
			vi = vars->get_uid("("+chp+")");

		space->traces[vi][uid] = value("1");
		if (from != -1)
			space->traces[vi][from] = value("0");

		for (int i = 0; i < space->traces[vi].size(); i++)
			space->states[i].assign(vi, space->traces[vi][i], value("X"));
	}
}

void guard::print_hse()
{
	cout << chp;
}
