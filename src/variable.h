/*
 * variable.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
 */

#include "common.h"

#ifndef variable_h
#define variable_h

/* This structure describes a variable in the chp program. We need
 * to keep track of a variables name, its type (record, keyword, process),
 * and its bit width.
 */
struct variable
{
	variable()
	{
		name = "";
		type = "";
		width = 0;
	}
	variable(string chp)
	{
		parse(chp);
	}
	~variable()
	{
		name = "";
		type = "";
		width = 0;
	}

	string		name;		// the name of the instantiated variable
	string		type;		// the name of the type of the instantiated variable
	uint16_t	width;		// the bit width of the instantiated variable

	variable &operator=(variable v)
	{
		name = v.name;
		type = v.type;
		width = v.width;
		return *this;
	}

	void parse(string chp)
	{
		cout << "\t\tvariable! -> "+chp << endl;

		int width_start = chp.find_first_of("< ");
		int name_start = chp.find_first_of("> ");

		name = chp.substr(chp.find_first_of("> ")+1);
		type = chp.substr(0, width_start);
		if (chp.find_first_of("<>") != chp.npos)
			width = atoi(chp.substr(width_start+1, name_start - (width_start+1)).c_str());

		cout << "\t\t\ttype! -> " << type << endl;
		cout << "\t\t\twidth!-> " << width << endl;
		cout << "\t\t\tname! -> " << name << endl;
	}
};

#endif
