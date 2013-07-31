/*
 * variable.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "variable.h"

variable::variable()
{
	chp = "";
	name = "";
	type = "";
	width = 0;
	fixed = false;
	uid = -1;
	driven = false;
	arg = false;
	flags = NULL;
}

variable::variable(string name, string type, uint16_t width, bool arg, flag_space *flags)
{
	this->chp = type + "<" + to_string(width) + ">" + name;
	this->name = name;
	this->type = type;
	this->width = width;
	this->fixed = true;
	this->uid = -1;
	this->driven = false;
	this->arg = arg;
	this->flags = flags;
}

variable::variable(string chp, bool arg, flag_space *flags)
{
	this->chp = chp;
	this->uid = -1;
	this->arg = arg;
	this->flags = flags;
	this->driven = false;

	parse(chp);
}

variable::~variable()
{
	name = "";
	type = "";
	width = 0;
	fixed = false;
}

variable &variable::operator=(variable v)
{
	chp = v.chp;
	name = v.name;
	type = v.type;
	width = v.width;
	fixed = v.fixed;
	uid = v.uid;
	driven = v.driven;
	arg = v.arg;
	flags = v.flags;

	return *this;
}

void variable::parse(string chp)
{
	this->chp = chp;

	string input;

	size_t width_start = find_first_of_l0(chp, "< ");
	size_t name_start = find_first_of_l0(chp, "> ");
	size_t input_start = find_first_of_l0(chp, "(", name_start);
	size_t input_end = find_first_of_l0(chp, ")", input_start);

	if (flags->log_base_hse())
		(*flags->log_file) << flags->tab << "Variable: " << chp << endl;

	if (input_start != chp.npos)
	{
		string temp;
		name = chp.substr(name_start+1, input_start - (name_start+1));
		input = chp.substr(input_start+1, input_end - (input_start+1));
		inputs.push_back(input.substr(0, input.find_first_of(",")));
		for (size_t i = input.find_first_of(","); i != input.npos; i = input.find_first_of(",", i+1))
			inputs.push_back(input.substr(i+1, input.find_first_of(",", i+1) - i-1));
	}
	else
		name = chp.substr(name_start+1);

	type = chp.substr(0, width_start);
	if (chp.find_first_of("<>") != chp.npos)
	{
		fixed = true;
		width = atoi(chp.substr(width_start+1, name_start - (width_start+1)).c_str());
	}
	else
		fixed = false;

	if (flags->log_base_hse())
	{
		(*flags->log_file) << flags->tab << "\tName:  " << name << endl;
		(*flags->log_file) << flags->tab << "\tArg:    " << input << endl;
		(*flags->log_file) << flags->tab << "\tType:  " << type << endl;
		(*flags->log_file) << flags->tab << "\tWidth: " << width << endl;
		(*flags->log_file) << flags->tab << "\tArg:    " << arg << endl;
	}
}

ostream &operator<<(ostream &os, map<string, variable> g)
{
	map<string, variable>::iterator i;
	for (i = g.begin(); i != g.end(); i++)
		os << i->first << " ";

	return os;
}

ostream &operator<<(ostream &os, variable s)
{
    os << s.name << "<" << s.width << ">";

    return os;
}

