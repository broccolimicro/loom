/*
 * block.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
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
block::block(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity)
{
	_kind = "block";
	parse(id, raw, types, vars, init, tab, verbosity);
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

void block::parse(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity)
{
	clear();

	if (verbosity >= VERB_PARSE)
		cout << tab << "Block: " << raw << endl;

	global = vars;
	chp = raw;
	uid = id;

	char nid = 'a';

	string		raw_instr;	// chp of a sub block

	instruction *instr; 	// instruction parser
	variable	*v;			// variable instantiation parser

	map<string, state> current_state, change_state;

	list<instruction*>		::iterator	ii, ix;
	map<string, variable*>	::iterator	vi, vj;
	map<string, space>		::iterator	si, sj, sk;
	map<string, state>		::iterator	l, m;
	list<state>				::iterator	a, b;
	map<string, keyword*>	::iterator	t;
	list<list<variable*> >	::iterator	di;
	list<variable*>			::iterator	dvi;
	string					::iterator	i, j;
	size_t								ij, ik;

	list<map<string, state> >			tracer_changes;
	list<map<string, state> > :: iterator xi;
	map<string, variable*>				affected;
	list<list<variable*> >				delta_out;
	list<variable*> delta;

	map<string, size_t>					prgm_start;
	map<string, size_t>					prgm_ctr;
	map<string, string>					prgm_protocol;
	map<string, string>		::iterator	proti;
	map<string, size_t>		::iterator	pi;
	list<rule>				::iterator	ri;
	list<size_t>			::iterator  pj;

	size_t								k, p, n;
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
		// the end of an instruction.
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == ';' || i == chp.end()))
		{
			// Get the instruction string.
			raw_instr = chp.substr(j-chp.begin(), i-j);

			instr = NULL;
			// This sub block is a set of parallel sub sub blocks. s0 || s1 || ... || sn
			if (para)
				instr = new parallel(uid + nid++, raw_instr, types, global, current_state, tab+"\t", verbosity);
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')')
				instr = new block(uid + nid++, raw_instr.substr(1, raw_instr.length()-2), types, global, current_state, tab+"\t", verbosity);
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new loop(uid + nid++, raw_instr, types, global, current_state, tab+"\t", verbosity);
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new conditional(uid + nid++, raw_instr, types, global, current_state, tab+"\t", verbosity);
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
					v = new variable(raw_instr, tab, verbosity);
					local.insert(pair<string, variable*>(v->name, v));
					global.insert(pair<string, variable*>(v->name, v));
				}
				// This sub block is an assignment instruction.
				else if (raw_instr.length() != 0)
					instr = new instruction(uid + nid++, raw_instr, types, global, current_state, tab+"\t", verbosity);
			}

			// Make sure that this wasn't a variable declaration (they don't affect the state space).
			if (instr != NULL)
			{
				instrs.push_back(instr);

				// Now that we have parsed the sub block, we need to
				// check the resulting state space deltas of that sub block.
				// Loop through all of the affected variables.
				delta.clear();
				for (l = instr->result.begin(); l != instr->result.end(); l++)
				{
					// If this variable exists, we mark whether or not we need to
					// generate a production rule for this instruction.
					vi = global.find(l->first);
					if (vi == global.end() && l->first != "Unhandled")
						cout<< "Error: you are trying to call an instruction that operates on a variable not in this block's scope: " + l->first << " " << instr->chp << endl;
					else if (vi != global.end())
					{
						// The delta list is a list of parents of signals that are
						// currently firing. Add these signals to the list.
						if ((l->second.prs) && (l->second.data != vi->second->last.data))
						{
							vj = global.find(l->first.substr(0, l->first.find_first_of(".")));
							if (vj != global.end())
								delta.push_back(vj->second);
						}

						// Set the last value of the variable
						vi->second->last = l->second;

						// And make sure that we keep track of this variable if it has been
						// newly created.
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
					si = states.find(vi->first);

					// If this is the first time we have seen this variable
					// in the state space, then we need to bring it up to speed.
					if (states.find(vi->first) == states.end())
					{
						states.insert(pair<string, space>(vi->first, space(vi->first, list<state>())));
						si = states.find(vi->first);
						// If this variable is in the init list, then we have it's initial value.
						if ((l = init.find(vi->first)) != init.end())
							((space&)si->second).states.push_back(l->second);
						// Otherwise, use this variable's reset value.
						else
							((space&)si->second).states.push_back(vi->second->reset);
					}

					// Now we need to fill in the rest of the state space. Loop through the instructions.
					for (ii = instrs.begin(), di = delta_out.begin(), k = 0; ii != instrs.end() && di != delta_out.end(); ii++, di++, k++)
					{
						// We only need to generate states if we haven't already
						if (k >= ((space&)si->second).states.size()-1)
						{
							// Get the variable and its new value as affected by this instruction
							l = (*ii)->result.find(vi->first);
							vj = global.find(vi->first.substr(0, vi->first.find_first_of(".")));
							if (vj != global.end())
							{
								// Get the variable's type and check to see if
								// it has an associated program counter.
								t = types.find(vj->second->type);
								pi = prgm_ctr.find(vj->first);
								proti = prgm_protocol.find(vj->second->name);

								if (l != (*ii)->result.end())
								{
									// Figure out what this instruction would look like as
									// this variable. For example, an instruction like l.r:=1
									// would turn into r:=1 and [l.a] to [a]
									// We will need this to figure out if this instruction
									// belongs to the send or the receive.
									search0 = (*ii)->chp;
									while ((p = search0.find(vj->first + ".")) != search0.npos)
										search0 = search0.substr(0, p) + search0.substr(p + vj->first.length() + 1);

									// Check to see if this variable has a defined program counter.
									// If it doesn't and this variable is a channel, then we need to
									// initialize a new program counter and add it to the list. If it
									// does, then we need to update its program counter.
									if (pi == prgm_ctr.end())
									{
										if (t != types.end() && t->second->kind() == "channel")
										{
											prgm_ctr.insert(pair<string, size_t>(vj->first, 0));
											prgm_start.insert(pair<string, size_t>(vj->first, k));
											pi = prgm_ctr.find(vj->first);
										}
									}
									else
										pi->second = k - prgm_start.find(vj->first)->second;

									// Check to see if we know which protocol we are tracing, send or receive.
									// If we don't know the protocol, then we need to figure it out by comparing
									// instruction strings at the proper program counter (instruction index)
									if (pi != prgm_ctr.end() && t != types.end() && t->second->kind() == "channel" && (proti == prgm_protocol.end() || proti->second == "?"))
									{
										// TODO This is supposed to support the case where both the send and receive functions have the same
										// first couple instructions, but that feature does not work.
										for (ix = ((channel*)t->second)->send.def.instrs.begin(), n = 0; ix != ((channel*)t->second)->send.def.instrs.end() && n < pi->second; ix++, n++);

										if (ix != ((channel*)t->second)->send.def.instrs.end())
											search1 = (*ix)->chp;
										else
											search1 = (*((channel*)t->second)->send.def.instrs.begin())->chp;

										for (ix = ((channel*)t->second)->recv.def.instrs.begin(), n = 0; ix != ((channel*)t->second)->recv.def.instrs.end() && n < pi->second; ix++, n++);

										if (ix != ((channel*)t->second)->recv.def.instrs.end())
											search2 = (*ix)->chp;
										else
											search2 = (*((channel*)t->second)->recv.def.instrs.begin())->chp;

										if (search1.find(search0) != search1.npos && search2.find(search0) != search2.npos)
											prgm_protocol.insert(pair<string, string>(vj->first, "?"));
										else if (search1.find(search0) != search1.npos)
											prgm_protocol.insert(pair<string, string>(vj->first, "send"));
										else if (search2.find(search0) != search2.npos)
											prgm_protocol.insert(pair<string, string>(vj->first, "recv"));
									}
								}

								proti = prgm_protocol.find(vj->second->name);

								if (l != (*ii)->result.end() && l->second.data != "NA")
									((space&)si->second).states.push_back(l->second);
								else if (di->size() > 0 && !((space&)si->second).states.rbegin()->prs)
								{
									if (verbosity >= VERB_TRACE)
										cout << tab << "Maybe X Out " << vi->first << " " << (*ii)->chp << endl;
									// Use channel send and recv functions to determine whether or not we need to X out the state
									if (t != types.end() && t->second->kind() == "channel" && pi != prgm_ctr.end())
									{
										p = pi->second;

										if (proti != prgm_protocol.end())
										{
											// TODO Protocol tracing only works for flat protocols. This means that the two
											// phase protocol is currently not supported, but the four phase is.
											if (proti->second == "send")
											{
												tracer_changes = ((channel*)t->second)->recv.def.changes;
												for (pj = waits.begin(); pj != waits.end() && prgm_start.find(vj->first)->second >= *pj; pj++);
												for (xi = tracer_changes.begin(); pj != waits.end() && xi != tracer_changes.end() && p >= *pj; pj++, xi++);
											}
											else if (proti->second == "recv")
											{
												tracer_changes = ((channel*)t->second)->send.def.changes;
												for (pj = waits.begin(); pj != waits.end() && prgm_start.find(vj->first)->second >= *pj; pj++);
												for (xi = tracer_changes.begin(); pj != waits.end() && xi != tracer_changes.end() && p >= *pj; pj++, xi++);
											}

											if (xi != tracer_changes.end() && proti->second != "?")
											{
												m = xi->find(vi->first.substr(vi->first.find_first_of(".")+1));
												if (m != xi->end())
												{
													tstate = *(((space&)si->second).states.rbegin()) || m->second;
													tstate.prs = ((space&)si->second).states.rbegin()->prs;
													((space&)si->second).states.push_back(tstate);
												}
												else
													((space&)si->second).states.push_back(((space&)si->second).states.back());
											}
											else
												((space&)si->second).states.push_back(*(((space&)si->second).states.rbegin()));
										}
										else
											((space&)si->second).states.push_back(*(((space&)si->second).states.rbegin()));
									}
									else
										((space&)si->second).states.push_back(*(((space&)si->second).states.rbegin()));
								}
								// there is no delta in the output variables or this is an output variable
								else
									((space&)si->second).states.push_back(((space&)si->second).states.back());

								current_state[vi->first] = ((space&)si->second).states.back();
							}
							else
								cout << "Error: Something is royally fucked up." << endl;
						}
					}
				}

				// This counts wait instructions and figures out
				// what states changes in between wait instructions
				if (instr->kind() == "conditional" || i == chp.end())
				{
					change_state.clear();
					if (instr->kind() != "conditional" && i == chp.end())
						ij++;

					for (vi = affected.begin(); vi != affected.end(); vi++)
					{
						if ((si = states.find(vi->first)) != states.end())
						{
							first = true;
							for (ik = 0, a = ((space&)si->second).states.begin(); waits.size() > 0 && ik <= (*waits.rbegin()) && a != ((space&)si->second).states.end(); ik++, a++);
							for (; ((instr->kind() != "conditional" && i == chp.end()) || ik <= ij) && a != ((space&)si->second).states.end(); ik++, a++)
							{
								tstate = first ? *a : tstate || *a;

								first = false;
							}

							change_state.insert(pair<string, state>(vi->first, tstate));
						}
					}

					waits.push_back(ij);
					changes.push_back(change_state);
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

	change_state.clear();
	for (vi = affected.begin(); vi != affected.end(); vi++)
	{
		l = changes.begin()->find(vi->first);
		si = states.find(vi->first);
		if (l != changes.begin()->end() && si != states.end())
		{
			l->second = l->second || (*((space&)si->second).states.rbegin());
			change_state.insert(pair<string, state>(vi->first, l->second));
		}
		else if (l != changes.begin()->end())
			change_state.insert(pair<string, state>(vi->first, l->second));
		else if (si != states.end())
		{
			changes.begin()->insert(pair<string, state>(vi->first, *((space&)si->second).states.rbegin()));
			change_state.insert(pair<string, state>(vi->first, *((space&)si->second).states.rbegin()));
		}
	}
	changes.push_back(change_state);

	if (verbosity >= VERB_PARSE)
	cout << endl;

	if (verbosity >= VERB_STATES)
		cout << tab << chp << endl;
	for(si = states.begin(); si != states.end(); si++)
	{
		if (verbosity >= VERB_STATES)
			cout << tab << si->second << endl;
		if (local.find(si->first) == local.end())
			result.insert(pair<string, state>(si->first, *(((space&)si->second).states.rbegin())));
	}

	if (verbosity >= VERB_STATES)
	{
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
	}

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "Result:\t";

		for (l = result.begin(); l != result.end(); l++)
			cout << "{" << l->first << " = " << l->second << "} ";
		cout << endl;
	}

	rules = production_rule(instrs, states, tab, verbosity);

	if (!production_rule_check(&raw, this, tab, verbosity))
	{
		//Now that we have the corrected instruction stream, rerun parse!
		if (verbosity >= VERB_STATEVAR)
		{
			for (ri = rules.begin(); ri != rules.end(); ri++)
				cout << tab << *ri << endl;
			if (rules.size() > 0)
				cout << endl;
		}

		if (verbosity >= VERB_STATEVAR)
			cout << tab << "=============Reparse=============" << endl;		//Let the user know we are trashing the above block.
		parse(id, raw, types, vars, init, tab, verbosity);
	}
	else
	{
		if (verbosity >= VERB_PRS)
		{
			for (ri = rules.begin(); ri != rules.end(); ri++)
				cout << tab << *ri << endl;
			if (rules.size() > 0)
				cout << endl;
		}

		if (verbosity >= VERB_STATEVAR)
			cout << tab << "==============Valid==============" << endl;		//Let the user know that the block parsed correctly.
	}
}

/* This function cleans up all of the memory allocated
 * during parsing, and prepares for the next parsing.
 */
