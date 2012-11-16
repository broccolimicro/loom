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

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
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

	instruction *instr; 	// instruction parser
	variable	*v;			// variable instantiation parser

	map<string, state> current_state;

	list<instruction*>		::iterator	ii;
	map<string, variable*>	::iterator	vi;
	map<string, space>		::iterator	si, sj, sk;
	map<string, state>		::iterator	l;
	list<state>				::iterator	a, b;
	map<string, keyword*>	::iterator	t;
	list<bool>				::iterator	di;
	string					::iterator	i, j;

	map<string, variable*>				affected;
	list<bool>							delta_out;
	size_t								k;

	bool delta		= false;
	bool para	= false;
	bool vdef		= false;

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

			instr = NULL;
			// This sub block is a set of parallel sub sub blocks. s0 || s1 || ... || sn
			if (para)
				instr = new parallel(raw_instr, types, global, current_state, tab+"\t");
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')')
				instr = new block(raw_instr.substr(1, raw_instr.length()-2), types, global, current_state, tab+"\t");
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new loop(raw_instr, types, global, current_state, tab+"\t");
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new conditional(raw_instr, types, global, current_state, tab+"\t");
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
					v = new variable(raw_instr, "", tab);
					local.insert(pair<string, variable*>(v->name, v));
					global.insert(pair<string, variable*>(v->name, v));
				}
				// This sub block is an assignment instruction.
				else if (raw_instr.length() != 0)
					instr = new instruction(raw_instr, types, global, current_state, tab+"\t");
			}

			if (instr != NULL)
			{
				instrs.push_back(instr);

				// Now that we have parsed the sub block, we need to
				// check the resulting state space deltas of that sub block.
				// Loop through all of the affected variables.
				delta = false;
				for (l = instr->result.begin(); l != instr->result.end(); l++)
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
						// The first state for every variable is always all X
						if ((l = init.find(vi->first)) != init.end())
						{
							states[vi->first].states.push_back(l->second);
							states[vi->first].var = vi->first;
						}
						else
						{
							states[vi->first].states.push_back(vi->second->reset);
							states[vi->first].var = vi->first;
						}
					}

					for (ii = instrs.begin(), di = delta_out.begin(), k = 0; ii != instrs.end() && di != delta_out.end() && k < states[vi->first].states.size(); ii++, di++, k++);

					for (; ii != instrs.end() && di != delta_out.end(); ii++, di++)
					{
						l = (*ii)->result.find(vi->first);

						if (l != (*ii)->result.end() && l->second.data != "NA")
							states[vi->first].states.push_back(l->second);
						else if ((*di) && !states[vi->first].states.rbegin()->prs)
							states[vi->first].states.push_back(state(string(vi->second->width, 'X'), false));
						// there is no delta in the output variables or this is an output variable
						else
							states[vi->first].states.push_back(*(states[vi->first].states.rbegin()));

						current_state[vi->first] = *states[vi->first].states.rbegin();
					}
				}
			}
			j = i+1;
			para = false;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp string. This is
		// the middle of a parallel sub block.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
			para = true;
	}

	cout << endl;

	for(vi = affected.begin(); vi != affected.end(); vi++)
	{
		cout << tab << states[vi->first] << endl;
		result.insert(pair<string, state>(vi->first, *(states[vi->first].states.rbegin())));
	}

	rules = production_rule(states, global);

	list<rule>::iterator ri;
	for (ri = rules.begin(); ri != rules.end(); ri++)
		cout << *ri << endl;
}

