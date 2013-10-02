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

sstring instruction::kind()
{
	return _kind;
}

pair<sstring, instruction*> instruction::expand_expression(sstring expr, sstring top)
{
	if (flags->log_base_hse())
		(*flags->log_file) << flags->tab << "Decompose: " << expr << endl;

	smap<sstring, variable>::iterator v;
	type_space::iterator k;
	smap<sstring, sstring>::iterator c;
	list<sstring>::iterator s;
	list<instruction*>::iterator i;
	list<sstring> ops;
	list<sstring> ex;

	sstring left, right, op = "";
	operate *proc;

	pair<sstring, instruction*> A, B, C;

	int p;

	if (op == "")
	{
		p = expr.find_first_of_l0("|");
		if (p != expr.npos)
		{
			op = "|";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = expr.find_first_of_l0("&");
		if (p != expr.npos)
		{
			op = "&";
			left = expr.substr(0, p);
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = expr.find_first_of_l0("^");
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
		p = expr.find_first_of_l0(ops);
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
		p = expr.find_first_of_l0(ops);
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
		p = expr.find_first_of_l0(ops, 0, ex);
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
		p = expr.find_first_of_l0(ops);
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
		p = expr.find_first_of_l0("+-");
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
		p = expr.find_first_of_l0("*/");
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
		p = expr.find_first_of_l0("~");
		if (p != expr.npos)
		{
			op = "~";
			left = "";
			right = expr.substr(p+1);
		}
	}

	if (op == "")
	{
		p = expr.find_first_of_l0("?");
		if (p != expr.npos)
		{
			op = "?";
			left = expr.substr(0, p);
			right = "";
		}
	}

	if (op == "")
	{
		p = expr.find_first_of_l0("#");
		if (p != expr.npos)
		{
			op = "#";
			left = "";
			right = expr.substr(p+1);
		}
	}

	C = pair<sstring, instruction*>("", (instruction*)NULL);
	if (expr[0] == '(' && expr[expr.length()-1] == ')' && op == "")
	{
		C = expand_expression(expr.substr(1, expr.length()-2), top);
		if (C.first.find_first_of("&|^=<>/+-*?!#()") != C.first.npos)
			C.first = "(" + C.first + ")";
		return C;
	}

	A = pair<sstring, instruction*>(left, (instruction*)NULL);
	B = pair<sstring, instruction*>(right, (instruction*)NULL);
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
		return pair<sstring, instruction*>(A.first + op + B.first, (instruction*)NULL);

	sstring type = "operator" + op + "(";
	if (op == "?")
		type = vars->get_type(left) + "." + type;
	else if (op == "#")
		type = vars->get_type(right) + "." + type;

	if (A.first != "")
	{
		if (A.first.find_first_of("|&~") != A.first.npos)
			type += "node<1>";
		else if (op != "?")
			type += vars->get_info(A.first);
	}

	if (B.first != "")
	{
		if (A.first != "")
			type += ",";

		if (B.first.find_first_of("|&~") != B.first.npos)
			type += "node<1>";
		else if (op != "#")
			type += vars->get_info(B.first);
	}

	type += ")";

	proc = (operate*)vars->find_type(type);
	if (proc == NULL)
	{
		cerr << "Error: Undefined operator " << type << " used in " << expr << "." << endl;

		if (A.second != NULL)
			delete A.second;
		if (B.second != NULL)
			delete B.second;

		return pair<sstring, instruction*>(expr, (instruction*)NULL);
	}

	sequential* ret = new sequential();
	parallel *sub = new parallel();

	sstring name;
	if (op == "?")
		name = vars->unique_name(A.first + "." + "_fn");
	else if (op == "#")
		name = vars->unique_name(B.first + "." + "_fn");
	else
		name = vars->unique_name("_fn");

	name += "(";
	if (top == "" && op != "#")
	{
		C = add_unique_variable(ret, "_op", "", proc->vars.get_info(proc->args.front()), vars, flags);
		name += C.first;
	}
	else if (op != "#")
		name += top;

	if (A.first != "" && op != "?")
		name += "," + A.first;
	if (B.first != "" && op != "#")
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

	cout << "here" << endl;

	if (op == "#")
	{
		instruction *probe = expand_instantiation(ret, type + " " + name, vars, NULL, flags, true);
		if (probe->chp.find_first_of("&|~") != probe->chp.npos)
			C.first = "(" + probe->chp + ")";
		else
			C.first = probe->chp;
		delete probe;
		delete ret;
		probe = NULL;
		ret = NULL;
	}
	else
	{
		cout << "enter instantiation" << endl;
		instruction *bullshit = expand_instantiation(ret, type + " " + name, vars, NULL, flags, true);
		cout << "passed instantiation" << endl;
		ret->push(bullshit);
	}
	cout << "Here" << endl;

	return pair<sstring, instruction*>(C.first, ret);
}
