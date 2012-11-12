/*
 * block.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "block.h"
#include "common.h"
#include "conditional.h"
#include "loop.h"
#include "parallel.h"

block::block()
{
	chp = "";
	_kind = "block";
}
block::block(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab)
{
	_kind = "block";
	parse(raw, types, vars, init, tab);
}
block::~block()
{
	chp = "";
	_kind = "block";

	map<string, variable*>::iterator i;
	for (i = local.begin(); i != local.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	local.clear();
}

block &block::operator=(block b)
{
	chp = b.chp;
	instrs = b.instrs;
	states = b.states;
	return *this;
}

void block::parse(string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab)
{
	result.clear();
	local.clear();
	global.clear();
	instrs.clear();
	states.clear();

	cout << tab << "Block: " << raw << endl;

	global = vars;
	chp = raw;

	string		raw_instr;	// chp of a sub block

	instruction instr; 		// instruction parser
	conditional cond;		// conditional parser
	loop		loopcond;	// loop parser
	parallel	para;		// parallel execution parser
	block		blk;		// sequential execution parser
	variable	*v;			// variable instantiation parser

	list<instruction>		::iterator	ii;
	map<string, variable*>	::iterator	vi;
	map<string, space>		::iterator	si, sj, sk;
	map<string, state>		::iterator	l;
	list<state>				::iterator	a, b;
	map<string, keyword*>	::iterator	t;
	list<bool>				::iterator	di;
	string					::iterator	i, j;

	map<string, variable*>				affected;
	list<bool>							delta_out;
	unsigned int						k;

	bool delta		= false;
	bool parallel	= false;
	bool vdef		= false;

	state xstate;
	state tstate;

	for (l = init.begin(); l != init.end(); l++)
		affected.insert(pair<string, variable*>(l->first, vars[l->first]));

	// Parse the instructions, making sure to stay in the current scope (outside of any bracket/parenthesis)
	int depth[3] = {0};
	for (i = chp.begin(), j = chp.begin(); i != chp.end()+1; i++)
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

		// We are in the current scope, and the current character
		// is a semicolon or the end of the chp string. This is
		// the end of a block.
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == ';' || i == chp.end()))
		{
			// Get the block string.
			raw_instr = chp.substr(j-chp.begin(), i-j);

			// This sub block is a set of parallel sub sub blocks. s0 || s1 || ... || sn
			if (parallel)
			{
				para.parse(raw_instr, types, global, tab+"\t");
				instrs.push_back(para);
				instr = para;
			}
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')')
			{
				blk.parse(raw_instr.substr(1, raw_instr.length()-2), types, global, map<string, state>(), tab+"\t");
				instrs.push_back(blk);
				instr = blk;
			}
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']')
			{
				loopcond.parse(raw_instr, types, global, tab+"\t");
				instrs.push_back(loopcond);
				instr = loopcond;
			}
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']')
			{
				cond.parse(raw_instr, types, global, tab+"\t");
				instrs.push_back(cond);
				instr = cond;
			}
			// This sub block is either a variable definition or an assignment instruction.
			else
			{
				vdef = false;
				for (t = types.begin(); t != types.end(); t++)
					if (raw_instr.find(t->first) != raw_instr.npos)
					{
						vdef = true;
						break;
					}

				// This sub block is a variable definition. keyword<bitwidth> name
				if (vdef)
				{
					v = new variable(raw_instr, tab);
					local.insert(pair<string, variable*>(v->name, v));
					global.insert(pair<string, variable*>(v->name, v));
				}
				// This sub block is an assignment instruction.
				else if (raw_instr.length() != 0)
				{
					instr.parse(raw_instr, types, global, tab+"\t");
					instrs.push_back(instr);
				}
			}

			// Now that we have parsed the sub block, we need to
			// check the resulting state space deltas of that sub block.
			// Loop through all of the affected variables.
			delta = false;
			for (l = instr.result.begin(); l != instr.result.end(); l++)
			{
				// If this variable exists, then we check the resultant value against
				// its current bitwidth and adjust the bitwidth to fit the resultant value.
				// We also need to mark whether or not we need to generate a production rule
				// for this instruction.
				vi = global.find(l->first);
				if (vi == global.end() && l->first != "Unhandled")
					cout<< "Error: you are trying to call an instruction that operates on a variable not in this block's scope: " + l->first << endl;
				else if (vi != global.end())
				{
					delta |= ((l->second.prs) && (l->second.data != vi->second->last.data));
					vi->second->last = l->second;
					vi->second->width = max(vi->second->width, (uint16_t)(l->second.data.length()));
					if (affected.find(vi->first) == affected.end())
						affected.insert(pair<string, variable*>(vi->first, vi->second));
				}
			}
			delta_out.push_back(delta);

			j = i+1;
			parallel = false;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp string. This is
		// the middle of a parallel sub block.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
			parallel = true;
	}

	cout << endl;

	// Fill in the state space based upon the recorded delta values from instruction parsing above.
	// Right now, we X out the input variables when an instruction changes an output value. This will
	// have to be modified in the future so that we only X out the input variables depending upon the
	// communication protocol.
	for(vi = affected.begin(); vi != affected.end(); vi++)
	{
		xstate = state(string(vi->second->width, 'X'), false);
		// The first state for every variable is always all X
		if ((l = init.find(vi->first)) != init.end())
		{
			states[vi->first].states.push_back(l->second);
			states[vi->first].var = vi->first;
		}
		else
		{
			states[vi->first].states.push_back(xstate);
			states[vi->first].var = vi->first;
		}
	}

	// Loop through the instructions and add a state for each instruction
	for (ii = instrs.begin(), di = delta_out.begin(); ii != instrs.end() && di != delta_out.end(); ii++, di++)
	{
		// Check to see if this instruction affects this particular variable
		for(vi = affected.begin(); vi != affected.end(); vi++)
		{

			l = ii->result.find(vi->first);

			if (l != ii->result.end() && l->second.data != "NA")
			{

				// If the states of the two variables aren't the same size, then we
				// need to zero extend the smaller one
				if (l->second.data.length() < xstate.data.length())
				{
					tstate.prs = l->second.prs;
					tstate.data = l->second.data[0];
					for (k = 0; k < xstate.data.length() - l->second.data.length(); k++)
						tstate.data += "0";
					if(l->second.data[0] == '='){
						cout << "Expr eval here!" << l->second << endl;
						tstate.data += expr_eval(l->second.data.substr(1)).data;
					}else{
						cout << "No expr eval here." << l->second << endl;
						tstate.data += l->second.data;
					}

				}
				else{
					if(l->second.data[0] == '='){
						cout << "Expr eval here!"<< l->second << endl;
						tstate = expr_eval(l->second.data.substr(1));
					}else{
						cout << "No expr eval here!"<< l->second << endl;
						tstate = l->second.data;
					}
				}

				// this is the state change we are looking for.
				states[vi->first].states.push_back(tstate);
			}
			// we need to X the variable out because there was a delta and this variable
			// is an input variable.
			else if ((*di) && !states[vi->first].states.rbegin()->prs)
				states[vi->first].states.push_back(xstate);
			// there is no delta in the output variables or this is an output variable
			else
				states[vi->first].states.push_back(*(states[vi->first].states.rbegin()));
		}
	}

	for(vi = affected.begin(); vi != affected.end(); vi++)
	{
		cout << tab << states[vi->first] << endl;
		result.insert(pair<string, state>(vi->first, *(states[vi->first].states.rbegin())));
	}

	// Generate the production rules
	string rule;
	string oldstate = "";
	int cmin = 99999999;
	int c;
	int ins_idx;
	int tmp_idx;
	space rspace, tspace;
	bool first = true;

	for (si = states.begin(); si != states.end(); si++)
	{
		ins_idx = 0;
		for (a = si->second.states.begin(); a != si->second.states.end(); a++)
		{
			rspace.states.clear();
			rspace.var = "";
			first = true;
			if (a->prs && a->data != oldstate)
			{
				rule = "->" + si->first;
				if ((*a == state("0",false)).data == "1")
					rule += "-";
				else
					rule += "+";

				cmin = 99999999;
				for (sj = states.begin(); sj != states.end(); sj++)
				{
					if (sj != si)
					{
						for (b = sj->second.states.begin(), tmp_idx = 0; b != sj->second.states.end() && tmp_idx < (ins_idx-1); b++, tmp_idx++);

						tspace = (sj->second == *b);

						if (first)
							c = count(tspace);
						else
							c = count(rspace & tspace);
						if (c > 0 && c < cmin && rspace.var.find(tspace.var) == rspace.var.npos && b->data.find_first_of("X") == b->data.npos)
						{
							cmin = c;
							if (first)
								rspace = tspace;
							else
								rspace = rspace & tspace;
							first = false;
						}
					}
				}
				cout << rspace.var << rule << " " << cmin << endl;
			}
			ins_idx++;
			oldstate = a->data;
		}
		cout << endl;
	}
}


state expr_eval(string raw){

	// TODO:
	//Add paren!
	//Read variables
	//Less than, greater than
	//Add negative support for variables before operators besides +- (ex. a*-b should not be a*0-b)
	//NEEDS TESTING!
	//Test functionality

	// Supported operators: + - * / & | << >> == != <= >= < >
	//Find occurrences in strings!
	// Weakest bind set below
	// |
	int first_or = raw.find("|");

	// &
	int first_and = raw.find("&");

	//== !=
	int first_equal = raw.find("==");
	int first_nequal = raw.find("!=");
	int first_comp = raw.npos;
	if((first_equal != raw.npos)&&(first_equal < first_nequal)){
		first_comp = first_equal;
	}else if(first_nequal != raw.npos){
		first_comp = first_nequal;
	}

	//<= >=
	int first_ltequal = raw.find("<=");
	int first_gtequal = raw.find(">=");
	int first_ltgtequal = raw.npos;
	if((first_ltequal != raw.npos)&&(first_ltequal < first_gtequal)){
		first_ltgtequal = first_ltequal;
	}else if(first_gtequal != raw.npos){
		first_ltgtequal = first_gtequal;
	}

	//< >
	//Not yet supported

	// << >>
	int first_shift_l = raw.find("<<");
	int first_shift_r = raw.find(">>");
	int first_shift = raw.npos;
	if((first_shift_l != raw.npos)&&(first_shift_l < first_shift_r)){
		first_shift = first_shift_l;
	}else if(first_shift_r != raw.npos){
		first_shift = first_shift_r;
	}

	//+ -
	int first_addsub = raw.find_first_of("+-");

	//* /
	int first_muldiv = raw.find_first_of("*/");

	//()
	//Not yet supported

	//Strongest bind set above

	state result; 	//The value we are to return.

	//Any ors?
	if(first_or != raw.npos){
		cout << "Doing an or between " + raw.substr(0,first_or) + " and " +raw.substr(first_or+1) << endl;
		result = expr_eval(raw.substr(0,first_or)) | expr_eval(raw.substr(first_or+1));
		cout << "Result of or is: " + result.data << endl;
		return result;
	}
	//Any ands?
	if(first_and != raw.npos){
		cout << "Doing an and between " + raw.substr(0,first_and) + " and " +raw.substr(first_and+1) << endl;
		result = expr_eval(raw.substr(0,first_and)) & expr_eval(raw.substr(first_and+1));
		cout << "Result of and is: " + result.data << endl;
		return result;
	}
	//Any comps?
	if(first_comp != raw.npos){
		if(raw[first_comp] == '='){		//It is a '=='!
			cout << "Doing an == between " + raw.substr(0,first_comp) + " and " +raw.substr(first_comp+2) << endl;
			result = expr_eval(raw.substr(0,first_comp)) == expr_eval(raw.substr(first_comp+2));
			cout << "Result of == is: " + result.data << endl;
			return result;
		}else{							//It is a '!='!
			cout << "Doing an != between " + raw.substr(0,first_comp) + " and " +raw.substr(first_comp+2) << endl;
			result = expr_eval(raw.substr(0,first_comp)) != expr_eval(raw.substr(first_comp+2));
			cout << "Result of != is: " + result.data << endl;
			return result;
		}
	}
	//Any ltgtequal?
	if(first_ltgtequal != raw.npos){
		if(raw[first_ltgtequal] == '>'){		//It is a '>='!
			cout << "Doing an >= between " + raw.substr(0,first_ltgtequal) + " and " +raw.substr(first_ltgtequal+2) << endl;
			result = expr_eval(raw.substr(0,first_ltgtequal)) >= expr_eval(raw.substr(first_ltgtequal+2));
			cout << "Result of >= is: " + result.data << endl;
			return result;
		}else{							//It is a '<='!
			cout << "Doing an <= between " + raw.substr(0,first_ltgtequal) + " and " +raw.substr(first_ltgtequal+2) << endl;
			result = expr_eval(raw.substr(0,first_ltgtequal)) <= expr_eval(raw.substr(first_ltgtequal+2));
			cout << "Result of <= is: " + result.data << endl;
			return result;
		}
	}
	//Any >> <<?
	/*
	// NOT SUPPORTED! WAITING FOR NED TO MAKE << >> A STATE STATE OPERATION
	if(first_shift != raw.npos){
		if(raw[first_shift] == '>'){		//It is a '>>'!
			cout << "Doing an >> between " + raw.substr(0,first_shift) + " and " +raw.substr(first_shift+2) << endl;
			result = expr_eval(raw.substr(0,first_shift)) >> expr_eval(raw.substr(first_shift+2));
			cout << "Result of >> is: " + result.data << endl;
			return result;
		}else{							//It is a '<<'!
			cout << "Doing an << between " + raw.substr(0,first_shift) + " and " +raw.substr(first_shift+2) << endl;
			result = expr_eval(raw.substr(0,first_shift)) << expr_eval(raw.substr(first_shift+2));
			cout << "Result of << is: " + result.data << endl;
			return result;
		}
	}*/
	//Any +/-?
	if(first_addsub != raw.npos){			//There is a plus or a minus!
		if (raw[first_addsub] == '+'){		//It is a plus!
			//recurse splitting the first occurrence of the weakest binder.
			cout << "Doing an add between " + raw.substr(0,first_addsub) + " and " +raw.substr(first_addsub+1) << endl;
			result = expr_eval(raw.substr(0,first_addsub)) + expr_eval(raw.substr(first_addsub+1));
			cout << "Result of add is: " + result.data << endl;
			return result;
		}else{								//It is a minus sign!
			//We need to handle whether this is a minus or a negative
			if(( first_addsub == 0 ) || (raw[first_addsub-1] == '+') || (raw[first_addsub-1] == '-') || (raw[first_addsub-1] == '*') || (raw[first_addsub-1] == '/')){
				raw = raw.substr(0,first_addsub) + "0" + raw.substr(first_addsub);
				return expr_eval(raw);
				// a * -b + c*d
			}else{			//This negative does not need to be 'fixed'
				//recurse splitting the first occurrence of the weakest binder.
				cout << "Doing an sub between " + raw.substr(0,first_addsub) + " and " +raw.substr(first_addsub+1) << endl;
				result = expr_eval(raw.substr(0,first_addsub)) - expr_eval(raw.substr(first_addsub+1));
				cout << "Result of sub is: " + result.data << endl;
				return result;
			}
		}
	}
	if(first_muldiv != raw.npos){		//There is a mul or a div!
		if(raw[first_muldiv] == '*'){	//It is a mul!
			//recurse splitting the first occurrence of the weakest binder.
			cout << "Doing an mul between " + raw.substr(0,first_muldiv) + " and " +raw.substr(first_muldiv+1) << endl;
			result = expr_eval(raw.substr(0,first_muldiv)) * expr_eval(raw.substr(first_muldiv+1));
			cout << "Result of mul is: " + result.data << endl;
			return result;
		}else{							//It is a div!
			//recurse splitting the first occurrence of the weakest binder.
			cout << "Doing an div between " + raw.substr(0,first_muldiv) + " and " +raw.substr(first_muldiv+1) << endl;
			result = expr_eval(raw.substr(0,first_muldiv)) / expr_eval(raw.substr(first_muldiv+1));
			cout << "Result of div is: " + result.data << endl;
			return result;
		}
	}

	//If the recursion gets down to here, it means we are at a 'basecase' i.e. a variable or number

	if( ac(raw[0]) ){		//Return a variable name!
		cout << "Parsing a variable!" + raw << endl;
		return state(raw, true);
	}else{					//Return a number
		// If this is a multi-bit number, then we need to make sure it is correctly parsed
		if (raw[1] == 'x')			// hexadecimal e.g. 0xFEEDFACE
			raw = hex_to_bin(raw.substr(2));

		else if (raw[1] == 'b')	// binary      e.g. 0b01100110
			raw = raw.substr(2);

		else									// decimal     e.g. 20114
			raw = dec_to_bin(raw);

		cout << "Output of bin conv is " + raw << endl;
		return state(raw, true);

	}

}


