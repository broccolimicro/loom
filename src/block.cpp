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
#include "channel.h"

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

	map<string, state> current_state, change_state;

	list<instruction*>		::iterator	ii;
	map<string, variable*>	::iterator	vi, vj;
	map<string, space>		::iterator	si, sj, sk;
	map<string, state>		::iterator	l, m;
	list<state>				::iterator	a, b;
	map<string, keyword*>	::iterator	t;
	list<list<variable*> >	::iterator	di;
	list<variable*>			::iterator	dvi;
	string					::iterator	i, j;
	size_t								ij, ik;

	list<map<string, state> > :: iterator xi;
	map<string, variable*>				affected;
	list<list<variable*> >				delta_out;
	list<variable*> delta;

	map<string, size_t>					prgm_ctr;
	map<string, string>					prgm_protocol;
	map<string, string>		::iterator	proti;
	map<string, size_t>		::iterator	pi;
	list<size_t>			::iterator  pj;

	size_t								k, p;
	state								tstate;

	string search0, search1, search2;

	bool para		= false;
	bool vdef		= false;
	bool first		= false;

	// Add the initial states into the affected variable list
	// This makes sure that evaluated guards pass through skip blocks
	for (l = init.begin(); l != init.end(); l++)
		if ((vi = vars.find(l->first)) != vars.end())
		{
			affected.insert(pair<string, variable*>(vi->first, vi->second));
			current_state.insert(pair<string, state>(l->first, l->second));
		}

	// Parse the instructions, making sure to stay in the current scope (outside of any bracket/parenthesis)
	int depth[3] = {0};
	ij = 0;
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
					v = new variable(raw_instr, tab);
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
				delta.clear();
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
						if ((l->second.prs) && (l->second.data != vi->second->last.data))
						{
							vj = global.find(l->first.substr(0, l->first.find_first_of(".")));
							if (vj != global.end())
								delta.push_back(vj->second);
						}

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


					for (ii = instrs.begin(), di = delta_out.begin(), k = 0; ii != instrs.end() && di != delta_out.end(); ii++, di++, k++)
					{
						if (k >= states[vi->first].states.size()-1)
						{
							l = (*ii)->result.find(vi->first);
							vj = global.find(vi->first.substr(0, vi->first.find_first_of(".")));
							if (vj != global.end())
							{
								t = types.find(vj->second->type);
								pi = prgm_ctr.find(vj->first);
								proti = prgm_protocol.find(vj->second->name);

								if (vj != global.end())
								{
									if(pi == prgm_ctr.end())
									{
										if (t != types.end() && t->second->kind() == "channel")
										{
											prgm_ctr.insert(pair<string, size_t>(vj->second->name, 0));
											cout << tab << "New Program Counter: " << vj->second->name << " 0" << endl;

										}
									}
									else if (k > pi->second)
									{
										pi->second = k;
										cout << tab << "Increment Program Counter: " << vj->second->name << " " << pi->second << endl;
									}

									if (t != types.end() && t->second->kind() == "channel" && (proti == prgm_protocol.end() || proti->second == "?"))
									{
										search0 = (*ii)->chp;
										while ((p = search0.find(vj->first + ".")) != search0.npos)
											search0 = search0.substr(0, p) + search0.substr(p + vj->first.length() + 1);

										search1 = (*((channel*)t->second)->send.def.instrs.begin())->chp;
										search2 = (*((channel*)t->second)->recv.def.instrs.begin())->chp;
										cout << tab << "Send: " << search0 << " in " << search1 << " " << search1.find(search0) << endl;
										cout << tab << "Recv: " << search0 << " in " << search2 << " " << search2.find(search0) << endl;
										if (search1.find(search0) != search1.npos && search2.find(search0) != search2.npos)
											prgm_protocol.insert(pair<string, string>(vj->second->name, "?"));
										else if (search1.find(search0) != search1.npos)
											prgm_protocol.insert(pair<string, string>(vj->second->name, "send"));
										else if (search2.find(search0) != search2.npos)
											prgm_protocol.insert(pair<string, string>(vj->second->name, "recv"));
										else
											cout << "Ah Hell... How in the name of his noodly goodness did I end up here?" << endl;
									}
								}

								if (l != (*ii)->result.end() && l->second.data != "NA")
									states[vi->first].states.push_back(l->second);
								else if (di->size() > 0 && !states[vi->first].states.rbegin()->prs)
								{

									// Use channel send and recv functions to determine whether or not we need to X out the state
									/*cout << tab << "Maybe X Out: " << vi->first << " " << pi->second << endl;
									if (t != types.end() && t->second->kind() == "channel" && pi != prgm_ctr.end())
									{
										p = pi->second;

										if (proti == prgm_protocol.end())
											cout << "Ok... Another place that has issues" << endl;
										else
										{
											if (proti->second == "send")
												for (pj = waits.begin(), xi = ((channel*)t->second)->recv.def.changes.begin(); pj != waits.end() && p > *pj && xi != ((channel*)t->second)->recv.def.changes.end(); p -= *pj, pj++);
											else if (proti->second == "recv")
												for (pj = waits.begin(), xi = ((channel*)t->second)->send.def.changes.begin(); pj != waits.end() && p > *pj && xi != ((channel*)t->second)->send.def.changes.end(); p -= *pj, pj++);

											cout << "BLARG! " << proti->second << " " << vi->first << endl;
											for (m = xi->begin(); m != xi->end(); m++)
												cout << m->first << " " << m->second << endl;
											m = xi->find(vi->first.substr(vi->first.find_first_of(".")+1));
											if (m != xi->end())
											{
												states[vi->first].states.push_back(*(states[vi->first].states.rbegin()) || m->second);
												cout << m->second << endl;
											}
											else
											{
												states[vi->first].states.push_back(*(states[vi->first].states.rbegin()));
												cout << "Fail" << endl;
											}
										}
									}
									else
									{*/
										states[vi->first].states.push_back(*(states[vi->first].states.rbegin()));
									//	cout << "Fail" << endl;
									//}
								}
								// there is no delta in the output variables or this is an output variable
								else
									states[vi->first].states.push_back(*(states[vi->first].states.rbegin()));

								current_state[vi->first] = *states[vi->first].states.rbegin();
							}
						}
					}
				}

				// This counts wait instructions and figures out
				// what states changes in between wait instructions
				if (instr->kind() == "conditional" || i == chp.end())
				{
					change_state.clear();
					for (vi = affected.begin(); vi != affected.end(); vi++)
					{
						if ((si = states.find(vi->first)) != states.end())
						{
							first = true;
							for (ik = 0, a = si->second.states.begin(); waits.size() > 0 && ik <= (*waits.rbegin()) && a != si->second.states.end(); ik++, a++);
							for (; ((instr->kind() != "conditional" && i == chp.end()) || ik <= ij) && a != si->second.states.end(); ik++, a++)
							{
								tstate = first ? *a : tstate || *a;

								first = false;
							}

							change_state.insert(pair<string, state>(vi->first, tstate));
						}
					}

					if (instr->kind() != "conditional" && i == chp.end())
						ij++;

					changes.push_back(change_state);
					waits.push_back(ij);
				}

				ij++;

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

	cout << tab << chp << endl;
	for(si = states.begin(); si != states.end(); si++)
	{
		cout << tab << si->second << endl;
		result.insert(pair<string, state>(si->first, *(si->second.states.rbegin())));
	}
	cout << tab << "Waits: ";
	for (pj = waits.begin(); pj != waits.end(); pj++)
		cout << *pj << " ";
	cout << endl;

	cout << tab << "Changes: ";
	for (xi = changes.begin(); xi != changes.end(); xi++)
	{
		for (m = xi->begin(); m != xi->end(); m++)
			cout << m->first << ":" << m->second << " ";
		cout << ", ";
	}
	cout << endl;

	cout << tab << "Result:\t";

	for (l = result.begin(); l != result.end(); l++)
		cout << "{" << l->first << " = " << l->second << "} ";
	cout << endl;

	rules = production_rule(states, global, tab);

	list<rule>::iterator ri;
	list<int> state_locations; 	//See where_state_var's return for details of how this is being used.
	list<int>::iterator li;
	list<int> temp;
	for (ri = rules.begin(); ri != rules.end(); ri++){
		cout << tab << *ri << endl;
		cout << tab << "Production rules vs. desired functionality: ";
		temp = where_state_var(ri->left, ri->right, tab);
		state_locations.merge(temp);
	}

	//instruction *state_inst_up, *state_instr_down;
	for (li = state_locations.begin(); li != state_locations.end(); li ++){
		cout << tab << *li << endl;

		cout << tab << *(++li) << endl;
	}
}

