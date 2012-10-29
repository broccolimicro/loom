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
	width = 0;
	last = "X";
}

variable::variable(string n, string t, uint16_t w)
{
	name = n;
	type = t;
	width = w;
	last = "X";
}

variable::variable(string chp)
{
	parse(chp);
	last = "X";
}

variable::~variable()
{
	name = "";
	type = "";
	width = 0;
	last = "X";
}

variable &variable::operator=(variable v)
{
	name = v.name;
	type = v.type;
	width = v.width;
	last = v.last;
	return *this;
}

void variable::parse(string chp)
{
	cout << "\t\tvariable! -> "+chp << endl;

	int width_start = chp.find_first_of("< ");
	int name_start = chp.find_first_of("> ");

	name = chp.substr(chp.find_first_of("> ")+1);
	type = chp.substr(0, width_start);
	if (chp.find_first_of("<>") != chp.npos)
		width = atoi(chp.substr(width_start+1, name_start - (width_start+1)).c_str());
	else
		width = 0;

	cout << "\t\t\ttype! -> " << type << endl;
	cout << "\t\t\twidth!-> " << width << endl;
	cout << "\t\t\tname! -> " << name << endl;
}

ostream &operator<<(ostream &os, variable s)
{
    os << s.name << "<" << s.width << ">";

    return os;
}

