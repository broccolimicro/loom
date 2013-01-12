/*
 * instruction.cpp
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "instruction.h"
#include "common.h"

instruction::instruction()
{
	_kind = "instruction";
}
instruction::instruction(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity)
{
	_kind = "instruction";
	parse(id, raw, types, vars, init, tab, verbosity);
}
instruction::~instruction()
{
	_kind = "instruction";
}

instruction &instruction::operator=(instruction i)
{
	chp = i.chp;
	result = i.result;
	return *this;
}

string instruction::kind()
{
	return _kind;
}

void instruction::parse(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity)
{
	result.clear();

	chp = raw;
	uid = id;

	string::iterator i;
	size_t j, k;
	size_t name_end;

	string temp;
	state tstate;

	list<string> var;
	list<string>::iterator var_iter;
	list<state> val;
	list<state>::iterator val_iter;

	map<string, state>::iterator state_iter;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Instruction:\t"+chp << endl;

	//Convert var+ to var:=1
	if(chp.find(":=") == chp.npos && chp.find("+") != chp.npos)
		chp = chp.substr(0,chp.find("+")) + ":=1";

	//Convert var- to var:=0
	if(chp.find(":=") == chp.npos && chp.find("-") != chp.npos)
		chp = chp.substr(0,chp.find("-")) + ":=0";

	// Identify that this instruction is an assign.
	// Currently only handles single variable assignments
	// TODO we need to involve the defined operators here as well, not just solve for the state space.
	if (chp.find(":=") != chp.npos)
	{
		// Separate the two operands (the variable to be assigned and the value to assign)
		name_end = chp.find(":=");
		temp = chp.substr(0,name_end);
		for (j = temp.find_first_of(","), k = 0; j != temp.npos; j = temp.find_first_of(",", j+1))
		{
			var.push_back(temp.substr(k, j-k));
			k = j;
		}
		var.push_back(temp.substr(k));

		temp = chp.substr(name_end+2);
		for (j = temp.find_first_of(","), k = 0; j != temp.npos; j = temp.find_first_of(",", j+1))
		{
			val.push_back(expression(temp.substr(k, j-k), init, tab + "\t", verbosity));
			k = j;
		}
		val.push_back(expression(temp.substr(k), init, tab + "\t", verbosity));

		for (var_iter = var.begin(), val_iter = val.begin(); var_iter != var.end() && val_iter != val.end(); var_iter++, val_iter++)
			result.insert(pair<string, state>(*var_iter, *val_iter));
	}
	// Parse skip
	else if (chp.find("skip") != chp.npos)
		return;
	// If all else fails, complain to the user.
	else
		cout << "Error: Instruction not handled: " << chp << endl;

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "Result:\t";

		for (state_iter = result.begin(); state_iter != result.end(); state_iter++)
			cout << "{" << state_iter->first << " = " << state_iter->second << "} ";
		cout << endl;
	}

	rules = production_rule(init, result, vars, tab+"\t", verbosity);
}

list<rule> production_rule(map<string, state> previous, map<string, state> next, map<string, variable*> globals, string tab, int verbosity)
{
	list<rule> prs;
	map<string, state>::iterator si, sj;
	int bi0, bi1;
	rule r;
	state s0, s1;
	bool first = true;

	if (verbosity >= VERB_PRSALG)
		cout << tab << "Production Rules" << endl;

	for (si = next.begin(); si != next.end(); si++)
	{
		// Expand multibit variables into their single bit constituents
		for (bi0 = 0; bi0 < globals.find(si->first)->second->width && si->second.prs; bi0++)
		{
			s0 = si->second[bi0];

			r.clear(0);
			r.right.var = si->first + (globals.find(si->first)->second->width > 1 ? "[" + to_string(bi0) + "]" : "") + (s0.data == "1" ? "+" : "-");
			first = true;
			for (sj = previous.begin(); sj != previous.end(); sj++)
			{
				// Expand multibit variables into their single bit constituents
				for (bi1 = 0; bi1 < globals.find(sj->first)->second->width && si->first != sj->first; bi1++)
				{
					s1 = sj->second[bi1];

					if (s1.data != "X")
					{
						r.left.var = (s1.data == "0" ? "~" : "") + sj->first + (globals.find(sj->first)->second->width > 1 ? "[" + to_string(bi1) + "]" : "") + (first ? "" : "&") + r.left.var;
						first = false;
					}
				}
			}
			if (verbosity >= VERB_PRSALG)
				cout << tab+"\t" << r << endl;
			prs.push_back(r);
		}
	}

	return prs;
}
