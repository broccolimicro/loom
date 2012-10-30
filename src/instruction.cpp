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
instruction::instruction(string raw)
{
	_kind = "instruction";
	parse(raw);
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

void instruction::parse(string raw)
{
	chp = raw;

	string::iterator i;
	int name_start, name_end;
	int assign_start;

	string var, val;

	if (chp.find(":=") != chp.npos)				//Is it an assignment instruction?
	{
		name_end = chp.find_first_of(" =-!?;:|,*+()[]{}&<>@#");
		var = chp.substr(0,name_end);
		assign_start = chp.find_first_of(":");

		if (chp[assign_start+3] == 'x')
			val = "o" + hex_to_bin(chp.substr(assign_start+4));
		else if (chp[assign_start+3] == 'b')
			val = "o" + chp.substr(assign_start+4);
		else
			val = "o" + dec_to_bin(chp.substr(assign_start+2));

		result.insert(pair<string, string>(var, val));

		cout << "\t\tInstruction:\t"+chp << endl;
	}
	else if (chp.find_first_of("?!@") != chp.npos)
	{

	}
	else if (chp.find("skip") != chp.npos)
		return;
	else
	{
		var = "Unhandled Instruction";
		val = "NA";
		cout << "\t\tError: Instruction not handled: "+chp << endl;
	}

	cout << "\t\t\tVariable affected -> " << var << endl;
	cout << "\t\t\tValue at end -> " << val << endl;

	/*else if(chp.find("->skip") != chp.npos)	//Is it a [G->skip] instruction MULTIGUARD SELECTION STATEMENTS UNHANDLED
	{
		name_start = 0;
		for(i = chp.begin();i != chp.end(); i++)
		{

			if (ac(*i))
			{
				var_affected = chp.substr(i-chp.begin(), chp.find_last_of("-")-(i-chp.begin()));
				break;
			}
			else
				name_start++;
		}


		if(chp.substr(name_start-1,name_start) == "~")
			val_at_end = "i1";
		else
			val_at_end = "i0";

		cout << "\t\tInstruction:  \t "+chp << endl;

	}
	*/
}
