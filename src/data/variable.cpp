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
	reset = value("X");
	uid = -1;
	driven = false;
	io = false;
}

variable::variable(string name, string type, value reset, uint16_t width, bool io)
{
	this->chp = type + "<" + to_string(width) + ">" + name + ":=" + reset.data;
	this->name = name;
	this->type = type;
	this->width = width;
	this->reset = reset;
	this->fixed = true;
	this->uid = -1;
	this->driven = false;
	this->io = io;

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "Variable: " << chp << endl;
		cout << tab << "\tName:  " << name << endl;
		cout << tab << "\tType:  " << type << endl;
		cout << tab << "\tWidth: " << width << endl;
		cout << tab << "\tReset: " << reset << endl;
		cout << tab << "\tIO:    " << io << endl;
	}
}

variable::variable(string chp, bool io, string tab, int verbosity)
{
	this->chp = chp;
	this->uid = -1;
	this->io = io;
	this->tab = tab;
	this->verbosity = verbosity;
	this->driven = false;

	parse(chp);
}

variable::~variable()
{
	name = "";
	type = "";
	width = 0;
	reset = value("X");
	fixed = false;
}

variable &variable::operator=(variable v)
{
	chp = v.chp;
	name = v.name;
	type = v.type;
	width = v.width;
	fixed = v.fixed;
	reset = v.reset;
	uid = v.uid;
	driven = v.driven;
	io = v.io;

	return *this;
}

void variable::parse(string chp)
{
	reset = value("X");

	this->chp = chp;

	string input;

	size_t width_start = find_first_of_l0(chp, "< ");
	size_t name_start = find_first_of_l0(chp, "> ");
	size_t input_start = find_first_of_l0(chp, "(", name_start);
	size_t input_end = find_first_of_l0(chp, ")", input_start);
	size_t reset_start = chp.find(":=");

	if (verbosity >= VERB_PARSE)
		cout << tab << "Variable: " << chp << endl;

	if (input_start != chp.npos)
	{
		string temp;
		name = chp.substr(name_start+1, input_start - (name_start+1));
		input = chp.substr(input_start+1, input_end - (input_start+1));
		inputs.push_back(input.substr(0, input.find_first_of(",")));
		for (size_t i = input.find_first_of(","); i != input.npos; i = input.find_first_of(",", i+1))
			inputs.push_back(input.substr(i+1, input.find_first_of(",", i+1) - i-1));
	}
	else if (reset_start != chp.npos)
		name = chp.substr(name_start+1, reset_start - (name_start+1));
	else
		name = chp.substr(name_start+1);

	if (reset_start != chp.npos)
	{
		reset = value(chp.substr(reset_start+2));
		if (reset.data[1] == 'x')				// hexadecimal e.g. 0xFEEDFACE
			reset.data = hex_to_bin(reset.data.substr(2));
		else if (reset.data[1] == 'b')			// binary      e.g. 0b01100110
			reset.data = reset.data.substr(2);
		else									// decimal     e.g. 20114
			reset.data = dec_to_bin(reset.data);
	}
	else
		reset = value("X");

	type = chp.substr(0, width_start);
	if (chp.find_first_of("<>") != chp.npos)
	{
		fixed = true;
		width = atoi(chp.substr(width_start+1, name_start - (width_start+1)).c_str());
	}
	else
	{
		fixed = false;
		width = reset.data.length();
	}

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "\tName:  " << name << endl;
		cout << tab << "\tIO:    " << input << endl;
		cout << tab << "\tType:  " << type << endl;
		cout << tab << "\tWidth: " << width << endl;
		cout << tab << "\tReset: " << reset << endl;
		cout << tab << "\tIO:    " << io << endl;
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

