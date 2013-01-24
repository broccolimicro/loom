#include "assignment.h"

assignment::assignment()
{
	_kind = "assignment";
}

assignment::assignment(string uid, string chp, map<string, keyword*> *types, map<string, variable*> globals, string tab, int verbosity)
{
	this->_kind		= "assignment";
	this->uid		= uid;
	this->chp		= chp;
	this->tab		= tab;
	this->verbosity = verbosity;

	expand_shortcuts();
	parse(types);
}

assignment::~assignment()
{
	_kind = "assignment";
}

void assignment::expand_shortcuts()
{
	// Convert var+ to var:=1
	if(chp.find(":=") == chp.npos && chp.find("+") != chp.npos)
		chp = chp.substr(0, chp.find("+")) + ":=1";

	// Convert var- to var:=0
	if(chp.find(":=") == chp.npos && chp.find("-") != chp.npos)
		chp = chp.substr(0, chp.find("-")) + ":=0";
}

void assignment::parse(map<string, keyword*> *types)
{
	size_t middle;
	string left_raw, right_raw;
	size_t i, j;
	size_t k, l;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Assignment:\t" + chp << endl;

	// TODO we need to involve the defined operators here as well, not just solve for the state space.

	// Identify that this instruction is an assign.
	if (chp.find(":=") != chp.npos)
	{
		// Separate the two operands (the variable to be assigned and the value to assign)
		middle = chp.find(":=");
		left_raw = chp.substr(0, middle);
		right_raw = chp.substr(middle+2);
		for (i = left_raw.find_first_of(","), j = left_raw.find_first_of(","), k = 0, l = 0; i != left_raw.npos && j != right_raw.npos; i = left_raw.find_first_of(",", i+1), j = right_raw.find_first_of(",", j+1))
		{
			expr.insert(pair<string, string>(left_raw.substr(k, i-k), right_raw.substr(l, j-l)));
			k = i;
			l = j;
		}
		expr.insert(pair<string, string>(left_raw.substr(k), right_raw.substr(l)));
	}
	// Parse skip
	else if (chp.find("skip") != chp.npos)
		return;
	// If all else fails, complain to the user.
	else
		cout << "Error: Instruction not handled: " << chp << endl;
}

void assignment::generate_states(map<string, state> init)
{
	map<string, space>::iterator space_iter;
	map<string, state>::iterator state_iter;
	map<string, string>::iterator expr_iter;


	for (state_iter = init.begin(); state_iter != init.end(); state_iter++)
	{
		states.insert(pair<string, space>(state_iter->first, space()));
		states.rbegin()->second->states.push_back(*(state_iter->second));
		states.rbegin()->second->states.push_back(*(state_iter->second));
	}

	for (expr_iter = expr.begin(); expr_iter != expr.end(); expr_iter++)
	{
		space_iter = states.find(expr_iter->first);
		*(space_iter.second->states.rbegin()) = expression(expr_iter->second, init, tab, verbosity);
	}

	print_state_space();
}

void assignment::generate_prs(map<string, variable*> globals)
{
	map<string, space>::iterator si, sj;
	int bi0, bi1;
	rule r;
	state s0, s1;
	bool first = true;

	for (si = states.begin(); si != states.end(); si++)
	{
		// Expand multibit variables into their single bit constituents
		for (bi0 = 0; bi0 < globals.find(si->first)->second->width && si->second->states.front().prs; bi0++)
		{
			s0 = si->second->states.front()[bi0];

			r.clear(0);
			r.right.var = si->first + (globals.find(si->first)->second->width > 1 ? "[" + to_string(bi0) + "]" : "") + (s0.data == "1" ? "+" : "-");
			first = true;
			for (sj = states.begin(); sj != states.end(); sj++)
			{
				// Expand multibit variables into their single bit constituents
				for (bi1 = 0; bi1 < globals.find(sj->first)->second->width && si->first != sj->first; bi1++)
				{
					s1 = sj->second->states.back()[bi1];

					if (s1.data != "X")
					{
						r.left.var = (s1.data == "0" ? "~" : "") + sj->first + (globals.find(sj->first)->second->width > 1 ? "[" + to_string(bi1) + "]" : "") + (first ? "" : "&") + r.left.var;
						first = false;
					}
				}
			}

			rules.push_back(r);
		}
	}

	print_prs();
}
