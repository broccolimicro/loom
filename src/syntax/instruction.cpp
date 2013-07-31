/*
 * instruction.cpp
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "instruction.h"
#include "parallel.h"
#include "sequential.h"

instruction::instruction()
{
	parent = NULL;
	vars = NULL;
	net = NULL;
	chp = "";
	_kind = "instruction";
	flags = NULL;
}

instruction::~instruction()
{
	_kind = "instruction";
	parent = NULL;
	flags = NULL;
}

string instruction::kind()
{
	return _kind;
}

pair<string, instruction*> instruction::expand_expression(string expr, string top)
{
	if (flags->log_base_hse())
		(*flags->log_file) << flags->tab << "Decompose: " << expr << endl;

	map<string, variable>::iterator v;
	type_space::iterator k;
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
		p = find_first_of_l0(expr, "|");
		if (p != expr.npos)
		{
			op = "|";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(expr, "&");
		if (p != expr.npos)
		{
			op = "&";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(expr, "^");
		if (p != expr.npos)
		{
			op = "^";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		ops.clear();
		ops.push_back("==");
		ops.push_back("~=");
		p = find_first_of_l0(expr, ops);
		if (p != expr.npos && expr.substr(p, 2) == "==")
		{
			op = "==";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
		else if (p != expr.npos && expr.substr(p, 2) == "!=")
		{
			op = "!=";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		ops.clear();
		ops.push_back("<=");
		ops.push_back(">=");
		p = find_first_of_l0(expr, ops);
		if (p != expr.npos && expr.substr(p, 2) == "<=")
		{
			op = "<=";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
		else if (p != expr.npos && expr.substr(p, 2) == ">=")
		{
			op = ">=";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
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
		p = find_first_of_l0(expr, ops, 0, ex);
		if (p != expr.npos && expr[p] == '<')
		{
			op = "<";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
		else if (p != expr.npos && expr[p] == '>')
		{
			op = ">";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		ops.clear();
		ops.push_back("<<");
		ops.push_back(">>");
		p = find_first_of_l0(expr, ops);
		if (p != expr.npos && expr.substr(p, 2) == "<<")
		{
			op = "<<";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
		else if (p != expr.npos && expr.substr(p, 2) == ">>")
		{
			op = ">>";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(expr, "+-");
		if (p != expr.npos && expr[p] == '+')
		{
			op = "+";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
		else if (p != expr.npos && expr[p] == '-')
		{
			op = "-";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(expr, "*/");
		if (p != expr.npos && expr[p] == '*')
		{
			op = "*";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
		else if (p != expr.npos && expr[p] == '/')
		{
			op = "/";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(expr, "~");
		if (p != expr.npos)
		{
			op = "~";
			left = "";
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(expr, "?");
		if (p != expr.npos)
		{
			op = "?";
			left = expr.substr(0, p);
			right = "";
		}
	}

	C = pair<string, instruction*>("", NULL);
	if (expr[0] == '(' && expr[expr.length()-1] == ')' && op == "")
	{
		C = expand_expression(expr.substr(1, expr.length()-2), top);
		if (C.first.find_first_of("&|^=<>/+-*?!#()") != C.first.npos)
			C.first = "(" + C.first + ")";
		return C;
	}

	A = pair<string, instruction*>(left, NULL);
	B = pair<string, instruction*>(right, NULL);
	if (left.find_first_of("&|~^=<>/+-*?!#()") != left.npos)
		A = expand_expression(left, "");
	if (right.find_first_of("&|~^=<>/+-*?!#()") != right.npos)
		B = expand_expression(right, "");

	if (top != "" && A.first.find_first_of("&|~^=<>/+-*?!#()") != A.first.npos)
		A.first = "(" + A.first + ")";
	if (top != "" && B.first.find_first_of("&|~^=<>/+-*?!#()") != B.first.npos)
		B.first = "(" + B.first + ")";

	if (A.second == NULL && B.second == NULL && (op == "&" || op == "|" || op == "~") && top == "" &&
	   (A.first.find_first_of("&|~") != A.first.npos || (vars->get_type(A.first) == "node" && vars->get_width(A.first) == 1) || A.first == "") &&
	   (B.first.find_first_of("&|~") != B.first.npos || (vars->get_type(B.first) == "node" && vars->get_width(B.first) == 1) || B.first == ""))
		return pair<string, instruction*>(A.first + op + B.first, NULL);

	string type = "operator" + op + "(";
	if (op == "?")
		type = vars->get_type(left) + "." + type;

	if (A.first != "")
	{
		if (A.first.find_first_of("|&~") != A.first.npos)
			type += "node<1>";
		else
			type += vars->get_info(A.first);
	}

	if (B.first != "")
	{
		if (A.first != "")
			type += ",";

		if (B.first.find_first_of("|&~") != B.first.npos)
			type += "node<1>";
		else
			type += vars->get_info(B.first);
	}

	type += ")";

	proc = (operate*)vars->find_type(type);
	if (proc == NULL)
	{
		cout << "Error: Undefined operator " << type << " used in " << expr << "." << endl;

		if (A.second != NULL)
			delete A.second;
		if (B.second != NULL)
			delete B.second;

		return pair<string, instruction*>(expr, NULL);
	}

	sequential* ret = new sequential();
	parallel *sub = new parallel();

	string name = vars->unique_name("_fn");
	if (op == "?")
		name = A.first + "." + name;

	name += "(";
	if (top == "")
	{
		C = add_unique_variable(ret, "_op", "", proc->vars.get_info(proc->args.front()), vars, flags);
		//cout << "YO DUERFEW EWFFEW EFWEFEWF " << proc->vars.get_info(proc->input.front()) << endl;
		name += C.first;
	}
	else
		name += top;

	if (A.first != "")
		name += "," + A.first;
	if (B.first != "")
		name += "," + B.first;
	name += ")";

	sub->flags = flags;
	sub->vars = vars;
	if (A.second != NULL)
		A.second->parent = sub;
	if (B.second != NULL)
		B.second->parent = sub;
	sub->push(A.second);
	sub->push(B.second);
	sub->parent = ret;


	ret->flags = flags;
	ret->vars = vars;
	ret->push(sub);
	ret->push(expand_instantiation(ret, type + " " + name, vars, NULL, flags, true));

	return pair<string, instruction*>(C.first, ret);
}
