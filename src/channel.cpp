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

channel::channel(string chp, map<string, keyword*> types, string tab, int verbosity)
{
	parse(chp, types, tab, verbosity);
	_kind = "channel";
}

channel::~channel()
{
	name = "";
	_kind = "channel";

	vars.clear();
}

channel &channel::operator=(channel r)
{
	vars = r.vars;
	return *this;
}

void channel::parse(string chp, map<string, keyword*> types, string tab, int verbosity)
{
	if (verbosity >= VERB_PARSE)
		cout << tab << "Channel: " << chp << endl;

	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("{");
	int block_start = chp.find_first_of("{")+1;
	int block_end = chp.length()-1;
	string::iterator i, j;
	string io_block;
	string raw;

	map<string, state> res;
	map<string, state>::iterator ri;

	map<string, variable> expansion;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(block_start, block_end - block_start);

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "\tName:  " << name << endl;
		cout << tab << "\tBlock: " << io_block << endl;
	}

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
			expand(io_block.substr(j-io_block.begin(), i - j), name, types, &vars, &labels, tab+"\t", verbosity);
			j = i+1;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && *i == '}')
		{
			// TODO do we need to reparse afterword and use the results of the first parsing as the init?
			// TODO send and recv should be defined as operators instead
			raw = io_block.substr(j-io_block.begin(), i - j + 1);
			if (raw.find("process send") != raw.npos)
			{
				send.parse(raw, types, vars, verbosity);
				//res = send.def.result;
				//for (ri = res.begin(); ri != res.end(); ri++)
				//	ri->second.prs = false;
				//send.def.parse(send.def.chp, types, vars, res, "\t", verbosity);
			}
			else if (raw.find("process recv") != raw.npos)
			{
				recv.parse(raw, types, vars, verbosity);
				//res = recv.def.result;
				//for (ri = res.begin(); ri != res.end(); ri++)
				//	ri->second.prs = false;
				//recv.def.parse(recv.def.chp, types, vars, res, "\t", verbosity);
			}

			j = i+1;
		}
	}
}

ostream &operator<<(ostream &os, channel s)
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



