/*
 * variable.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
 */

#include "variable.h"
#include "common.h"

variable::variable()
{
	name = "";
	type = "";
	super = "";
	width = 0;
	last = "iX";
	reset = "iX";
	fixed = false;
}

variable::variable(string n, string t, string s, uint16_t w)
{
	name = n;
	type = t;
	super = s;
	width = w;
	last = "iX";
	reset = "iX";
	fixed = true;
}

variable::variable(string chp, string spr, string tab)
{
	last = "iX";
	reset = "iX";
	super = spr;
	parse(chp, tab);
}

variable::~variable()
{
	name = "";
	type = "";
	super = "";
	width = 0;
	last = "iX";
	reset = "iX";
	fixed = false;
}

variable &variable::operator=(variable v)
{
	name = v.name;
	type = v.type;
	super = v.super;
	width = v.width;
	last = v.last;
	reset = v.reset;
	fixed = v.fixed;
	return *this;
}

void variable::parse(string chp, string tab)
{
	cout << tab << "Variable: " << chp << endl;

	size_t width_start = chp.find_first_of("< ");
	size_t name_start = chp.find_first_of("> ");
	size_t reset_start = chp.find(":=");

	if (reset_start != chp.npos)
	{
		name = chp.substr(name_start+1, reset_start - (name_start+1));
		reset.prs = true;
		reset.data = chp.substr(reset_start+2);
		if (reset.data[1] == 'x')				// hexadecimal e.g. 0xFEEDFACE
			reset.data = hex_to_bin(reset.data.substr(2));

		else if (reset.data[1] == 'b')			// binary      e.g. 0b01100110
			reset.data = reset.data.substr(2);

		else									// decimal     e.g. 20114
			reset.data = dec_to_bin(reset.data);
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
	{
		fixed = false;
		width = reset.data.length();
	}

	cout << tab << "\tName:  " << name << endl;
	cout << tab << "\tType:  " << type << endl;
	cout << tab << "\tWidth: " << width << endl;
	cout << tab << "\tReset:  " << reset << endl;
}

ostream &operator<<(ostream &os, variable s)
{
    os << s.name << "<" << s.width << ">";

    return os;
}