list<rule> production_rule(map<string, space> states, map<string, variable*> global)
{
	// Generate the production rules
	map<string, space> invars;
	map<string, space>		::iterator	si, sj;
	int bi0, bi1, o;
	int scount, ccount;
	int mscount, mcount;
	space tempspace, setspace;
	string invar;
	rule r, f;
	list<rule> rules;
	bool first, found;

	for (si = states.begin(); si != states.end(); si++)
	{
		for (bi0 = 0; bi0 < global.find(si->first)->second->width; bi0++)
		{
			cout << "================Production Rule================" << endl;
			f.right = up(si->second[bi0]);
			cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << endl;
			cout << f.right << "\t" << count(f.right) << "\t" << strict_count(f.right) << endl;
			for (o = 0; o < delta_count(f.right); o++)
			{
				r.clear(si->second.states.size());
				r.right = up(si->second[bi0], o);

				mscount = strict_count(r.right);
				mcount = r.right.states.size() - count(r.right);

				invars.clear();
				for (sj = states.begin(); sj != states.end(); sj++)
					for (bi1 = 0; bi1 < global.find(sj->first)->second->width; bi1++)
						if (sj != si || bi0 != bi1)
							invars.insert(pair<string, space>(sj->first + to_string(bi1), sj->second[bi1]));

				first = true;
				found = true;
				while (invars.size() > 0 && found && count(r.left) > count(r.right))
				{
					cout << "...................Iteration..................." << endl;
					setspace = r.left;

					found = false;
					for (sj = invars.begin(); sj != invars.end(); sj++)
					{
						if (first)
							tempspace = sj->second;
						else
							tempspace = r.left & sj->second;

						scount = strict_count(r.right & tempspace);
						ccount = count(tempspace) - count(r.right & tempspace);

						if (ccount < mcount && scount >= mscount && r.left.var.find(tempspace.var) == r.left.var.npos)
						{
							setspace = tempspace;
							invar = sj->first;
							mcount = ccount;
							mscount = scount;
						}

						cout << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;

						if (first)
							tempspace = ~sj->second;
						else
							tempspace = r.left & (~sj->second);

						scount = strict_count(r.right & tempspace);
						ccount = count(tempspace) - count(r.right & tempspace);

						if (ccount < mcount && scount >= mscount && r.left.var.find(tempspace.var) == r.left.var.npos)
						{
							setspace = tempspace;
							invar = sj->first;
							mcount = ccount;
							mscount = scount;
						}

						cout << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;
					}

					if (r.left.var.find(setspace.var) == r.left.var.npos)
					{
						r.left = setspace;
						invars.erase(invar);
						first = false;
						found = true;
					}
				}

				if (o == 0)
					f = r;
				else
					f.left = f.left | r.left;
			}

			if (delta_count(f.right) > 0)
				rules.push_back(f);


			f.right = down(si->second[bi0]);

			cout << "-----------------------------------------------" << endl;
			cout << f.right << "\t" << count(f.right) << "\t" << strict_count(f.right) << endl;
			for (o = 0; o < delta_count(f.right); o++)
			{
				r.clear(si->second.states.size());
				r.right = down(si->second[bi0], o);

				mscount = strict_count(r.right);
				mcount = r.right.states.size() - count(r.right);

				invars.clear();
				for (sj = states.begin(); sj != states.end(); sj++)
					for (bi1 = 0; bi1 < global.find(sj->first)->second->width; bi1++)
						if (sj != si || bi0 != bi1)
							invars.insert(pair<string, space>(sj->first + to_string(bi1), sj->second[bi1]));

				first = true;
				found = true;
				while (invars.size() > 0 && found && count(r.left) > count(r.right))
				{
					cout << "...................Iteration..................." << endl;
					setspace = r.left;

					found = false;
					for (sj = invars.begin(); sj != invars.end(); sj++)
					{
						if (first)
							tempspace = sj->second;
						else
							tempspace = r.left & sj->second;

						scount = strict_count(r.right & tempspace);
						ccount = count(tempspace) - count(r.right & tempspace);

						if (ccount < mcount && scount >= mscount && r.left.var.find(tempspace.var) == r.left.var.npos)
						{
							setspace = tempspace;
							invar = sj->first;
							mcount = ccount;
							mscount = scount;
						}

						cout << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;

						if (first)
							tempspace = ~sj->second;
						else
							tempspace = r.left & (~sj->second);

						scount = strict_count(r.right & tempspace);
						ccount = count(tempspace) - count(r.right & tempspace);

						if (ccount < mcount && scount >= mscount && r.left.var.find(tempspace.var) == r.left.var.npos)
						{
							setspace = tempspace;
							invar = sj->first;
							mcount = ccount;
							mscount = scount;
						}

						cout << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;
					}

					if (r.left.var.find(setspace.var) == r.left.var.npos)
					{
						r.left = setspace;
						invars.erase(invar);
						first = false;
						found = true;
					}
				}

				if (o == 0)
					f = r;
				else
					f.left = f.left | r.left;
			}

			if (delta_count(f.right) > 0)
				rules.push_back(f);
		}
	}

	return rules;
}



