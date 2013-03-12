/*
 * assignment.cpp
 *
 *  Created on: Jan 22, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "assignment.h"

assignment::assignment()
{
	_kind = "assignment";
}

assignment::assignment(string chp, vspace *vars, string tab, int verbosity)
{
	this->_kind		= "assignment";
	this->chp		= chp;
	this->tab		= tab;
	this->verbosity = verbosity;
	this->vars		= vars;

	expand_shortcuts();
	parse();
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
	this->vars		= a.vars;
	this->tab		= a.tab;
	this->verbosity	= a.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *assignment::duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	assignment *instr;

	instr 				= new assignment();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;
	instr->expr			= this->expr;

	size_t idx;
	string rep;

	map<string, string>::iterator i, j;
	list<pair<string, string> >::iterator e;
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

	for (e = instr->expr.begin(); e != instr->expr.end(); e++)
	{
		k = 0;
		while (k != e->first.npos)
		{
			j = convert.end();
			min = e->first.length();
			curr = 0;
			for (i = convert.begin(); i != convert.end(); i++)
			{
				curr = find_name(e->first, i->first, k);
				if (curr < min)
				{
					min = curr;
					j = i;
				}
			}

			if (j != convert.end())
			{
				rep = j->second;
				e->first.replace(min, j->first.length(), rep);
				if (e->first[min + rep.length()] == '[' && e->first[min + rep.length()-1] == ']')
				{
					idx = e->first.find_first_of("]", min + rep.length()) + 1;
					rep = flatten_slice(e->first.substr(min, idx - min));
					e->first.replace(min, idx - min, rep);
				}

				k = min + rep.length();
			}
			else
				k = e->first.npos;
		}

		k = 0;
		while (k != e->second.npos)
		{
			j = convert.end();
			min = e->second.length();
			curr = 0;
			for (i = convert.begin(); i != convert.end(); i++)
			{
				curr = find_name(e->second, i->first, k);
				if (curr < min)
				{
					min = curr;
					j = i;
				}
			}

			if (j != convert.end())
			{
				rep = j->second;
				e->second.replace(min, j->first.length(), rep);
				if (e->second[min + rep.length()] == '[' && e->second[min + rep.length()-1] == ']')
				{
					idx = e->second.find_first_of("]", min + rep.length()) + 1;
					rep = flatten_slice(e->second.substr(min, idx - min));
					e->second.replace(min, idx - min, rep);
				}

				k = min + rep.length();
			}
			else
				k = e->second.npos;
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

void assignment::parse()
{
	size_t middle;
	string left_raw, right_raw;
	size_t i, j;
	size_t k, l;
	string left, right;
	variable *v;

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
			left = left_raw.substr(k, i-k);
			right = right_raw.substr(l, j-l);
			expr.push_back(pair<string, string>(left, right));
			k = i+1;
			l = j+1;

			v = vars->find(left);
			if (v != NULL)
				v->driven = true;
		}
		left = left_raw.substr(k);
		right = right_raw.substr(l);
		expr.push_back(pair<string, string>(left, right));

		v = vars->find(left);
		if (v != NULL)
			v->driven = true;
	}
	// If all else fails, complain to the user.
	else
		cout << "Error: Instruction not handled: " << chp << endl;
}

int assignment::generate_states(graph *g, int init)
{
	space = g;
	from = init;
	cout << tab << "Assignment " << chp << endl;

	variable *v;
	map<string, variable>::iterator vi;
	list<pair<string, string> >::iterator ei;
	string search;
	state s;

	uid = g->states.size();

	// Set up the initial state
	s = g->states[init];
	s.prs = true;
	/*for(ei = expr.begin(); ei != expr.end(); ei++)
	{
		cout << ei->first << endl;
		vars->global[ei->first].prs = true;  //SET THE VAR AT THIS LOCATION TO PRS 1
	}*/
	// Evaluate each expression
	for (ei = expr.begin(); ei != expr.end(); ei++)
	{
		v = vars->find(ei->first);

		if (v != NULL)
		{
			s.assign(v->uid, evaluate(ei->second, vars, s.values, tab, verbosity), value("?"));

			// TODO Make this search smarter
			// Search for this channel's other variables and X them out.
			search = ei->first.substr(0, ei->first.find_last_of("."));
			if (vars->get_kind(search) == "channel")
			{
				for (vi = vars->global.begin(); vi != vars->global.end(); vi++)
					if (vi->first.substr(0, search.length()) == search && vi->first != ei->first)
						s.assign(vi->second.uid, value("X"));
			}
		}
		else
			cout << "Error: Undefined variable " << ei->first << "." << endl;
	}

	cout << tab << s << endl;

	if(CHP_EDGE)
		g->insert(s, init, chp);
	else
		g->insert(s, init, "Assign");

	return uid;
}

