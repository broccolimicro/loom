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
#include "rule.h"

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
	int			idx_instr;

	instruction instr; 		// instruction parser
	conditional cond;		// conditional parser
	loop		loopcond;	// loop parser
	parallel	para;		// parallel execution parser
	block		blk;		// sequential execution parser
	variable	*v;			// variable instantiation parser

	map<string, state> current_state;

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
	idx_instr = 0;
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
				para.parse(raw_instr, types, global, current_state, tab+"\t");
				instrs.push_back(para);
				instr = para;
			}
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')')
			{
				blk.parse(raw_instr.substr(1, raw_instr.length()-2), types, global, current_state, tab+"\t");
				instrs.push_back(blk);
				instr = blk;
			}
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']')
			{
				loopcond.parse(raw_instr, types, global, current_state, tab+"\t");
				instrs.push_back(loopcond);
				instr = loopcond;
			}
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']')
			{
				cond.parse(raw_instr, types, global, current_state, tab+"\t");
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
					instr.parse(raw_instr, types, global, current_state, tab+"\t");
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
					if (affected.find(vi->first) == affected.end())
						affected.insert(pair<string, variable*>(vi->first, vi->second));
				}
			}
			delta_out.push_back(delta);

			// Fill in the state space based upon the recorded delta values from instruction parsing above.
			// Right now, we X out the input variables when an instruction changes an output value. This will
			// have to be modified in the future so that we only X out the input variables depending upon the
			// communication protocol.
			for(vi = affected.begin(); vi != affected.end(); vi++)
			{
				if (states.find(vi->first) == states.end())
				{
					states.insert(pair<string, space>(vi->first, space(vi->first, list<state>())));
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

				for (ii = instrs.begin(), di = delta_out.begin(), k = 0; ii != instrs.end() && di != delta_out.end() && k < states[vi->first].states.size(); ii++, di++, k++);

				for (; ii != instrs.end() && di != delta_out.end(); ii++, di++)
				{
					l = ii->result.find(vi->first);

					if (l != ii->result.end() && l->second.data != "NA")
						states[vi->first].states.push_back(l->second);
					else if ((*di) && !states[vi->first].states.rbegin()->prs)
						states[vi->first].states.push_back(state(string(vi->second->width, 'X'), false));
					// there is no delta in the output variables or this is an output variable
					else
						states[vi->first].states.push_back(*(states[vi->first].states.rbegin()));

					current_state[vi->first] = *states[vi->first].states.rbegin();
				}
			}
			j = i+1;
			parallel = false;

			idx_instr++;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp string. This is
		// the middle of a parallel sub block.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
			parallel = true;
	}

	cout << endl;

	for(vi = affected.begin(); vi != affected.end(); vi++)
	{
		cout << tab << states[vi->first] << endl;
		result.insert(pair<string, state>(vi->first, *(states[vi->first].states.rbegin())));
	}

	// Generate the production rules
	map<string, space> invars;
	int bi0, bi1, o;
	int scount, ccount;
	int mscount, mcount;
	space negspace, posspace;
	space tempspace, setspace;
	string invar;
	rule r, f;
	bool firstpos, firstneg, found;

	for (si = states.begin(); si != states.end(); si++)
	{
		for (bi0 = 0; bi0 < global.find(si->first)->second->width; bi0++)
		{
			for (o = 0; o < delta_count(si->second[bi0]); o++)
			{
				//cout << "================Production Rule================" << endl;
				//cout << si->second[bi0] << "\t" << delta_count(si->second[bi0]) << endl;
				//cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << endl;
				posspace = up(si->second[bi0], o);
				negspace = down(si->second[bi0], o);

				//cout << posspace << "\t" << o << "\t" << count(posspace) << "\t" << strict_count(posspace) << endl;

				r.clear(si->second.states.size());
				r.var = si->first;
				if (global.find(si->first)->second->width > 1)
					r.var += "[" + to_string(bi0) + "]";

				mscount = strict_count(posspace);
				mcount = posspace.states.size() - count(posspace);

				invars.clear();
				for (sj = states.begin(); sj != states.end(); sj++)
					for (bi1 = 0; bi1 < global.find(sj->first)->second->width; bi1++)
						if (sj != si || bi0 != bi1)
							invars.insert(pair<string, space>(sj->first + to_string(bi1), sj->second[bi1]));

				firstpos = true;
				firstneg = true;

				found = true;
				while (invars.size() > 0 && found && count(r.plus) > count(posspace))
				{
					//cout << "...................Iteration..................." << endl;
					setspace = r.plus;

					found = false;
					for (sj = invars.begin(); sj != invars.end(); sj++)
					{
						if (firstpos)
							tempspace = sj->second;
						else
							tempspace = r.plus & sj->second;

						scount = strict_count(posspace & tempspace);
						ccount = count(tempspace) - count(tempspace & posspace);

						if (ccount < mcount && scount >= mscount && r.plus.var.find(tempspace.var) == r.plus.var.npos)
						{
							setspace = tempspace;
							invar = sj->first;
							mcount = ccount;
							mscount = scount;
						}

						//cout << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;

						if (firstpos)
							tempspace = ~sj->second;
						else
							tempspace = r.plus & (~sj->second);

						scount = strict_count(posspace & tempspace);
						ccount = count(tempspace) - count(tempspace & posspace);

						if (ccount < mcount && scount >= mscount && r.plus.var.find(tempspace.var) == r.plus.var.npos)
						{
							setspace = tempspace;
							invar = sj->first;
							mcount = ccount;
							mscount = scount;
						}

						//cout << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;
					}

					if (r.plus.var.find(setspace.var) == r.plus.var.npos)
					{
						r.plus = setspace;
						invars.erase(invar);
						firstpos = false;
						found = true;
					}
				}

				//cout << endl << r.plus.var << " -> " << r.var << "+" << "\t" << r.plus << "\t" << mcount << "/" << posspace.states.size() - count(posspace) << "\t" << mscount << "/" << strict_count(posspace) << endl;

				//cout << "-----------------------------------------------" << endl;

				mscount = strict_count(negspace);
				mcount = negspace.states.size() - count(negspace);

				invars.clear();
				for (sj = states.begin(); sj != states.end(); sj++)
					for (bi1 = 0; bi1 < global.find(sj->first)->second->width; bi1++)
						if (sj != si || bi0 != bi1)
							invars.insert(pair<string, space>(sj->first + to_string(bi1), sj->second[bi1]));

				//cout << negspace << "\t" << count(negspace) << "\t" << strict_count(negspace) << endl;

				found = true;
				while (invars.size() > 0 && found && count(r.minus) > count(negspace))
				{
					//cout << "...................Iteration..................." << endl;

					setspace = r.minus;
					found = false;
					for (sj = invars.begin(); sj != invars.end(); sj++)
					{
						if (firstneg)
							tempspace = sj->second;
						else
							tempspace = r.minus & (sj->second);

						scount = strict_count(negspace & tempspace);
						ccount = count(tempspace) - count(tempspace & negspace);

						if (ccount < mcount && scount >= mscount && r.minus.var.find(tempspace.var) == r.minus.var.npos)
						{
							setspace = tempspace;
							invar = sj->first;
							mcount = ccount;
							mscount = scount;
						}

						//cout << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;

						if (firstneg)
							tempspace = ~sj->second;
						else
							tempspace = r.minus & (~sj->second);

						scount = strict_count(negspace & (tempspace < 1));
						ccount = count(tempspace) - count(tempspace & negspace);

						if (ccount < mcount && scount >= mscount && r.minus.var.find(tempspace.var) == r.minus.var.npos)
						{
							setspace = tempspace;
							invar = sj->first;
							mcount = ccount;
							mscount = scount;
						}

						//cout << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;
					}

					if (r.minus.var.find(setspace.var) == r.minus.var.npos)
					{
						r.minus = setspace;
						invars.erase(invar);
						firstneg = false;
						found = true;
					}
				}

				//cout << endl << r.minus.var << " -> " << r.var << "-" << "\t" << r.minus << "\t" << mcount << "/" << negspace.states.size() - count(negspace) << "\t" << mscount << "/" << strict_count(negspace) << endl;
				if (o == 0)
					f = r;
				else
				{
					f.plus = f.plus | r.plus;
					f.minus = f.minus | r.minus;
				}
			}

			if (delta_count(si->second[bi0]) > 0)
				cout << f << endl;
		}
	}
}



