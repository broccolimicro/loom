/*
 * assignment.cpp
 *
 *  Created on: Jan 22, 2013
 *      Author: nbingham
 */

#include "assignment.h"
#include "operator.h"
#include "record.h"
#include "utility.h"

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
		while ((k = find_name(instr->chp, i->first, k+1)) != instr->chp.npos)
			instr->chp.replace(k, i->first.length(), i->second);
		for (j = instr->expr.begin(); j != instr->expr.end(); j++)
		{
			while ((k = find_name(j->first, i->first, k+1)) != j->first.npos)
				j->first.replace(k, i->first.length(), i->second);
			while ((k = find_name(j->second, i->first, k+1)) != j->second.npos)
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

		// TODO Make this search smarter
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
	if(CHP_EDGE)
		trans->insert_edge(init, uid, chp);
	else
		trans->insert_edge(init, uid, "Assign");

	return uid;
}

void assignment::generate_prs()
{


	print_prs();
}

instruction *expand_assignment(string chp, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, string tab, int verbosity)
{
	assignment *a = new assignment(chp, types, global, label, tab, verbosity);
	parallel *p = new parallel("", types, global, label, tab, verbosity);;
	pair<string, instruction*> result;
	list<pair<string, string> >::iterator i;

	for (i = a->expr.begin(); i != a->expr.end(); i++)
	{
		cout << i->first << " " << i->second << endl;
		if (i->second.find_first_of("|&=<>/+-*~?@") != i->second.npos)
		{
			result = expand_expression(i->second, types, global, label, tab, verbosity);
			i->second = result.first;
			p->push(result.second);

			if (get_kind(i->second, global, label, types) == "channel")
			{
				result = expand_expression(i->second+"?", types, global, label, tab, verbosity);
				i->second = result.first;
				p->push(result.second);
			}
		}
	}

	if (p->instrs.size() == 0)
	{
		delete p;
		return a;
	}

	p->push(a);

	return p;
}

