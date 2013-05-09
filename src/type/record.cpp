/*
 * record.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../syntax.h"
#include "../data.h"

#include "keyword.h"
#include "record.h"
#include "process.h"

record::record()
{
	name = "";
	_kind = "record";
}

record::record(string raw, map<string, keyword*> *types, int verbosity)
{
	_kind = "record";
	vars.types = types;

	parse(raw, verbosity);
}

record::~record()
{
	name = "";
	_kind = "record";

	vars.clear();
}

record &record::operator=(record r)
{
	chp = r.chp;
	vars = r.vars;
	return *this;
}

void record::parse(string raw, int verbosity)
{
	chp = raw;

	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("{");
	int sequential_start = chp.find_first_of("{")+1;
	int sequential_end = chp.length()-1;
	string::iterator i, j;
	string io_sequential;

	map<string, variable> expansion;

	name = chp.substr(name_start, name_end - name_start);
	io_sequential = chp.substr(sequential_start, sequential_end - sequential_start);

	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
	{
		cout << "Record: " << chp << endl;
		cout << "\tName:  " << name << endl;
		cout << "\tSequential: " << io_sequential << endl;
	}

	for (i = io_sequential.begin(), j = io_sequential.begin(); i != io_sequential.end(); i++)
	{
		if (*(i+1) == ';')
		{
			expand_instantiation(NULL, io_sequential.substr(j-io_sequential.begin(), i+1 - j), &vars, NULL, "\t", verbosity, false);

			j = i+2;
		}
	}

	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
	{
		cout << endl;
	}
}

ostream &operator<<(ostream &os, record s)
{
    os << s.name << "{";
    os << s.vars;
    os << "}";

    return os;
}
