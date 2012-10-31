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

block::block()
{
	chp = "";
	_kind = "block";
}
block::block(string raw, map<string, variable*> svars, string tab)
{
	_kind = "block";
	parse(raw, svars, tab);
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

void block::parse(string raw, map<string, variable*> svars, string tab)
{
	cout << tab << "Block: " << raw << endl;  		// Output the raw block

	global = svars;						//The variables this block uses.
	chp = raw;

	string raw_instr;							//String of CHP code to be tested as an instruction
	instruction instr; 							//Lists are pass by value, right? Else this wont work
	conditional cond;
	loop		loopcond;

	list<instruction>::iterator ii;  	//Used later to iterate through instr lists
	map<string, variable*>::iterator vi;
	map<string, state>::iterator l;
	map<string, variable*> affected;
	string::iterator i, j;
	unsigned int k;

	list<bool> delta_out;
	list<bool>::iterator di;
	bool delta;

	state xstate;
	state tstate;

	//Parse instructions!
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

		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == ';' || i == chp.end()))
		{
			raw_instr = chp.substr(j-chp.begin(), i-j);
			if (raw_instr.find("*[") != raw_instr.npos)			// Loop Block
			{
				loopcond.parse(raw_instr, global, tab+"\t");
				instrs.push_back(loopcond);
				instr = loopcond;
			}
			else if (raw_instr.find("[") != raw_instr.npos)		// Conditional Block
			{
				cond.parse(raw_instr, global, tab+"\t");
				instrs.push_back(cond);
				instr = cond;
			}
			else if (raw_instr.find(" ") != raw_instr.npos)		// THIS IS WRONG!
			{
				cout << tab << "variable definition!\n";
			}
			else if (raw_instr.length() != 0)					// Assignment Instruction
			{
				instr.parse(raw_instr, global, tab+"\t");
				instrs.push_back(instr);
			}

			delta = false;
			for (l = instr.result.begin(); l != instr.result.end(); l++)
			{
				vi = global.find(l->first);
				if (vi == global.end() && l->first != "Unhandled")
					cout<< "Error: you are trying to call an instruction that operates on a variable not in this block's scope: " + l->first << endl;
				else if (vi != global.end())
				{
					delta |= ((l->second.prs) && (l->second.data != vi->second->last.data));
					vi->second->last = l->second;
					vi->second->width = max(vi->second->width, (uint16_t)(l->second.data.length()));
					affected.insert(pair<string, variable*>(vi->first, vi->second));
				}
			}
			delta_out.push_back(delta);

			j = i+1;
		}
	}

	cout << endl;

	//Turn instructions into states!
	//Remember as we add instructions to X out the appropriate vars when we change "important" inputs
	for(vi = affected.begin(); vi != affected.end(); vi++)
	{
		xstate.prs = false;
		xstate.data = "";
		for (k = 0; k < vi->second->width; k++)
			xstate.data = xstate.data + "X";
		states[vi->first].states.push_back(xstate);
		states[vi->first].var = vi->first;

		for (ii = instrs.begin(), di = delta_out.begin(); ii != instrs.end() && di != delta_out.end(); ii++, di++)
		{
			for (l = ii->result.begin(); l != ii->result.end(); l++)
			{
				if (l->second.data.length() < xstate.data.length())
				{
					tstate.prs = l->second.prs;
					tstate.data = l->second.data[0];
					for (k = 0; k < xstate.data.length() - l->second.data.length(); k++)
						tstate.data += "0";
					tstate.data += l->second.data;
				}
				else
					tstate = l->second;

				if ((vi->first == l->first) && (l->second.data != "NA"))
					states[vi->first].states.push_back(tstate);
				else if (!(*di) || (states[vi->first].states.rbegin())->prs)
					states[vi->first].states.push_back(*(states[vi->first].states.rbegin()));
				else
					states[vi->first].states.push_back(xstate);
			}

		}

		cout << tab << states[vi->first] << endl;
	}
}