pair<string, instruction*> expand_expression(string chp, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, string tab, int verbosity)
{
	if (verbosity >= VERB_PARSE)
		cout << tab << "Decompose: " << chp << endl;

	map<string, variable>::iterator v;
	map<string, keyword*>::iterator k;
	map<string, string>::iterator c;
	list<string>::iterator s;
	list<instruction*>::iterator i;
	list<string> ops;
	list<string> ex;

	string left, right, op = "";
	operate *proc;

	pair<string, instruction*> A, B, C;

	size_t p;

	if (op == "")
	{
		p = find_first_of_l0(chp, "|");
		if (p != chp.npos)
		{
			op = "|";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(chp, "&");
		if (p != chp.npos)
		{
			op = "&";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
		ops.clear();
		ops.push_back("==");
		ops.push_back("~=");
		p = find_first_of_l0(chp, ops);
		if (p != chp.npos && chp.substr(p, 2) == "==")
		{
			op = "==";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
		else if (p != chp.npos && chp.substr(p, 2) == "!=")
		{
			op = "!=";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
		ops.clear();
		ops.push_back("<=");
		ops.push_back(">=");
		p = find_first_of_l0(chp, ops);
		if (p != chp.npos && chp.substr(p, 2) == "<=")
		{
			op = "<=";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
		else if (p != chp.npos && chp.substr(p, 2) == ">=")
		{
			op = ">=";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
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
			op = "<";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
		else if (p != chp.npos && chp[p] == '>')
		{
			op = ">";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
		ops.clear();
		ops.push_back("<<");
		ops.push_back(">>");
		p = find_first_of_l0(chp, ops);
		if (p != chp.npos && chp.substr(p, 2) == "<<")
		{
			op = "<<";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
		else if (p != chp.npos && chp.substr(p, 2) == ">>")
		{
			op = ">>";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(chp, "+-");
		if (p != chp.npos && chp[p] == '+')
		{
			op = "+";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
		else if (p != chp.npos && chp[p] == '-')
		{
			op = "-";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(chp, "*/");
		if (p != chp.npos && chp[p] == '*')
		{
			op = "*";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
		else if (p != chp.npos && chp[p] == '/')
		{
			op = "/";
			left = chp.substr(0, p);
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(chp, "~");
		if (p != chp.npos)
		{
			op = "~";
			left = "";
			right = chp.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(chp, "?");
		if (p != chp.npos)
		{
			op = "?";
			left = chp.substr(0, p);
			right = "";
		}
	}

	cout << p << " " << left << " " << right << " " << chp << endl;

	if (chp[0] == '(' && chp[chp.length()-1] == ')' && op == "")
		return expand_expression(chp.substr(1, chp.length()-2), types, global, label, tab+"\t", verbosity);

	k = types.find("operator" + op);
	if (k == types.end())
	{
		cout << "Error: Undefined operator " << op << " used in " << chp << "." << endl;
		return pair<string, instruction*>("", NULL);
	}

	proc = (operate*)k->second;

	A = pair<string, instruction*>("", NULL);
	B = pair<string, instruction*>("", NULL);
	if (left.find_first_of("|&=<>/+-*~?!@") != left.npos)
		A = expand_expression(left, types, global, label, tab+"\t", verbosity);

	if (right.find_first_of("|&=<>/+-*~?!@") != right.npos)
		B = expand_expression(right, types, global, label, tab+"\t", verbosity);

	map<string, string> convert;
	parallel* ret;
	for (s = proc->input.begin(); s != proc->input.end() && s->find_first_of(".") != s->npos; s++);
	if (s != proc->input.end())
	{
		v = proc->label.find(*s);
		if (v == proc->label.end())
			v = proc->global.find(*s);

		C = add_unique_variable("_op", "", v->second.type, types, global, label, tab+"\t", verbosity);
		convert.insert(pair<string, string>(*s, C.first));
		cout << *s << " " << C.first << endl;
	}
	if (left != "")
	{
		for (s++; s != proc->input.end() && s->find_first_of(".") != s->npos; s++);
		if (s != proc->input.end())
			convert.insert(pair<string, string>(*s, A.second == NULL ? left : A.first));
		cout << *s << " " << (A.second == NULL ? left : A.first) << endl;
	}
	if (right != "")
	{
		for (s++; s != proc->input.end() && s->find_first_of(".") != s->npos; s++);
		if (s != proc->input.end())
			convert.insert(pair<string, string>(*s, B.second == NULL ? right : B.first));
		cout << *s << " " << (B.second == NULL ? right : B.first) << endl;
	}

	int id = 0;
	while (global->find("_fn" + to_string(id)) != global->end() || label->find("_fn" + to_string(id)) != label->end())
		id++;

	string dec = "operator" + op + " " + "_fn" + to_string(id) + "(";

	for (c = convert.begin(); c != convert.end(); c++)
	{
		if (c != convert.begin())
			dec += ",";
		dec += c->second;
		cout << c->first << " -> " << c->second << endl;
	}

	dec += ")";
	cout << dec << endl;

	ret = (parallel*)expand_instantiation(dec, types, global, label, NULL, tab, verbosity, true);

	ret->push(A.second);
	ret->push(B.second);

	return pair<string, instruction*>(C.first, ret);
}

void assignment::print_hse()
{
	if (expr.size() == 1 && expr.begin()->second == "0")
		cout << expr.begin()->first << "-";
	else if (expr.size() == 1 && expr.begin()->second == "1")
		cout << expr.begin()->first << "+";
	else
	{
		list<pair<string, string> >::iterator i;
		for (i = expr.begin(); i != expr.end(); i++)
		{
			if (i != expr.begin())
				cout << ",";
			cout << i->first;
		}

		cout << ":=";

		for (i = expr.begin(); i != expr.end(); i++)
		{
			if (i != expr.begin())
				cout << ",";
			cout << i->second;
		}
	}
}
