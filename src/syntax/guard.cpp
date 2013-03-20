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

state guard::active_variant()
{
	return state();
}

state guard::passive_variant()
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
	/* TODO If a variable in a guard has a definite value in the previous state (not 'X'), then what do we do?
	 * Choice 1: replace their occurrence in the guard with their current, constant value
	 * Choice 2: X them out in the state before
	 * Choice 3: Set them such that the guard evaluates to false in the state before
	 * Choice 4: Do choice 1, simplify the expression, then do choice 2
	 * Choice 5: Do choice 1, simplify the expression, then do choice 3
	 *
	 * Important cases to consider are:
	 * a,b := 1,2;
	 * c := a+b		<-- this has variables a and b in a guard right after they are assigned. I suggest choice 3
	 *
	 * chan4p0b a
	 * a.r := 1;
	 * a?			<-- This might happen in the HSE due to HSE optimizations... I suggest choice 3
	 *
	 * After executing choice 1, you will be left with a bunch impossible states (underscores). These
	 * represent the branches of conditionals that will never happen. Since this state space represents
	 * the union of all execution paths and values, we can say that for sure. This could be a huge optimization,
	 * removing a bunch of hardware that will always run the same way.
	 */

	space = g;
	from = init;
	cout << tab << "Guard " << chp << endl;

	map<string, variable>::iterator vi;
	state s, temp;

	// Choice 1
	/*cout << "CHP Replace " << endl;
	cout << chp << endl;
	size_t k = 0, curr;
	string vname;
	temp = estimate(chp, vars);
	for (int i = 0; i < temp.size(); i++)
	{
		if (temp[i].data == "X" && g->states[init][i].data != "X")
		{
			vname = vars->get_name(i);
			k = 0;
			while (k < chp.length())
			{
				curr = find_name(chp, vname, k);
				if (curr == chp.npos)
					break;
				chp.replace(curr, vname.length(), g->states[init][i].data);
				k = curr + g->states[init][i].data.length();
			}
		}
	}
	cout << chp << endl;
	chp = expression(chp).simple;
	cout << chp << endl;*/

	// Choice 2
	bool prs = g->states[init].prs;
	int tag = g->states[init].tag;
	g->states[init] = g->states[init] || estimate(chp, vars);
	g->states[init].prs = prs;
	g->states[init].tag = tag;

	/*// Choice 3
	bool prs = g->states[init].prs;
	int tag = g->states[init].tag;
	g->states[init] = g->states[init] && solve(expression("~(" + chp + ")").simple, vars, "", -1);
	g->states[init].prs = prs;
	g->states[init].tag = tag;*/

	uid = g->states.size();
	solution = solve(chp, vars, tab, verbosity);
	cout << solution << endl;
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
		int vi = vars->insert(variable("("+chp+")", "int", 1, false));

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
