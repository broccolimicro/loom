/*
 * record.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
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

record::record(string chp, map<string, keyword*>	typ)
{
	parse(chp, typ);
	_kind = "record";
}

record::~record()
{
	name = "";
	_kind = "record";
}

record &record::operator=(record r)
{
	vars = r.vars;
	return *this;
}

void record::parse(string chp, map<string, keyword*>	typ)
{
	cout << "record! -> " << chp << endl;
	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("{");
	int block_start = chp.find_first_of("{")+1;
	int block_end = chp.length()-1;
	string::iterator i, j;
	string io_block;

	variable v;
	map<string, variable> expansion;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(block_start, block_end - block_start);

	cout << "\tname!   -> " << name << endl;
	cout << "\tblock!  -> " << io_block << endl;

	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*(i+1) == ';')
		{
			v.parse(io_block.substr(j-io_block.begin(), i+1 - j));
			expansion = expand(v, typ);

			vars.insert(expansion.begin(), expansion.end());

			j = i+2;
		}
	}
}

map<string, variable> expand(variable v, map<string, keyword*>	typ)
{
	map<string, variable> result;
	map<string, keyword*>::iterator var_type;
	map<string, variable>::iterator mem_var;
	string name;

	if ((var_type = typ.find(v.type)) != typ.end())
	{
		if (var_type->second->kind() == "keyword")
			result.insert(pair<string, variable>(v.name, v));
		else if (var_type->second->kind() == "record")
		{
			name = v.name;
			for (mem_var = ((record*)var_type->second)->vars.begin(); mem_var != ((record*)var_type->second)->vars.end(); mem_var++)
			{
				v = mem_var->second;
				v.name = name + "." + v.name;
				result.insert(pair<string, variable>(v.name, v));
			}
		}
		else
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



