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
}

operate::operate(string raw, map<string, keyword*> *types, int verbosity)
{
	_kind = "operate";
	vars.types = types;

	parse(raw, verbosity);
}

operate::~operate()
{
	name = "";
	_kind = "operate";

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
	size_t block_start = chp.find_first_of("{")+1;
	size_t block_end = chp.length()-1;
	string io_block;
	string::iterator i, j;

	map<string, variable> temp;
	map<string, variable>::iterator vi, vj;
	map<string, keyword*>::iterator ti;
	list<string>::iterator ii, ij;

	cout << "Operator:\t" << chp << endl;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(input_start, input_end - input_start);

	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_block.end())
		{
			expand_instantiation(NULL, io_block.substr(j-io_block.begin(), i+1 - j), &vars, &input, "\t", verbosity, false);
			j = i+2;
		}
	}

	if (input.size() > 3)
		cout << "Error: Operators can have at most two inputs and one output." << endl;

	def.init(chp.substr(block_start, block_end - block_start), &vars, "\t", verbosity);

	variable *tv;

	name += "(";
	ij = input.begin();
	ij++;
	for (ii = ij; ii != input.end(); ii++)
	{
		if (ii != ij)
			name += ",";
		tv = vars.find(*ii);

		if (tv != NULL)
		{
			if (tv->driven)
				cout << "Error: Input " << *ii << " driven in " << chp << endl;

			name += tv->type;
			if (tv->type == "int" && tv->fixed)
				name += "<" + to_string(tv->width) + ">";
		}
	}
	name += ")";

	if (verbosity >= VERB_PARSE)
	{
		cout << "\tName:\t" << name << endl;
		cout << "\tInputs:\t" << io_block << endl;
	}

	cout << vars << endl;
	def.print_hse();
	cout << endl << endl;
}
