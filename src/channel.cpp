/*
 * channel.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "channel.h"
#include "common.h"
#include "keyword.h"
#include "variable.h"

channel::channel()
{
	name = "";
	_kind = "channel";
}

channel::channel(string chp, map<string, keyword*> types, string tab)
{
	parse(chp, types, tab);
	_kind = "channel";
}

channel::~channel()
{
	name = "";
	_kind = "channel";

	map<string, variable*>::iterator i;
	for (i = vars.begin(); i != vars.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	vars.clear();
}

channel &channel::operator=(channel r)
{
	vars = r.vars;
	return *this;
}

void channel::parse(string chp, map<string, keyword*> types, string tab)
{
	cout << tab << "Channel: " << chp << endl;
	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("{");
	int block_start = chp.find_first_of("{")+1;
	int block_end = chp.length()-1;
	string::iterator i, j;
	string io_block;
	string raw;

	map<string, variable*> expansion;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(block_start, block_end - block_start);

	cout << tab << "\tName:  " << name << endl;
	cout << tab << "\tBlock: " << io_block << endl;

	int depth[3] = {0};
	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*i == '(')
			depth[0]++;
		else if (*i == '[')
			depth[1]++;
		else if (*i == '{')
			depth[2]++;
		else if (*i == ')')
			depth[0]--;
		else if (*i == ']')
			depth[1]--;
		else if (*i == '}')
			depth[2]--;

		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && *i == ';')
		{
			expansion = expand(io_block.substr(j-io_block.begin(), i - j), name, types, tab+"\t");
			vars.insert(expansion.begin(), expansion.end());

			j = i+1;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && *i == '}')
		{
			raw = io_block.substr(j-io_block.begin(), i - j + 1);
			if (raw.find("proc send") != raw.npos)
				send.parse(raw, types, vars);
			else if (raw.find("proc recv") != raw.npos)
				recv.parse(raw, types, vars);

			j = i+1;
		}
	}
}

ostream &operator<<(ostream &os, channel s)
{
    os << s.name << "{";
    map<string, variable*>::iterator i;
    for (i = s.vars.begin(); i != s.vars.end(); i++)
    {
    	os << *(i->second) << " ";
    }
    os << "}";

    return os;
}



