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

record::record(string raw, map<string, keyword*> types, int verbosity)
{
	parse(raw, types, verbosity);
	_kind = "record";
}

record::~record()
{
	name = "";
	_kind = "record";

	globals.clear();
}

record &record::operator=(record r)
{
	globals = r.globals;
	return *this;
}

void record::parse(string raw, map<string, keyword*> types, int verbosity)
{
	chp = raw;

	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("{");
	int block_start = chp.find_first_of("{")+1;
	int block_end = chp.length()-1;
	string::iterator i, j;
	string io_block;

	map<string, variable> expansion;

	if (verbosity >= VERB_PARSE)
		cout << "Record: " << chp << endl;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(block_start, block_end - block_start);

	if (verbosity >= VERB_PARSE)
	{
		cout << "\tName:  " << name << endl;
		cout << "\tBlock: " << io_block << endl;
	}

	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*(i+1) == ';')
		{
			expand_instantiation(io_block.substr(j-io_block.begin(), i+1 - j), types, &globals, &labels, NULL, "\t", verbosity, false);

			j = i+2;
		}
	}
}

ostream &operator<<(ostream &os, record s)
{
    os << s.name << "{";
    map<string, variable>::iterator i;
    for (i = s.globals.begin(); i != s.globals.end(); i++)
    {
    	os << i->second << " ";
    }
    os << "}";

    return os;
}
