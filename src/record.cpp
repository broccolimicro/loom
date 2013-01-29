/*
 * record.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "record.h"
#include "common.h"
#include "keyword.h"
#include "variable.h"

record::record()
{
	name = "";
	_kind = "record";
}

record::record(string raw, map<string, keyword*> types, string tab, int verbosity)
{
	parse(raw, types, tab, verbosity);
	_kind = "record";
}

record::~record()
{
	name = "";
	_kind = "record";

	vars.clear();
}

record &record::operator=(record r)
{
	vars = r.vars;
	return *this;
}

void record::parse(string raw, map<string, keyword*> types, string tab, int verbosity)
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
		cout << tab << "Record: " << chp << endl;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(block_start, block_end - block_start);

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "\tName:  " << name << endl;
		cout << tab << "\tBlock: " << io_block << endl;
	}

	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*(i+1) == ';')
		{
			expand(io_block.substr(j-io_block.begin(), i+1 - j), name, types, &vars, &labels, tab+"\t", verbosity);

			j = i+2;
		}
	}
}

void expand(string chp, string super, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, string tab, int verbosity)
{
	map<string, variable> result;
	map<string, keyword*>::iterator var_type;
	map<string, variable>::iterator mem_var;
	variable v = variable(chp, -1, tab+"\t", verbosity);
	string name;

	if ((var_type = types.find(v.type)) != types.end())
	{
		result.insert(pair<string, variable>(v.name, v));

		if (var_type->second->kind() == "record" || var_type->second->kind() == "channel")
		{
			name = v.name;
			for (mem_var = ((record*)var_type->second)->vars.begin(); mem_var != ((record*)var_type->second)->vars.end(); mem_var++)
			{
				v = mem_var->second;
				result.insert(pair<string, variable>(name + "." + v.name, variable(name + "." + v.name, -1, v.type, v.reset, v.width)));
			}
		}
		else if (var_type->second->kind() == "process")
			cout << "Error: Invalid use of type " << var_type->second->kind() << " in record definition." << endl;
	}
	else
		cout << "Error: Invalid typename: " << v.type << endl;

	return result;
}

ostream &operator<<(ostream &os, record s)
{
    os << s.name << "{";
    map<string, variable>::iterator i;
    for (i = s.vars.begin(); i != s.vars.end(); i++)
    {
    	os << i->second << " ";
    }
    os << "}";

    return os;
}