void block::clear()
{
	chp = "";
	_kind = "block";

	// Instructions and local variables are dynamically
	// allocated, but everything is static.
	map<string, variable*>::iterator i;
	for (i = local.begin(); i != local.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	result.clear();
	local.clear();
	global.clear();
	instrs.clear();
	states.clear();
	waits.clear();
	changes.clear();
	rules.clear();
}

bool cycle(rule start, rule end, list<rule> *prs)
{
	list<rule>::iterator ri;

	bool result = false;
	if (end.left.var.find(start.right.var.substr(0, start.right.var.length()-1)) != end.left.var.npos)
		result = true;
	else
		for (ri = prs->begin(); ri != prs->end(); ri++)
			if (end.left.var.find(ri->right.var.substr(0, ri->right.var.length() - 1)) != end.left.var.npos)
				result = result || cycle(start, *ri, prs);

	return result;
}

/* This function generates a set of production rules for the given state space and variable space.
 * It uses two primary measurements to help with this: the count, and the strict count. The count
 * is the number of states in a state space that are in the set {1, X}. The strict count is the number
 * of states in a state space that are in the set {1}.
 *
 * This doesn't always do a perfect job. However, it does the best it can given the state space it has.
 * It is the job of the state variable generation algorithm and the handshaking reshuffling algorithms
 * to remove all of the conflicts that this algorithm leaves behind.
 *
 * TODO The handshaking reshuffling algorithm has not yet been completed.
 */
list<rule> production_rule(list<instruction*> instrs, map<string, space> states, string tab, int verbosity)
{
	list<rule> prs;
	list<instruction*>::iterator ii;
	list<rule>::iterator ri, rj;
	map<string, space>::iterator si;
	string v;
	rule r;

	for (ii = instrs.begin(); ii != instrs.end(); ii++)
	{
		for (ri = (*ii)->rules.begin(); ri != (*ii)->rules.end(); ri++)
		{
			for (rj = prs.begin(); rj != prs.end() && rj->right.var != ri->right.var; rj++);

			v = ri->right.var.substr(0, ri->right.var.length()-1);
			si = states.find(v);
			if (rj == prs.end() && si != states.end())
			{
				r.clear(0);
				r.left = expression(ri->left.var, states, tab+"\t", verbosity);
				if (ri->right.var.substr(ri->right.var.length()-1) == "+")
					r.right = up((space&)si->second);
				else
					r.right = down((space&)si->second);

				// TODO I'm not sure if checking against the strict count right now is correct.
				if (strict_count(r.right) > 0)
					prs.push_back(r);
			}
			else if (rj != prs.end())
				rj->left = rj->left | expression(ri->left.var, states, tab+"\t", verbosity);
			else
			{
				r.clear(0);
				r.left = expression(ri->left.var, states, tab+"\t", verbosity);
				for (size_t i = 0; i < states.begin()->second.states.size(); i++)
					r.right.states.push_back(state("0", false));
				r.right.var = ri->right.var;

				prs.push_back(r);

				cout << "Ah Hell... " << v << endl;
			}
		}
	}

	return prs;
}

/* Search a backward in a conflict string starting at a necessary
 * firing for a clean state variable position.
 *
 * TODO I think it is valid to place a state variable transition
 * right before an indistinguishable state, but not after. Right
 * now we make sure we have a distinguishable state both before and
 * after the state variable transition, but I don't think this is necessary.
 */
size_t search_back(string s, size_t offset)
{
	// Find the next necessary firing
	size_t st = s.find_first_of("!", offset+1);
	int mindots = 1, numdots = 0;

	if (st == s.npos)
		return s.npos;

	// Starting at the next necessary firing, loop backward until we hit a
	// conflict or the current necessary firing. We must see at least
	// mindots '.' to have a clean state variable position
	for (size_t ct = st-1; ct > offset && ct < s.length() && ct >= 0; ct--)
	{
		if (s[ct] == 'C')
		{
			// We didn't find enough clean states. We must
			// pass this and keep going
			if (numdots < mindots)
			{
				numdots = 0;
				mindots = 2;
				st = ct;
			}
			// We found a valid position. Loop back until
			// we get as close to the necessary firing as possible
			else
			{
				ct = (st+ct)/2 + 1;
				for (; s[ct+1] != 'C' && s[ct] != '!'; ct++);

				return ct;
			}
		}
		else if (s[ct] == '.')
			numdots++;
	}

	return s.npos;
}

/* Search a forward in a conflict string starting at a necessary
 * firing for a clean state variable position.
 *
 * !.C
 * !..C
 * !...C
 * !....C
 * ect
 *
 *
 * The following cannot happen. If a production rule fires, it must
 * remain firing until another production rule has fired.
 * x+;x- is invalid CHP that will produce !C
 * x+;y+;x- is valid CHP that can only produce !.C
 *
 * !C
 * !CC
 * !CCC
 *
 */
size_t search_front(string s, size_t offset)
{
	// Find the previous necessary firing
	size_t st = s.find_last_of("!", offset-1);
	int mindots = 1, numdots = 0;

	if (st == s.npos)
		return s.npos;

	// Starting at the previous necessary firing, loop until we hit a
	// conflict or the current necessary firing. We must see at least
	// mindots '.' to have a clean state variable position
	for (size_t ct = st; ct < offset && ct < s.length() && ct >= 0; ct++)
	{
		if (s[ct] == 'C')
		{
			// We didn't find enough clean states. We must
			// pass this and keep going
			if (numdots < mindots)
			{
				numdots = 0;
				mindots = 2;
				st = ct;
			}
			// We found a valid position. Loop back until
			// we get as close to the necessary firing as possible
			else
			{
				ct = (st+ct)/2 + 1;
				for (; s[ct-2] != 'C' && s[ct-1] != '!'; ct--);

				return ct;
			}
		}
		else if (s[ct] == '.')
			numdots++;
	}

	return s.npos;
}

/* There can be four states: 1, 0, X, _ where underscore is empty set and X is full set.
 * Each index in the returned list represents the instruction before which there should be
 * a state variable transition.
 *
 * Ex: 3, 5 means sv0 goes high at 3 and sv2 goes high at 5.
 * !.C
 * C.!
 * C..C!
 * !C..C
*/
list<size_t> state_variable_positions(space left, space right, string tab, int verbosity)
{
	list<size_t> state_locations;
	list<size_t>::iterator si;
	size_t i;

	// Generate the conflict string
	string conflict = conflicts(left, right);

	// If no conflicts, we are done.
	if (conflict.find_first_of("C") == conflict.npos)
		return state_locations;

	if (verbosity >= VERB_STATEVAR)
		cout << tab << "Conflict:\t" << conflict << "\t\t" << left << " -> " << right << endl;

	// Loop through all necessary firings
	size_t foundb = conflict.npos, foundf = conflict.npos;
	size_t offset = 0;
	while (offset != conflict.npos)
	{
		// Search backward from the necessary firing
		// for a clean state variable position
		foundb = search_back(conflict, offset);

		if (foundb != conflict.npos && foundf != foundb)
			state_locations.push_back(foundb);

		// Search forward from the necessary firing
		// for a clean state variable position
		offset = conflict.find_first_of("!", offset+1);

		foundf = search_front(conflict, offset);

		if (foundf != conflict.npos && foundf != foundb)
			state_locations.push_back(foundf);
	}

	state_locations.sort();
	state_locations.unique();

	if (verbosity >= VERB_STATEVAR)
	{
		cout << tab << "Resolved:\t";
		for (i = 0, si = state_locations.begin(); i < conflict.length(); i++)
		{
			if (*si == i)
			{
				cout << "S";
				si++;
			}
			cout << conflict[i];
		}
		cout << "\t\t" << left << " -> " << right << endl;
	}

	return state_locations;
}

bool production_rule_check(string *raw, block *b, string tab, int verbosity)
{
	// At this point, block has a set of candidate rules. Check to see if those rules fire at the
	// appropriate times. If not, add state variables and reparse.

	list<rule>::iterator ri;
	list<size_t> state_locations; 	//See state_variable's return for details of how this is being used.
	list<size_t>::iterator li;
	list<size_t> temp;
	variable *v;
	int depth[3] = {0, 0, 0};

	//Loop through all produced rules run state_locations to get a list of indexes to insert state vars.
	for (ri = b->rules.begin(); ri != b->rules.end(); ri++)
	{
		temp = state_variable_positions(ri->left, ri->right, tab, verbosity);
		state_locations.merge(temp);
	}

	//If there are multiple production rules asking for a state variable to be inserted at a particular
	//index, we only need to insert a single state variable. Sort and unique provides a clean list.
	state_locations.sort();
	state_locations.unique();

	int highest_state_name = 0;				// Used for finding a unique name for the state variable
	int how_many_added = 0;					// How many instructions have we added?
	int how_many_inserted = 0;				// How many instructions have been inserted?
	list<pair<string,int> > to_insertl;

	//Loop through our list of desired state variable insertion indices.
	for (li = state_locations.begin(); li != state_locations.end(); li++)
	{
		//Find the lowest variable name not in globals (no name conflicts)
		highest_state_name = 0;
		while (b->global.find(b->uid + "_" + to_string(highest_state_name)) != b->global.end())
			highest_state_name++;

		//Create a new variable with the unique name.
		v = new variable("int<1>" + b->uid + "_" + to_string(highest_state_name), tab, verbosity);
		//Add to globals
		b->global.insert(pair<string, variable*>(v->name, v));
		//Add to locals
		b->local.insert(pair<string, variable*>(v->name, v));

		//Insert the state variable declaration and state variable transition
		to_insertl.push_back(pair<string,int>(v->name+":=1",*li));
		*raw = "int<1>" + v->name + ":=0;" + v->name + ":=0;" + *raw;
		how_many_added+=2;
	}

	list<pair<string,int> >::iterator instr_adderl;
	//Add leading and tailing semicolon to assist instruction counting
	*raw = ";"  + *raw + ";";
	//For every instruction we are to insert into the instruction stream...
	for(instr_adderl = to_insertl.begin(); instr_adderl != to_insertl.end();instr_adderl++)
	{
		unsigned int insertion_location = 0;
		//Loop through to find the semicolon at which we want to insert the current state variable
		//Note that the offset of how_many_added/inserted is required as the instruction size of
		//the instruction stream grows as instructions are added.
		// TODO Clean this up, there was an error where we were simply looking for semicolons in the string instead of paying attention to depth
		depth[0] = 0;
		depth[1] = 0;
		depth[2] = 0;
		for (int counter = 0; counter <= (instr_adderl->second+how_many_inserted+how_many_added) && insertion_location < raw->length(); insertion_location++)
		{
			if (raw->at(insertion_location) == '(')
				depth[0]++;
			else if (raw->at(insertion_location) == '[')
				depth[1]++;
			else if (raw->at(insertion_location) == '{')
				depth[2]++;
			else if (raw->at(insertion_location) == ')')
				depth[0]--;
			else if (raw->at(insertion_location) == ']')
				depth[1]--;
			else if (raw->at(insertion_location) == '}')
				depth[2]--;

			if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && raw->at(insertion_location) == ';')
				counter++;
		}

		if (insertion_location < 0)
			cout << "Error: State variable insertion failed. " << endl;
		else
		{
			*raw = raw->substr(0, insertion_location) + instr_adderl->first + ";" + raw->substr(insertion_location);
			how_many_inserted++;
		}
	}

	//If we added a state variable...
	if (how_many_added > 0)
	{
		*raw = raw->substr(1, raw->length() - 2);
		return false;
	}
	else
		return true;
}
