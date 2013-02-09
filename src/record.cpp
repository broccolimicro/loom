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
#include "process.h"

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
			expand_instantiation(io_block.substr(j-io_block.begin(), i+1 - j), types, &vars, &labels, NULL, tab+"\t", verbosity, false);

			j = i+2;
		}
	}
}

instruction *expand_instantiation(string chp, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, list<string> *input, string tab, int verbosity, bool allow_process)
{
	map<string, keyword*>::iterator var_type;
	map<string, variable>::iterator mem_var;
	variable v = variable(chp, global->size(), !allow_process, tab, verbosity);

	if (input != NULL)
		input->push_back(v.name);

	string name;

	if ((var_type = types.find(v.type)) != types.end())
	{
		if (v.type == "int")
			global->insert(pair<string, variable>(v.name, v));
		else
			label->insert(pair<string, variable>(v.name, v));

		if (var_type->second->kind() == "record" || var_type->second->kind() == "channel")
		{
			name = v.name;
			for (mem_var = ((record*)var_type->second)->vars.begin(); mem_var != ((record*)var_type->second)->vars.end(); mem_var++)
			{
				v = mem_var->second;

				if (v.type == "int")
					global->insert(pair<string, variable>(name + "." + v.name, variable(name + "." + v.name, global->size(), v.type, v.reset, v.width, !allow_process)));
				else
					label->insert(pair<string, variable>(name + "." + v.name, variable(name + "." + v.name, global->size(), v.type, v.reset, v.width, !allow_process)));
			}
		}
		else if (var_type->second->kind() == "process" || var_type->second->kind() == "operate")
		{
			if (allow_process)
			{
				cout << "Instantiating Process: " << v.type << " " << v.name << endl;

				map<string, string> convert;
				list<string>::iterator i, j;
				for (i = v.inputs.begin(), j = ((process*)var_type->second)->input.begin(); i != v.inputs.end() && j != ((process*)var_type->second)->input.end(); i++, j++)
					convert.insert(pair<string, string>(*j, *i));

				for (mem_var = ((process*)var_type->second)->global.begin(); mem_var != ((process*)var_type->second)->global.end(); mem_var++)
					if (!mem_var->second.io)
					{
						global->insert(pair<string, variable>(v.name + "." + mem_var->second.name, variable(mem_var->second.name, global->size(), mem_var->second.type, mem_var->second.reset, mem_var->second.width, mem_var->second.io)));
						convert.insert(pair<string, string>(mem_var->second.name, v.name + "." + mem_var->second.name));
					}
				for (mem_var = ((process*)var_type->second)->label.begin(); mem_var != ((process*)var_type->second)->label.end(); mem_var++)
					if (!mem_var->second.io)
					{
						label->insert(pair<string, variable>(v.name + "." + mem_var->second.name, variable(mem_var->second.name, global->size(), mem_var->second.type, mem_var->second.reset, mem_var->second.width, mem_var->second.io)));
						convert.insert(pair<string, string>(mem_var->second.name, v.name + "." + mem_var->second.name));
					}

				return ((process*)var_type->second)->def.duplicate(global, label, convert, tab, verbosity);
			}
			else
				cout << "Error: Invalid use of type " << var_type->second->kind() << " in record definition." << endl;
		}
	}
	else
		cout << "Error: Invalid typename: " << v.type << endl;

	return NULL;
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
