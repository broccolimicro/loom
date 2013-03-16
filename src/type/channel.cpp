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
	send = NULL;
	recv = NULL;
	probe = NULL;
}

channel::channel(string chp, map<string, keyword*> *types, int verbosity)
{
	_kind = "channel";
	vars.types = types;
	send = NULL;
	recv = NULL;
	probe = NULL;
	parse(chp, verbosity);
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

// TODO don't allow reset values for channels
void channel::parse(string chp, int verbosity)
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
			expand_instantiation(NULL, io_block.substr(j-io_block.begin(), i - j), &vars, NULL, "\t", verbosity, false);
			j = i+1;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && *i == '}')
		{
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

	send = new operate();
	recv = new operate();
	probe = new operate();

	send->vars = vars;
	recv->vars = vars;
	probe->vars = vars;

	send->parse(s, verbosity);
	recv->parse(r, verbosity);
	probe->parse(p, verbosity);
}

ostream &operator<<(ostream &os, channel s)
{
    os << s.name << "{";
    os << s.vars;
    os << "}";

    return os;
}



