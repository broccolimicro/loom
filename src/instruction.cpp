/*
 * instruction.cpp
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham
 */

#include "instruction.h"
#include "common.h"

instruction::instruction()
{
	_kind = "instruction";
}
instruction::instruction(string raw, map<string, variable*> svars, string tab)
{
	_kind = "instruction";
	parse(raw, svars, tab);
}
instruction::~instruction()
{
	_kind = "instruction";
}

instruction &instruction::operator=(instruction i)
{
	chp = i.chp;
	result = i.result;
	return *this;
}

string instruction::kind()
{
	return _kind;
}

void instruction::parse(string raw, map<string, variable*> svars, string tab)
{
	chp = raw;

	string::iterator i;
	int name_start, name_end;
	int assign_start;

	string var;
	state val;

	// Parse assignment instructions
	// Currently only handles single variable assignments
	if (chp.find(":=") != chp.npos)
	{
		// Separate the two operands (the variable to be assigned and the value to assign)
		name_end = chp.find_first_of(" =-!?;:|,*+()[]{}&<>@#");
		var = chp.substr(0,name_end);
		assign_start = chp.find_first_of(":");

		// If this is a multi-bit number, then we need to make sure it is correctly parsed
		if (chp[assign_start+3] == 'x')			// hexadecimal e.g. 0xFEEDFACE
			val = state(hex_to_bin(chp.substr(assign_start+4)), true);

		else if (chp[assign_start+3] == 'b')	// binary      e.g. 0b01100110
			val = state(chp.substr(assign_start+4), true);

		else									// decimal     e.g. 20114
			val = state(dec_to_bin(chp.substr(assign_start+2)), true);

		result.insert(pair<string, state>(var, val));

		cout << tab << "Instruction:\t"+chp << endl;
	}
	// Parse Communication instructions (send, receive, and probe)
	// Currently unsupported
	else if (chp.find_first_of("!?@") != chp.npos)
	{

	}
	// Parse skip
	else if (chp.find("skip") != chp.npos)
		return;
	// The I don't know block
	else
	{
		var = "Unhandled Instruction";
		val.data = "NA";
		val.prs = false;
		cout << "\t\tError: Instruction not handled: "+chp << endl;
	}

	cout << tab << "Result:\t" << var << ", " << val << endl;
}