list<rule> production_rule(map<string, space> states, map<string, variable*> global, string tab)
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
			cout << tab << "================Production Rule================" << endl;
			f.right = up(si->second[bi0]);
			cout << tab << "+++++++++++++++++++++++++++++++++++++++++++++++" << endl;
			cout << tab << f.right << "\t" << count(f.right) << "\t" << strict_count(f.right) << endl;
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
					cout << tab << "...................Iteration..................." << endl;
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

						cout << tab << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;

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

						cout << tab << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;
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

			cout << tab << "-----------------------------------------------" << endl;
			cout << tab << f.right << "\t" << count(f.right) << "\t" << strict_count(f.right) << endl;
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
					cout << tab << "...................Iteration..................." << endl;
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

						cout << tab << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;

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

						cout << tab << "\t" << tempspace << "\t" << ccount << "/" << mcount << "\t" << scount << "/" << mscount << endl;
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

//There can be four states: 1, 0, X, _ where underscore is empty set and X is full set.
//The return list will be even by design. Each pair is a different state variable. The first
//element of the pair is the index of state up, the second is state down.
//Ex: 0, 1, 5, 7 means sv0 goes high at 0, low at 1, and s2 goes high at 5, and low at 7.
list<int> where_state_var(space left, space right, string tab)
{
	list<state> left_list, right_list;
	list<state>::iterator i,j;
	list<int> state_locations;
	state result;
	string a, b, conflicts = "";
	left_list = left.states;
	right_list = right.states;

	//Loop through all of the production rule states (left) and the corresponding desired functionality (right)
	for (i = left_list.begin(),j = right_list.begin() ; i != left_list.end() && j != right_list.end(); i++, j++)
	{
		if(i->data == "0" && j->data == "0" )
			conflicts += ".";		//Doesn't fire, shouldn't fire. Good.
		else if(i->data == "0" && j->data == "1" ){
			cout << "ERROR: Production rule doesn't fire during a place where it should." << endl;
			conflicts += "E";	//Error fire! Our PRS aren't good enough.
		}
		else if(i->data == "1" && j->data == "0" )
			conflicts += "C";  // Illegal fire (fires when it shouldn't)
		else if(i->data == "1" && j->data == "1" )
			conflicts += "!";  // This fires, and it must keep firing after we after we add a state var
		else if(j->data == "X" )
			conflicts += "."; 	//Don't really care if it fires or not. Ambivalence.
		else{
			cout << "ERROR! State var generate is very confused right now. " << endl;
			conflicts += "E";	//Error fire! Not quite sure how you got here...
		}
	}
	cout << conflicts << endl;
	if(conflicts.find_first_of("C") == conflicts.npos)
	{
		return state_locations;		//No conflicts! We are golden.
	}
	// For now, I am simply going to surround the conflicting state with a state variable. This is probably not optimal.
	size_t st = conflicts.find_first_of("C");
	while (st != conflicts.npos)
	{
		state_locations.push_back(st);
		state_locations.push_back(st+1);
		conflicts = conflicts.substr(0,st) + "." + conflicts.substr(st+1);
		//Get the next conflict
		cout << tab << conflicts << endl;
		st = conflicts.find_first_of("C");
	}
	return state_locations;

}

