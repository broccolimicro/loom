/*
 * channel.cpp
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
#include "channel.h"

channel::channel()
{
	name = "";
	_kind = "channel";
}

channel::channel(string chp, map<string, keyword*> types, int verbosity)
{
	parse(chp, types, verbosity);
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

void channel::parse(string chp, map<string, keyword*> types, int verbosity)
{
	if (verbosity >= VERB_PARSE)
		cout << "Channel: " << chp << endl;

	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("{");
	int block_start = chp.find_first_of("{")+1;
	int block_end = chp.length()-1;
	string::iterator i, j;
	string io_block;
	string raw;

	string s = "", r = "", p = "";

	map<string, state> res;
	map<string, state>::iterator ri;

	map<string, variable> expansion;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(block_start, block_end - block_start);

	if (verbosity >= VERB_PARSE)
	{
		cout << "\tName:  " << name << endl;
		cout << "\tBlock: " << io_block << endl;
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
			expand_instantiation(io_block.substr(j-io_block.begin(), i - j), types, &vars, NULL, "\t", verbosity, false);
			j = i+1;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && *i == '}')
		{
			// TODO Do we need to reparse afterword and use the results of the first parsing as the init?
			raw = io_block.substr(j-io_block.begin(), i - j + 1);
			if (raw.find("operator!") != raw.npos)
				s = raw;
			else if (raw.find("operator?") != raw.npos)
				r = raw;
			else if (raw.find("operator@") != raw.npos)
				p = raw;

			j = i+1;
		}
	}

	/* These variables won't automatically be instantiated as
	 * [operator name].[var name] because they are considered
	 * to be io variables. If you look above at the expand instantiation
	 * function, allow process is false, making the io flag true.
	 */

	send.vars.global.insert(vars.global.begin(), vars.global.end());
	send.vars.label.insert(vars.label.begin(), vars.label.end());
	recv.vars.global.insert(vars.global.begin(), vars.global.end());
	recv.vars.label.insert(vars.label.begin(), vars.label.end());
	probe.vars.global.insert(vars.global.begin(), vars.global.end());
	probe.vars.label.insert(vars.label.begin(), vars.label.end());

	send.parse(s, types, verbosity);
	recv.parse(r, types, verbosity);
	probe.parse(p, types, verbosity);
}

ostream &operator<<(ostream &os, channel s)
{
    os << s.name << "{";
    os << s.vars;
    os << "}";

    return os;
}



