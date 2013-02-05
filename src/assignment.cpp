/*
 * assignment.cpp
 *
 *  Created on: Jan 22, 2013
 *      Author: nbingham
 */

#include "assignment.h"

assignment::assignment()
{
	_kind = "assignment";
}

assignment::assignment(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity)
{
	this->_kind		= "assignment";
	this->chp		= chp;
	this->tab		= tab;
	this->verbosity = verbosity;
	this->global	= globals;
	this->label		= label;

	expand_shortcuts();
	parse(types);
}

assignment::~assignment()
{
	_kind = "assignment";
}

assignment &assignment::operator=(assignment a)
{
	this->uid		= a.uid;
	this->expr		= a.expr;
	this->chp		= a.chp;
	this->rules		= a.rules;
	this->global	= a.global;
	this->label		= a.label;
	this->tab		= a.tab;
	this->verbosity	= a.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *assignment::duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert, string tab, int verbosity)
{
	assignment *instr;

	instr 				= new assignment();
	instr->chp			= this->chp;
	instr->global		= globals;
	instr->label		= labels;
	instr->tab			= tab;
	instr->verbosity	= verbosity;
	instr->expr			= this->expr;

	map<string, string>::iterator i;
	list<pair<string, string> >::iterator j;
	size_t k;
	for (i = convert.begin(); i != convert.end(); i++)
	{
		while ((k = find_name(instr->chp, i->first)) != instr->chp.npos)
			instr->chp.replace(k, i->first.length(), i->second);
		for (j = instr->expr.begin(); j != instr->expr.end(); j++)
		{
			while ((k = find_name(j->first, i->first)) != j->first.npos)
				j->first.replace(k, i->first.length(), i->second);
			while ((k = find_name(j->second, i->first)) != j->second.npos)
				j->second.replace(k, i->first.length(), i->second);
		}
	}

	return instr;
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

void assignment::parse(map<string, keyword*> types)
{
	size_t middle;
	string left_raw, right_raw;
	size_t i, j;
	size_t k, l;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Assignment:\t" + chp << endl;

	// Identify that this instruction is an assign.
	if (chp.find(":=") != chp.npos)
	{
		// Separate the two operands (the variable to be assigned and the value to assign)
		middle = chp.find(":=");
		left_raw = chp.substr(0, middle);
		right_raw = chp.substr(middle+2);
		for (i = left_raw.find_first_of(","), j = right_raw.find_first_of(","), k = 0, l = 0; i != left_raw.npos && j != right_raw.npos; i = left_raw.find_first_of(",", i+1), j = right_raw.find_first_of(",", j+1))
		{
			expr.push_back(pair<string, string>(left_raw.substr(k, i-k), right_raw.substr(l, j-l)));
			k = i+1;
			l = j+1;
		}
		expr.push_back(pair<string, string>(left_raw.substr(k), right_raw.substr(l)));
	}
	// If all else fails, complain to the user.
	else
		cout << "Error: Instruction not handled: " << chp << endl;
}

int assignment::generate_states(state_space *space, graph *trans, int init)
{
	cout << tab << "Assignment " << chp << endl;

	map<string, variable>::iterator vi;
	list<pair<string, string> >::iterator ei;
	string search;
	state s;

	uid = space->size();

	// Set up the initial state
	s = (*space)[init];

	// Evaluate each expression
	for (ei = expr.begin(); ei != expr.end(); ei++)
	{
		vi = global->find(ei->first);

		if (vi != global->end())
			s.assign(vi->second.uid, evaluate(ei->second, global, s.values, tab, verbosity), value("?"));
		else
			cout << "Error: Undefined variable " << ei->first << "." << endl;

		// TODO make this search smarter
		// Search for this channel's other variables and X them out.
		search = ei->first.substr(0, ei->first.find_last_of("."));
		for (vi = global->begin(); vi != global->end(); vi++)
		{
			if (vi->first.substr(0, search.length()) == search && vi->first != ei->first)
			{
				s.assign(vi->second.uid, value("X"));
				//cout << vi->first << endl;
			}
		}
		//cout << search << endl;
	}

	cout << tab << s << endl;

	space->push_back(s);
	trans->insert_edge(init, uid, chp);

	return uid;
}

void assignment::generate_prs()
{


	print_prs();
}

instruction *expand_expression(string chp, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, string tab, int verbosity)
{
	assignment *a = new assignment(chp, types, global, label, tab, verbosity);
	list<pair<string, string> >::iterator i;

	for (i = a->expr.begin(); i != a->expr.end(); i++)
	{

	}

	return a;
}

instruction *decompose_expression(string chp, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, string tab, int verbosity)
{
	if (verbosity >= VERB_PARSE)
		cout << tab << "Decompose: " << chp << endl;

	typename map<string, variable>::iterator v;
	list<string> ops;
	list<string> ex;
	size_t p;

	p = find_first_of_l0(chp, "|");
	if (p != chp.npos)
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	p = find_first_of_l0(chp, "&");
	if (p != chp.npos)
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	ops.clear();
	ops.push_back("==");
	ops.push_back("~=");
	p = find_first_of_l0(chp, ops);
	if (p != chp.npos && chp.substr(p, 2) == "==")
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}
	else if (p != chp.npos && chp.substr(p, 2) == "!=")
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	ops.clear();
	ops.push_back("<=");
	ops.push_back(">=");
	p = find_first_of_l0(chp, ops);
	if (p != chp.npos && chp.substr(p, 2) == "<=")
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}
	else if (p != chp.npos && chp.substr(p, 2) == ">=")
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	ops.clear();
	ops.push_back("<");
	ops.push_back(">");
	ex.clear();
	ex.push_back(">>");
	ex.push_back("<<");
	ex.push_back("<=");
	ex.push_back(">=");
	p = find_first_of_l0(chp, ops, 0, ex);
	if (p != chp.npos && chp[p] == '<')
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}
	else if (p != chp.npos && chp[p] == '>')
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	ops.clear();
	ops.push_back("<<");
	ops.push_back(">>");
	p = find_first_of_l0(chp, ops);
	if (p != chp.npos && chp.substr(p, 2) == "<<")
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}
	else if (p != chp.npos && chp.substr(p, 2) == ">>")
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	p = find_first_of_l0(chp, "+-");
	if (p != chp.npos && chp[p] == '+')
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}
	else if (p != chp.npos && chp[p] == '-')
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	p = find_first_of_l0(chp, "*/");
	if (p != chp.npos && chp[p] == '*')
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}
	else if (p != chp.npos && chp[p] == '/')
	{
		decompose_expression(chp.substr(0, p-1), types, global, label, tab+"\t", verbosity);
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	p = find_first_of_l0(chp, "~");
	if (p != chp.npos)
	{
		decompose_expression(chp.substr(p), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	if (chp[0] == '(' && chp[chp.length()-1] == ')')
	{
		decompose_expression(chp.substr(1, chp.length()-2), types, global, label, tab+"\t", verbosity);
		return NULL;
	}

	return NULL;
}