void assignment::generate_scribes()
{

}

instruction *expand_assignment(string chp, vspace *vars, string tab, int verbosity)
{
	assignment *a = new assignment(chp, vars, tab, verbosity);
	parallel *p = new parallel("", vars, tab, verbosity);
	pair<string, instruction*> result;
	list<pair<string, string> >::iterator i;
	list<pair<string, string> > remove;
	variable *v;

	p->print_hse();

	for (i = a->expr.begin(); i != a->expr.end(); i++)
	{
		v = vars->find(i->second);
		if (i->second.find_first_of("&|~^=<>/+-*?@()") != i->second.npos)
		{
			result = expand_expression(i->second, vars, i->first, tab, verbosity);
			i->second = result.first;
			p->push(result.second);
			remove.push_back(*i);
		}
		else if (v != NULL)
		{
			cout << "LOOKHERE " << v->name << " " << v->width << endl;
		}
	}

	for (i = remove.begin(); i != remove.end(); i++)
		a->expr.remove(*i);

	if (p->instrs.size() == 0)
	{
		delete p;
		return a;
	}

	if (a->expr.size() > 0)
		p->push(a);
	else
		delete a;

	return p;
}

pair<string, instruction*> expand_expression(string chp, vspace *vars, string top, string tab, int verbosity)
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
		p = find_first_of_l0(chp, "^");
		if (p != chp.npos)
		{
			op = "^";
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

	C = pair<string, instruction*>("", NULL);
	if (chp[0] == '(' && chp[chp.length()-1] == ')' && op == "")
	{
		C = expand_expression(chp.substr(1, chp.length()-2), vars, top, tab+"\t", verbosity);
		if (C.first.find_first_of("&|^=<>/+-*?!@()") != C.first.npos)
			C.first = "(" + C.first + ")";
		return C;
	}

	A = pair<string, instruction*>(left, NULL);
	B = pair<string, instruction*>(right, NULL);
	if (left.find_first_of("&|~^=<>/+-*?!@()") != left.npos)
		A = expand_expression(left, vars, "", tab+"\t", verbosity);
	if (right.find_first_of("&|~^=<>/+-*?!@()") != right.npos)
		B = expand_expression(right, vars, "", tab+"\t", verbosity);

	if (top != "" && A.first.find_first_of("&|~^=<>/+-*?!@()") != A.first.npos)
		A.first = "(" + A.first + ")";
	if (top != "" && B.first.find_first_of("&|~^=<>/+-*?!@()") != B.first.npos)
		B.first = "(" + B.first + ")";

	if (A.second == NULL && B.second == NULL && (op == "&" || op == "|" || op == "~") && top == "" &&
	   (A.first.find_first_of("&|~") != A.first.npos || (vars->get_type(A.first) == "int" && vars->get_width(A.first) == 1) || A.first == "") &&
	   (B.first.find_first_of("&|~") != B.first.npos || (vars->get_type(B.first) == "int" && vars->get_width(B.first) == 1) || B.first == ""))
		return pair<string, instruction*>(A.first + op + B.first, NULL);

	string type = "operator" + op + "(";
	if (op == "?")
		type = vars->get_type(left) + "." + type;

	if (A.first != "")
	{
		if (A.first.find_first_of("|&~") != A.first.npos)
			type += "int<1>";
		else
			type += vars->get_info(A.first);
	}

	if (B.first != "")
	{
		if (A.first != "")
			type += ",";

		if (B.first.find_first_of("|&~") != B.first.npos)
			type += "int<1>";
		else
			type += vars->get_info(B.first);
	}

	type += ")";

	proc = (operate*)vars->find_type(type);
	if (proc == NULL)
	{
		cout << "Error: Undefined operator " << type << " used in " << chp << "." << endl;

		if (A.second != NULL)
			delete A.second;
		if (B.second != NULL)
			delete B.second;

		return pair<string, instruction*>(chp, NULL);
	}

	string name = vars->unique_name("_fn");
	if (op == "?")
		name = A.first + "." + name;

	name += "(";
	if (top == "")
	{
		C = add_unique_variable("_op", "", proc->vars.get_info(proc->input.front()), vars, tab+"\t", verbosity);
		name += C.first;
	}
	else
		name += top;

	if (A.first != "")
		name += "," + A.first;
	if (B.first != "")
		name += "," + B.first;
	name += ")";

	parallel *sub = new parallel();
	sub->tab = tab;
	sub->verbosity = verbosity;
	sub->vars = vars;
	sub->push(A.second);
	sub->push(B.second);

	block* ret = new block();
	ret->tab = tab;
	ret->verbosity = verbosity;
	ret->vars = vars;
	ret->push(sub);
	ret->push(expand_instantiation(type + " " + name, vars, NULL, tab, verbosity, true));

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
