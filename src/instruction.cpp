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

state expr_eval(string raw){
	cout<< "expreval: " + raw <<endl;
	int first_plus = raw.find_first_of("+");
	int first_minus = raw.find_first_of("-");

	int first_mul = raw.find_first_of("*");
	int first_div = raw.find_first_of("/");

	//This could be done more simply. When Becky doesn't want attention.
	if((first_plus != raw.npos)||(first_minus != raw.npos)){	//We have adds and/or subtracts!
		if ((first_plus != raw.npos)&&(first_minus != raw.npos)){	//We have at least one add and a sub.
			//evaluate left to right...
			if(first_plus < first_minus){
				//recurse splitting the first occurence of the weakest binder.
				return expr_eval(raw.substr(0,first_plus)) + expr_eval(raw.substr(first_plus+1));
			}else{
				//recurse splitting the first occurence of the weakest binder.
				return expr_eval(raw.substr(0,first_minus)) + expr_eval(raw.substr(first_minus+1));
			}
		}else{			//We only have adds or subs.
			if(first_plus != raw.npos){		//We only have adds
				return expr_eval(raw.substr(0,first_plus)) + expr_eval(raw.substr(first_plus+1));
			}else{				//We only have subs.
				return expr_eval(raw.substr(0,first_minus)) + expr_eval(raw.substr(first_minus+1));
			}
		}
	}

	if((first_mul != raw.npos)||(first_div != raw.npos)){	//We have muls and/or divs!
		if ((first_mul != raw.npos)&&(first_div != raw.npos)){	//We have at least one mul and one div.
			//evaluate left to right...
			if(first_mul < first_div){
				//recurse splitting the first occurence of the weakest binder.
				return expr_eval(raw.substr(0,first_mul)) + expr_eval(raw.substr(first_mul+1));
			}else{
				//recurse splitting the first occurence of the weakest binder.
				return expr_eval(raw.substr(0,first_div)) + expr_eval(raw.substr(first_div+1));
			}
		}else{			//We only have muls or divs.
			if(first_mul != raw.npos){		//We only have muls
				return expr_eval(raw.substr(0,first_mul)) + expr_eval(raw.substr(first_mul+1));
			}else{				//We only have divs.
				return expr_eval(raw.substr(0,first_div)) + expr_eval(raw.substr(first_div+1));
			}
		}
	}

	return state(raw, true);


}

void instruction::parse(string raw, map<string, variable*> svars, string tab)
{
	chp = raw;

	string::iterator i;
	unsigned long int j, k;
	int name_end;
	int assign_start;

	string temp;

	list<string> var;
	list<string>::iterator var_iter;
	list<state> val;
	list<state>::iterator val_iter;

	map<string, state>::iterator state_iter;

	// Parse assignment instructions
	// Currently only handles single variable assignments
	if (chp.find(":=") != chp.npos)
	{
		// Separate the two operands (the variable to be assigned and the value to assign)
		name_end = chp.find(":=");
		temp = chp.substr(0,name_end);
		for (j = temp.find_first_of(","), k = 0; j != temp.npos; j = temp.find_first_of(",", j+1))
		{
			var.push_back(temp.substr(k, j-k));
			k = j;
		}
		var.push_back(temp.substr(k));

		temp = chp.substr(name_end+2);
		for (j = temp.find_first_of(","), k = 0; j != temp.npos; j = temp.find_first_of(",", j+1))
		{
			val.push_back(expr_eval(temp.substr(k, j-k)));
			cout << temp.substr(k, j-k) << endl;
			k = j;
		}
		val.push_back(expr_eval(temp.substr(k)));
		cout << temp.substr(k) << endl;

		/*
		// If this is a multi-bit number, then we need to make sure it is correctly parsed
		if (chp[assign_start+3] == 'x')			// hexadecimal e.g. 0xFEEDFACE
			val = state(hex_to_bin(chp.substr(assign_start+4)), true);

		else if (chp[assign_start+3] == 'b')	// binary      e.g. 0b01100110
			val = state(chp.substr(assign_start+4), true);

		else									// decimal     e.g. 20114
			val = state(dec_to_bin(chp.substr(assign_start+2)), true);
		*/

		for (var_iter = var.begin(), val_iter = val.begin(); var_iter != var.end() && val_iter != val.end(); var_iter++, val_iter++)
			result.insert(pair<string, state>(*var_iter, *val_iter));

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
		cout << "\t\tError: Instruction not handled: "+chp << endl;

	cout << tab << "Result:\t";

	for (state_iter = result.begin(); state_iter != result.end(); state_iter++)
		cout << "{" << state_iter->first << " = " << state_iter->second << "} ";
	cout << endl;
}


