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

record::record(string raw, type_space *types, flag_space *flags)
{
	_kind = "record";
	vars.types = types;
	this->flags = flags;

	parse(raw);
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
	flags = r.flags;
	return *this;
}

void record::parse(string raw)
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

	if (flags->log_base_hse())
	{
		(*flags->log_file) << "Record: " << chp << endl;
		(*flags->log_file) << "\tName:  " << name << endl;
		(*flags->log_file) << "\tSequential: " << io_sequential << endl;
	}

	string instr;
	for (i = io_sequential.begin(), j = io_sequential.begin(); i != io_sequential.end(); i++)
	{
		if (*(i+1) == ';')
		{
			instr = io_sequential.substr(j-io_sequential.begin(), i+1 - j);
			if (instr.find_first_of("{}") != string::npos)
				debug(NULL, instr, &vars, flags).generate_class_requirements();
			else
				expand_instantiation(NULL, instr, &vars, NULL, flags, false);
			j = i+2;
		}
	}

	if (flags->log_base_hse())
	{
		(*flags->log_file) << endl;
	}
}

ostream &operator<<(ostream &os, record s)
{
    os << s.name << "{";
    os << s.vars;
    os << "}";

    return os;
}
