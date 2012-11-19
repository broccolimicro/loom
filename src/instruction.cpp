/*
 * instruction.cpp
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "instruction.h"
#include "common.h"

instruction::instruction()
{
	_kind = "instruction";
}
instruction::instruction(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab)
{
	_kind = "instruction";
	parse(raw, types, vars, init, tab);
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

void instruction::parse(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab)
{
	result.clear();

	chp = raw;

	string::iterator i;
	unsigned long int j, k;
	int name_end;

	string temp;

	list<string> var;
	list<string>::iterator var_iter;
	list<state> val;
	list<state>::iterator val_iter;

	map<string, state>::iterator state_iter;

	cout << tab << "Instruction:\t"+chp << endl;
	// Parse assignment instructions

	//Convert ;var+; to ;var:=1;
	if(chp.find(":=") == chp.npos && chp.find("+") != chp.npos){
		//Note: Leave these debug statements here until we know this doesn't suffer false positives in bizarre cases.
		cout << tab << "Expanding " << chp;
		chp = chp.substr(0,chp.find("+")) + ":=1";
		cout << " to " << chp << endl;
	}
	//Convert ;var-; to ;var:=0;
	if(chp.find(":=") == chp.npos && chp.find("-") != chp.npos){
		//Note: Leave these debug statements here until we know this doesn't suffer false positives in bizarre cases.
		cout << tab << "Expanding " << chp;
		chp = chp.substr(0,chp.find("-")) + ":=0";
		cout << " to " << chp << endl;
	}

	//Identify that this instruction is an assign.
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
			val.push_back(expr_eval(temp.substr(k, j-k),init,tab + "\t"));
			cout << temp.substr(k, j-k) << endl;
			k = j;
		}
		val.push_back(expr_eval(temp.substr(k),init,tab + "\t"));



		for (var_iter = var.begin(), val_iter = val.begin(); var_iter != var.end() && val_iter != val.end(); var_iter++, val_iter++)
			result.insert(pair<string, state>(*var_iter, *val_iter));


	}
	// Parse Communication instructions (send, receive, and probe)
	else if (chp.find_first_of("!?@") != chp.npos)
	{
		//Handled elsewhere. Don't return an error.
	}
	// Parse skip
	else if (chp.find("skip") != chp.npos)
		return;
	// If all else fails, complain to the user.
	else
		cout << "\t\tError: Instruction not handled: "+chp << endl;

	cout << tab << "Result:\t";

	for (state_iter = result.begin(); state_iter != result.end(); state_iter++)
		cout << "{" << state_iter->first << " = " << state_iter->second << "} ";
	cout << endl;
}

state expr_eval(string raw, map<string, state> init, string tab){
	cout << tab <<"Expression: " + raw << endl;
	// Supported operators: + - * / & | << >> == != <= >= < > ~

	//Tested to be fairly functional:
	//Adds, subtracts
	//Multiplies
	//lte gte
	//== !=
	//Ands and Ors
	//Variables
	//Parens

	// TODO:
	//Add negative support for variables before operators besides +- (ex. a*-b should not be a*0-b)
	//Weirdly never bothered to add lt gt
	//Test functionality

	// Weakest binding set, descending into strongest binding.

	// |
	size_t first_or = raw.find("|");

	// &
	size_t first_and = raw.find("&");

	//== !=
	size_t first_equal = raw.find("==");
	size_t first_nequal = raw.find("!=");
	size_t first_comp = raw.npos;
	if((first_equal != raw.npos)&&((first_nequal == raw.npos)||(first_equal < first_nequal))){
		first_comp = first_equal;
	}else if(first_nequal != raw.npos){
		first_comp = first_nequal;
	}

	//<= >=
	size_t first_ltequal = raw.find("<=");
	size_t first_gtequal = raw.find(">=");
	size_t first_ltgtequal = raw.npos;
	if((first_ltequal != raw.npos)&&((first_gtequal == raw.npos)|| (first_ltequal < first_gtequal))){
		first_ltgtequal = first_ltequal;
	}else if(first_gtequal != raw.npos){
		first_ltgtequal = first_gtequal;
	}

	//< >
	//Not yet supported

	// << >>
	size_t first_shift_l = raw.find("<<");
	size_t first_shift_r = raw.find(">>");
	size_t first_shift = raw.npos;
	if((first_shift_l != raw.npos)&&((first_shift_r == raw.npos)||(first_shift_l < first_shift_r))){
		first_shift = first_shift_l;
	}else if(first_shift_r != raw.npos){
		first_shift = first_shift_r;
	}

	//+ -
	size_t first_addsub = raw.find_first_of("+-");

	//* /
	size_t first_muldiv = raw.find_first_of("*/");

	//()
	size_t first_paren = raw.npos;
	size_t last_paren = raw.npos;
	//Find the first matching set of parentheses.
	string::iterator i;
	int depth = 0;
	for (i = raw.begin(); i != raw.end(); i++){
		if (*i == '(' && !depth++){
			first_paren = i - raw.begin();
		}
		else if (*i == ')' && !--depth){
			last_paren = i - raw.begin();
			i = raw.end()-1;
			//cout << tab << "first paren: " << first_paren << " last paren: " << last_paren <<endl;
		}
	}

	//~
	size_t first_not = raw.find_first_of("~");

	//Strongest bind set above, weakening in acsending order.

	state result; 	//The value we are to return.

	//If there are any parentheses, we rip them out and return their value before even considering the rest.
	if(first_paren != raw.npos && last_paren != raw.npos){
		//cout << tab << "Paren parses to " + raw.substr(0,first_paren) + " '(' " + raw.substr(first_paren + 1,last_paren - first_paren -1) + " ')' " +raw.substr(last_paren + 1 ) << endl;
		raw = raw.substr(0,first_paren) + "0b" + expr_eval(raw.substr(first_paren + 1,last_paren - first_paren -1), init,tab + "\t").data + raw.substr(last_paren + 1);
		cout << tab << "Result:  " + raw << endl;
		result = expr_eval(raw,init,tab + "\t");
		return result;
	}

	//Proceed to split instructions based on weakest binding power first.
	//This way, stronger binding operators 'hold on to' their corresponding variables.

	//Any ors?
	if(first_or != raw.npos){
		//cout << tab << "Doing an or between " + raw.substr(0,first_or) + " and " +raw.substr(first_or+1) << endl;
		result = expr_eval(raw.substr(0,first_or),init ,tab + "\t") | expr_eval(raw.substr(first_or+1), init,tab + "\t");
		cout << tab << "Result: " + result.data << endl;
		return result;
	}
	//Any ands?
	if(first_and != raw.npos){
		//cout << tab << "Doing an and between " + raw.substr(0,first_and) + " and " +raw.substr(first_and+1) << endl;
		result = expr_eval(raw.substr(0,first_and), init,tab + "\t") & expr_eval(raw.substr(first_and+1), init,tab + "\t");
		cout << tab << "Result: " + result.data << endl;
		return result;
	}
	//Any comps?
	if(first_comp != raw.npos){
		if(raw[first_comp] == '='){		//It is a '=='!
			//cout << tab << "Doing an == between " + raw.substr(0,first_comp) + " and " +raw.substr(first_comp+2) << endl;
			result = expr_eval(raw.substr(0,first_comp), init,tab + "\t") == expr_eval(raw.substr(first_comp+2), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}else{							//It is a '!='!
			//cout << tab << "Doing an != between " + raw.substr(0,first_comp) + " and " +raw.substr(first_comp+2) << endl;
			result = expr_eval(raw.substr(0,first_comp), init,tab + "\t") != expr_eval(raw.substr(first_comp+2), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}
	}
	//Any ltgtequal?
	if(first_ltgtequal != raw.npos){

		if(raw[first_ltgtequal] == '>'){		//It is a '>='!
			//cout << tab << "Doing an >= between " + raw.substr(0,first_ltgtequal) + " and " +raw.substr(first_ltgtequal+2) << endl;
			result = expr_eval(raw.substr(0,first_ltgtequal), init,tab + "\t") >= expr_eval(raw.substr(first_ltgtequal+2), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}else{							//It is a '<='!
			//cout << tab << "Doing an <= between " + raw.substr(0,first_ltgtequal) + " and " +raw.substr(first_ltgtequal+2) << endl;
			result = expr_eval(raw.substr(0,first_ltgtequal), init,tab + "\t") <= expr_eval(raw.substr(first_ltgtequal+2), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}
	}
	//Any >> <<?
	if(first_shift != raw.npos){
		if(raw[first_shift] == '>'){		//It is a '>>'!
			//cout << tab << "Doing an >> between " + raw.substr(0,first_shift) + " and " +raw.substr(first_shift+2) << endl;
			result = expr_eval(raw.substr(0,first_shift), init,tab + "\t") >> expr_eval(raw.substr(first_shift+2), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}else{							//It is a '<<'!
			//cout << tab << "Doing an << between " + raw.substr(0,first_shift) + " and " +raw.substr(first_shift+2) << endl;
			result = expr_eval(raw.substr(0,first_shift), init,tab + "\t") << expr_eval(raw.substr(first_shift+2), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}
	}
	//Any +/-?
	if(first_addsub != raw.npos){			//There is a plus or a minus!
		if (raw[first_addsub] == '+'){		//It is a plus!
			//recurse splitting the first occurrence of the weakest binder.
			//cout << tab << "Doing an add between " + raw.substr(0,first_addsub) + " and " +raw.substr(first_addsub+1) << endl;
			result = expr_eval(raw.substr(0,first_addsub), init,tab + "\t") + expr_eval(raw.substr(first_addsub+1), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}else{								//It is a minus sign!
			//We need to handle whether this is a minus or a negative
			if(( first_addsub == 0 ) || (raw[first_addsub-1] == '+') || (raw[first_addsub-1] == '-') || (raw[first_addsub-1] == '*') || (raw[first_addsub-1] == '/')){
				raw = raw.substr(0,first_addsub) + "0" + raw.substr(first_addsub);
				return expr_eval(raw, init,tab + "\t");
				// a * -b + c*d
			}else{			//This negative does not need to be 'fixed'
				//recurse splitting the first occurrence of the weakest binder.
				//cout << tab << "Doing an sub between " + raw.substr(0,first_addsub) + " and " +raw.substr(first_addsub+1) << endl;
				result = expr_eval(raw.substr(0,first_addsub), init,tab + "\t") - expr_eval(raw.substr(first_addsub+1), init,tab + "\t");
				cout << tab << "Result: " + result.data << endl;
				return result;
			}
		}
	}
	if(first_muldiv != raw.npos){		//There is a mul or a div!
		if(raw[first_muldiv] == '*'){	//It is a mul!
			//recurse splitting the first occurrence of the weakest binder.
			//cout << tab << "Doing an mul between " + raw.substr(0,first_muldiv) + " and " +raw.substr(first_muldiv+1) << endl;
			result = expr_eval(raw.substr(0,first_muldiv), init,tab + "\t") * expr_eval(raw.substr(first_muldiv+1), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}else{							//It is a div!
			//recurse splitting the first occurrence of the weakest binder.
			//cout << tab << "Doing an div between " + raw.substr(0,first_muldiv) + " and " +raw.substr(first_muldiv+1) << endl;
			result = expr_eval(raw.substr(0,first_muldiv), init,tab + "\t") / expr_eval(raw.substr(first_muldiv+1), init,tab + "\t");
			cout << tab << "Result: " + result.data << endl;
			return result;
		}
	}
	//Any nots?
	if(first_not != raw.npos){
		cout << tab << "Doing a not on " << raw.substr(first_not+1) << endl;
		result =  ~expr_eval(raw.substr(first_not+1), init,tab + "\t");
		cout << tab << "Result: " + result.data << endl;
		return result;
	}

	//If the recursion gets down to here, it means we are at a 'basecase' i.e. a variable or number

	if( ac(raw[0]) ){		//Return a variable name!
		//cout << "Loading a variable!" + raw + " = " << init[raw]<< endl;
		cout << tab << "Result:" + raw + " = " << init[raw]<< endl;
		result = init[raw];
		result.prs = 1;
		return init[raw];
	}else{					//Return a number
		// If this is a multi-bit number, then we need to make sure it is correctly parsed
		if (raw[1] == 'x')			// hexadecimal e.g. 0xFEEDFACE
			raw = hex_to_bin(raw.substr(2));

		else if (raw[1] == 'b')	// binary      e.g. 0b01100110
			raw = raw.substr(2);

		else									// decimal     e.g. 20114
			raw = dec_to_bin(raw);

		result = state(raw, true);
		result.prs = 1;
		return result;

	}

}
