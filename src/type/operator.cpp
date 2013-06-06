/*
 * process.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../syntax.h"
#include "../data.h"

#include "record.h"
#include "channel.h"
#include "process.h"
#include "operator.h"

operate::operate()
{
	name = "";
	_kind = "operate";
	is_inline = true;
}

operate::operate(string raw, map<string, keyword*> *types, int verbosity)
{
	_kind = "operate";
	vars.types = types;
	is_inline = true;

	parse(raw, verbosity);
}

operate::~operate()
{
	name = "";
	_kind = "operate";
	is_inline = true;

	vars.clear();
}

operate &operate::operator=(operate p)
{
	def = p.def;
	prs = p.prs;
	vars = p.vars;
	return *this;
}

void operate::parse(string raw, int verbosity)
{
	chp = raw;

	size_t name_start = 0;
	size_t name_end = chp.find_first_of("(");
	size_t input_start = chp.find_first_of("(")+1;
	size_t input_end = chp.find_first_of(")");
	size_t sequential_start = chp.find_first_of("{")+1;
	size_t sequential_end = chp.length()-1;
	string io_sequential;
	string::iterator i, j;

	map<string, variable> temp;
	map<string, variable>::iterator vi, vj;
	map<string, keyword*>::iterator ti;
	list<string>::iterator ii, ij;

	name = chp.substr(name_start, name_end - name_start);
	io_sequential = chp.substr(input_start, input_end - input_start);

	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
	{
		cout << "Operator:\t" << chp << endl;
		cout << "\tName:\t" << name << endl;
		cout << "\tInputs:\t" << io_sequential << endl;
	}

	for (i = io_sequential.begin(), j = io_sequential.begin(); i != io_sequential.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_sequential.end())
		{
			expand_instantiation(NULL, io_sequential.substr(j-io_sequential.begin(), i+1 - j), &vars, &args, "\t", verbosity, false);
			j = i+2;
		}
	}

	if (args.size() > 3)
		cout << "Error: Operators can have at most two inputs and one output." << endl;

	def.init(chp.substr(sequential_start, sequential_end - sequential_start), &vars, "\t", verbosity);

	variable *tv;

	name += "(";
	ij = args.begin();
	ij++;
	for (ii = ij; ii != args.end(); ii++)
	{
		if (ii != ij)
			name += ",";
		tv = vars.find(*ii);

		if (tv != NULL)
		{
			if (tv->driven)
				cout << "Error: Input " << *ii << " driven in " << chp << endl;

			name += tv->type;
			if (tv->type == "node" && tv->fixed)
				name += "<" + to_string(tv->width) + ">";
		}
	}
	name += ")";

	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
	{
		cout << "\tVariables:" << endl;
		vars.print("\t\t");
		cout << "\tHSE:" << endl;
		def.print_hse("\t\t");
		cout << endl << endl;
	}
}
