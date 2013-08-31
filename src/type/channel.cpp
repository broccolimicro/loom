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

channel::channel(string chp, type_space *types, flag_space *flags)
{
	_kind = "channel";
	vars.types = types;
	send = NULL;
	recv = NULL;
	probe = NULL;
	this->flags = flags;
	parse(chp);
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
	flags = r.flags;
	return *this;
}

void channel::parse(string chp)
{
	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("{");
	int sequential_start = chp.find_first_of("{")+1;
	int sequential_end = chp.length()-1;
	string::iterator i, j;
	string io_sequential;
	string raw;

	string s = "", r = "", p = "";

	map<string, variable> expansion;

	name = chp.substr(name_start, name_end - name_start);
	io_sequential = chp.substr(sequential_start, sequential_end - sequential_start);

	if (flags->log_base_hse())
	{
		(*flags->log_file) << "Channel: " << chp << endl;
		(*flags->log_file) << "\tName:  " << name << endl;
		(*flags->log_file) << "\tSequential: " << io_sequential << endl;
	}

	int depth[3] = {0};
	string instr;
	for (i = io_sequential.begin(), j = io_sequential.begin(); i != io_sequential.end(); i++)
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
			instr = io_sequential.substr(j-io_sequential.begin(), i - j);
			if (instr.find_first_of("{}") != string::npos)
				debug(NULL, instr, &vars, flags).generate_class_requirements();
			else
				expand_instantiation(NULL, instr, &vars, NULL, flags, false);
			j = i+1;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && *i == '}')
		{
			raw = io_sequential.substr(j-io_sequential.begin(), i - j + 1);
			if (raw.find("operator!") != raw.npos)
				s = raw;
			else if (raw.find("operator?") != raw.npos)
				r = raw;
			else if (raw.find("operator#") != raw.npos)
				p = raw;

			j = i+1;
		}
	}

	if (flags->log_base_hse())
	{
		(*flags->log_file) << endl;
	}

	vars.types->insert(pair<string, channel*>(name, this));


	/* These variables won't automatically be instantiated as
	 * [operator name].[var name] because they are considered
	 * to be io variables. If you look above at the expand instantiation
	 * function, allow process is false, making the io flag true.
	 */

	send = new operate();
	recv = new operate();
	probe = new operate();

	variable_space temp;
	temp.types = vars.types;
	expand_instantiation(NULL, name + " this", &temp, NULL, flags, false);

	send->vars = temp;
	send->flags = flags;
	recv->vars = temp;
	recv->flags = flags;
	probe->vars = temp;
	probe->flags = flags;

	send->parse(s);
	recv->parse(r);
	probe->parse(p);

	vars.types->insert(pair<string, operate*>(name + "." + send->name, send));
	vars.types->insert(pair<string, operate*>(name + "." + recv->name, recv));
	vars.types->insert(pair<string, operate*>(name + "." + probe->name, probe));
}

ostream &operator<<(ostream &os, channel s)
{
    os << s.name << "{";
    os << s.vars;
    os << "}";

    return os;
}



