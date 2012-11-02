/*
 * parallel.cpp
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham
 */

#include "parallel.h"
#include "conditional.h"
#include "loop.h"
#include "block.h"

parallel::parallel()
{
	chp = "";
	_kind = "parallel";
}
parallel::parallel(string raw, map<string, keyword*> types, map<string, variable*> vars, string tab)
{
	_kind = "parallel";
	parse(raw, types, vars, tab);
}
parallel::~parallel()
{
	chp = "";
	_kind = "parallel";

	map<string, variable*>::iterator i;
	for (i = local.begin(); i != local.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	local.clear();
}

void parallel::parse(string raw, map<string, keyword*> types, map<string, variable*> vars, string tab)
{
	cout << tab << "parallel: " << raw << endl;  		// Output the raw parallel

	global = vars;						//The variables this parallel uses.
	chp = raw;

	string raw_instr;							//String of CHP code to be tested as an instruction
	instruction instr; 							//Lists are pass by value, right? Else this wont work
	conditional cond;
	loop		loopcond;
	block		blk;

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

		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
		{
			raw_instr = chp.substr(j-chp.begin(), i-j);
			if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')')
			{
				blk.parse(raw_instr.substr(1, raw_instr.length()-2), types, global, tab+"\t");
				instrs.push_back(blk);
				instr = blk;
			}
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']')			// Loop Block
			{
				loopcond.parse(raw_instr, types, global, tab+"\t");
				instrs.push_back(loopcond);
				instr = loopcond;
			}
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']')		// Conditional Block
			{
				cond.parse(raw_instr, types, global, tab+"\t");
				instrs.push_back(cond);
				instr = cond;
			}
			else if (raw_instr.find(" ") != raw_instr.npos)		// THIS IS WRONG!
			{
				cout << tab << "variable definition!\n";
			}
			else if (raw_instr.length() != 0)					// Assignment Instruction
			{
				instr.parse(raw_instr, types, global, tab+"\t");
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

			j = i+2;
		}
	}

	cout << endl;

	//Turn instructions into states!
	//Remember as we add instructions to X out the appropriate vars when we change "important" inputs
	for(vi = affected.begin(); vi != affected.end(); vi++)
	{
		xstate.prs = false;
		xstate.data = string(vi->second->width, 'X');
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
