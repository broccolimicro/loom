/*
 * rule.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "rule.h"
#include "../utility.h"

rule::rule()
{
	this->uid = -1;
	guards[0] = 0;
	guards[1] = 0;
	explicit_guards[0] = 0;
	explicit_guards[1] = 0;
	implicants[0] = svector<petri_index>();
	implicants[1] = svector<petri_index>();
	net = NULL;
	flags = NULL;
}

rule::rule(int uid)
{
	this->uid = uid;
	guards[0] = 0;
	guards[1] = 0;
	explicit_guards[0] = 0;
	explicit_guards[1] = 0;
	implicants[0] = svector<petri_index>();
    implicants[1] = svector<petri_index>();
	net = NULL;
	flags = NULL;
}

rule::rule(int uid, petri_net *g, flag_space *flags, bool bubble)
{
	this->uid = uid;
	this->net = g;
	this->flags = flags;
	implicants[0] = svector<petri_index>();
    implicants[1] = svector<petri_index>();

	if (bubble)
		gen_minterms();
	else
		gen_bubbleless_minterms();
}

rule::rule(sstring u, sstring d, sstring v, petri_net *net, flag_space *flags)
{
	this->net = net;
	this->flags = flags;
	implicants[0] = svector<petri_index>();
    implicants[1] = svector<petri_index>();

	gen_variables(u, net->vars, flags);
	gen_variables(d, net->vars, flags);

	if ((uid = net->vars->get_uid(v)) < 0)
		uid = net->vars->insert(variable(v, "node", 1, 0, false, flags));

	this->guards[1] = canonical(u, net->vars);
	this->guards[0] = canonical(d, net->vars);
	//cout << u << endl << this->guards[1].print(vars) << endl << endl;
	//cout << d << endl << this->guards[0].print(vars) << endl << endl << endl;
}

rule::~rule()
{
	this->uid = -1;
	guards[0] = 0;
	guards[1] = 0;
	explicit_guards[0] = 0;
	explicit_guards[1] = 0;
	implicants[0] = svector<petri_index>();
    implicants[1] = svector<petri_index>();
	net = NULL;
	flags = NULL;
}

rule &rule::operator=(rule r)
{
	uid = r.uid;
	guards[1] = r.guards[1];
	guards[0] = r.guards[0];
	explicit_guards[0] = r.explicit_guards[0];
	explicit_guards[1] = r.explicit_guards[1];
	implicants[0] = r.implicants[0];
    implicants[1] = r.implicants[1];
	net = r.net;
	flags = r.flags;
	return *this;
}

struct reductionhdl
{
	reductionhdl()
	{

	}

	reductionhdl(const reductionhdl &r)
	{
		implicant = r.implicant;
		path = r.path;
		covered = r.covered;
		end = r.end;
		arcs = r.arcs;
	}

	reductionhdl(petri_net *net, petri_index start)
	{
		if (start.is_trans())
		{
			svector<petri_index> i = net->prev(start);
			this->implicant = 1;
			for (int j = 0; j < i.size(); j++)
				this->implicant &= net->at(i[j]).index;

			cout << "Generating Initial State" << endl;
			this->path.push_back(petri_state(net, i, true));
			cout << "Done" << endl;
		}
		else
		{
			this->implicant = net->at(start).index;
			this->path.push_back(petri_state(net, svector<petri_index>(1, start), true));
		}

		this->end.push_back(pair<int, canonical>(0, canonical(1)));

		cout << "LOOK " << this->path.back() << endl;
		this->covered.resize(net->S.size(), false);
	}

	~reductionhdl()
	{

	}

	canonical implicant;
	svector<petri_state> path;
	svector<pair<int, int> > arcs;
	svector<pair<int, canonical> > end;
	svector<bool> covered;

	petri_index &at(int e, int i)
	{
		return path[end[e].first].state[i];
	}

	void increment(int i)
	{
		path.push_back(path[end[i].first]);
		arcs.push_back(pair<int, int>(end[i].first, path.size()-1));
		path[end[i].first].state.sort();
		end[i].first = path.size()-1;
	}

	int duplicate(int i)
	{
		path.push_back(path[end[i].first]);
		arcs.push_back(pair<int, int>(end[i].first, path.size()-1));
		end.push_back(pair<int, canonical>(path.size()-1, end[i].second));
		return end.size()-1;
	}

	void split(int i, petri_index n)
	{
		path.back().state.push_back(path.back().state[i]);
		path.back().state.back() = n;
	}

	reductionhdl &operator=(reductionhdl r)
	{
		implicant = r.implicant;
		path = r.path;
		covered = r.covered;
		arcs = r.arcs;
		end = r.end;
		return *this;
	}
};

void rule::strengthen(int t)
{
	list<reductionhdl> reductions;
	for (int i = 0; i < implicants[t].size(); i++)
		reductions.push_back(reductionhdl(net, implicants[t][i]));

	for (list<reductionhdl>::iterator reduction = reductions.begin(); reduction != reductions.end(); reduction++)
	{
		cout << "Begin Execution" << endl;
		/**
		 * Work backwards from an implicant until we hit a state
		 * in which our production rule should not fire, but does
		 * given the current guard expression. From there we work
		 * forward until we find a transition that separates the
		 * conflicting state from the implicant state. We then use
		 * the greedy algorithm to pick the smallest combination of
		 * values from that transition that will separate the two.
		 *
		 * The use of greedy is only needed when we need to use a
		 * passive transition to separate the two because passive
		 * transitions can be a complex expression and not just one
		 * variable and one value.
		 */
		bool done = false;
		while (!done)
		{
			for (int e = 0; e < reduction->end.size(); e++)
			{
				svector<int> ready_transitions;
				svector<int> ready_places;
				/* Figure out which indices are ready to be moved
				 * and separate them out into places and transitions.
				 */
				for (int i = 0; i < reduction->path.back().state.size(); i++)
				{
					if (reduction->at(e, i).is_place())
					{
						int total = 0;
						for (int k = i; k < reduction->path.back().state.size(); k++)
							if (net->prev(reduction->path.back().state[k])[0] == net->prev(reduction->path.back().state[i])[0])
								total++;

						if (total == net->next(net->prev(reduction->at(e, i))).unique().size())
							ready_places.push_back(i);
					}
					else
						ready_transitions.push_back(i);
				}

				cout << reduction->path.back() << " " << reduction->end[e].second.print(net->vars) << endl;

				reduction->increment(e);

				/*
				 * This is the meat of the whole function. At this point,
				 * we have reached a new state (where all indices are at
				 * a place) and we need to determine whether or not we
				 * need to add a transition to separate this place from
				 * our initial implicant.
				 */
				if (reduction->path.back().is_state())
				{
					/* Check to see if we are done (we have already seen all
					 * of these indices before) and then mark all indices as
					 * seen.
					 */
					done = true;
					for (int i = 0; i < reduction->path.back().state.size(); i++)
						if (!reduction->covered[reduction->at(e, i).idx()])
							done = false;

					for (int i = 0; i < reduction->path.back().state.size(); i++)
						reduction->covered[reduction->at(e, i).idx()] = true;

					/* Check to see if we need to separate this state from the
					 * implicant by looking for a transition to add in.
					 */
					svector<petri_index> o = net->next(reduction->path.back().state).unique();
					bool is_implicant = false;
					for (int i = 0; i < implicants[t].size(); i++)
						for (int j = 0; j < o.size(); j++)
							if (o[j] == implicants[t][i])
								is_implicant = true;

					canonical encoding = net->get_effective_state_encoding(reduction->path.back(), reduction->path.front());

					/* If this state needs to be separated from the implicant state,
					 * work backwards in the path until we find the closest set of
					 * transitions that separate them. Then, use greedy to get the
					 * least number of values needed to separate them.
					 */
					if (!is_implicant && (canonical(uid, 1-t) & reduction->end[e].second & encoding) != 0)
					{
						cout << "Conflict " << encoding.print(net->vars) << endl;
						bool found = false;
						for (int i = reduction->path.size()-2; !found && i > 0; i--)
						{
							cout << "\t" << reduction->path[i] << endl;

							if (reduction->path[i].is_state())
							{
								canonical transitions = 1;
								for (int j = 0; j < reduction->path[i].state.size(); j++)
								{
									o = net->next(reduction->path[i].state[j]);
									svector<petri_index> valid_o;
									for (int k = i-1; k > 0; k--)
										for (int l = 0; l < o.size(); l++)
											if (reduction->path[k].state.find(o[l]) != reduction->path[k].state.end())
												valid_o.push_back(o[l]);
									valid_o.unique();

									if (valid_o.size() > 0)
									{
										canonical transition_term;
										for (int k = 0; k < valid_o.size(); k++)
											transition_term |= net->at(valid_o[k]).index;
										transitions &= transition_term;
									}
								}
								transitions = transitions.hide(uid);

								canonical temp2 = 0;
								found = true;
								for (int l = 0; l < transitions.terms.size() && found; l++)
								{
									svector<int> vlist = transitions.terms[l].vars();
									found = false;
									for (int j = 1; j <= vlist.size() && !found; j++)
									{
										svector<int> comb = first_combination(j);
										do
										{
											canonical temp = reduction->end[e].second;
											canonical legal_check = 0;
											for (int k = 0; k < comb.size(); k++)
											{
												temp &= canonical(vlist[comb[k]], transitions.terms[l].val(vlist[comb[k]]));
												legal_check |= canonical(vlist[comb[k]], 1-transitions.terms[l].val(vlist[comb[k]]));
											}

											bool legal = false;
											for (int k = 0; !legal && k < reduction->implicant.terms.size(); k++)
												if ((legal_check & reduction->end[e].second & reduction->implicant.terms[k]) == 0)
													legal = true;

											if (legal && temp != 0 && (temp & encoding) == 0)
											{
												temp2 |= temp;
												found = true;
											}
										} while (next_combination(vlist.size(), &comb) && !found);
										comb.clear();
									}
									vlist.clear();
								}
								if (found && transitions != 0)
								{
									cout << "\tFound " << temp2.print(net->vars) << endl;
									reduction->end[e].second = temp2;
								}
							}
						}
					}
				}

				/* Transitions are always handled first to ensure that
				 * we will reach a valid state.
				 */
				if (!done && ready_transitions.size() > 0)
				{
					for (int i = 0; i < ready_transitions.size(); i++)
					{
						svector<petri_index> p = net->prev(reduction->at(e, ready_transitions[i]));
						for (int j = p.size()-1; j >= 0; j--)
						{
							if (j > 0)
								reduction->split(ready_transitions[i], p[j]);
							else
								reduction->at(e, ready_transitions[i]) = p[j];
						}
					}
				}
				/* Then places are handled. Every ordering of moving
				 * from a place to a transition creates a new possible
				 * execution.
				 */
				else if (!done && ready_places.size() > 0)
				{
					for (int i = ready_places.size()-1; i >= 0; i--)
					{
						for (int k = ready_places[i]+1; k < reduction->path.back().state.size(); )
						{
							if (net->prev(reduction->path.back().state[k])[0] == net->prev(reduction->path.back().state[ready_places[i]])[0])
							{
								for (int j = i+1; j < ready_places.size(); j++)
									if (ready_places[j] > k)
										ready_places[j]--;

								reduction->path.back().state.erase(reduction->path.back().state.begin() + k);
							}
							else
								k++;
						}

						list<reductionhdl>::iterator curr = reduction;
						if (i > 0)
						{
							reductions.push_back(*reduction);
							curr = prev(reductions.end());
						}

						svector<petri_index> p = net->prev(curr->at(e, ready_places[i]));
						for (int j = p.size()-1; j >= 0; j--)
						{
							if (j > 0)
							{
								reductions.push_back(*curr);
								reductions.back().at(e, ready_places[i]) = p[j];
							}
							else
								curr->at(e, ready_places[i]) = p[j];
						}
					}
				}
			}
		}
	}

	/* Once we are done with the above, there is still one more step.
	 * If you have two rules {x->z-, x&y->z-} and you OR them together,
	 * then you would end up with {x->z-}. This would create a conflict,
	 * causing the {x&y->z-} rule to fire where it shouldn't. If we AND
	 * them together, we would get {x&y->z-}. This would prevent the
	 * {x->z-} rule from firing when it needs to.
	 *
	 * So instead of doing either of these operations, we need to pick
	 * some values that we can AND into each or either rule to separate
	 * them. For example {x->z-, x&y->z-} would turn into {x&w->z-, x&y->z-}
	 * making them now safe to OR, resulting in the rule {x&w|x&y->z-}.
	 *
	 * This does not need to be done if the two rules are exactly equal,
	 * only if information is lost by ORing the two together. To check this,
	 * you need to go back to The Quine McCluskey algorithm to see when it
	 * decides to merge minterms.
	 *
	 * Picking the right information is just a matter of the greedy algorithm.
	 * Look for the smallest combination of values that separates the two
	 * minterms and use that.
	 */
	svector<pair<canonical, canonical> > temp_guards;
	cout << "Result" << endl;
	for (list<reductionhdl>::iterator reduction = reductions.begin(); reduction != reductions.end(); reduction++)
	{
		for (int e = 0; e < reduction->end.size(); e++)
		{
			if (reduction->end[e].second != 1)
			{
				bool found = false;
				for (int i = 0; !found && i < temp_guards.size(); i++)
					if (temp_guards[i].first == reduction->end[e].second)
					{
						temp_guards[i].second &= reduction->implicant;
						found = true;
					}

				if (!found)
				{
					temp_guards.push_back(pair<canonical, canonical>(reduction->end[e].second, reduction->implicant));
					cout << reduction->end[e].second.print(net->vars) << " -> " << net->vars->get_name(uid) << (t == 1 ? "+" : "-") << endl;
				}
			}
		}
	}
	reductions.clear();
	cout << endl;

	for (int j = 0; j < temp_guards.size(); j++)
		for (int k = j+1; k < temp_guards.size(); k++)
		{
			// If the two guards are derived from the same minterm then ORing them together will create a conflict
			// so we need to merge them.
			if ((temp_guards[j].first & temp_guards[j].second) == (temp_guards[k].first & temp_guards[k].second))
			{
				temp_guards[j].first &= temp_guards[k].first;
				temp_guards[k].first = temp_guards[j].first;
			}
		}

	for (int j = 0; j < temp_guards.size(); j++)
		for (int k = j+1; k < temp_guards.size(); k++)
			if (temp_guards[j].first != temp_guards[k].first && mergible(&temp_guards[j].first, &temp_guards[k].first))
			{
				canonical temp = temp_guards[j].first;
				canonical temp2 = temp_guards[k].first;

				if ((temp_guards[j].first | temp_guards[k].first) == temp_guards[j].first)
				{
					canonical temp3 = 0;
					bool found = true;
					for (int l = 0; l < temp.terms.size() && found; l++)
					{
						svector<int> vlist = temp.terms[l].vars();
						found = false;
						for (int m = 1; m <= vlist.size() && !found; m++)
						{
							svector<int> comb = first_combination(m);
							do
							{
								canonical temp4;
								for (int n = 0; n < comb.size(); n++)
									temp4 = (n == 0 ? canonical(vlist[comb[n]], temp.terms[l].val(vlist[comb[n]]))  :
											  temp4 & canonical(vlist[comb[n]], temp.terms[l].val(vlist[comb[n]])) );

								if (!is_mutex(&temp4, &temp_guards[j].first, &temp_guards[j].second) && is_mutex(&temp4, &temp2, &net->vars->enforcements))
								{
									temp3 |= temp4;
									found = true;
								}
							} while (next_combination(vlist.size(), &comb) && !found);
							comb.clear();
						}
						vlist.clear();
					}

					if (!found)
					{
						cerr << "Error: Conflict created during guard strengthening for " << net->vars->get_name(uid) << (t == 0 ? "-" : "+") << "." << endl;
						temp_guards[j].first &= temp_guards[k].first;
					}
					else
						temp_guards[j].first &= temp3;
				}
				else if ((temp_guards[j].first | temp_guards[k].first) == temp_guards[k].first)
				{
					canonical temp3 = 0;
					bool found = true;
					for (int l = 0; l < temp2.terms.size() && found; l++)
					{
						svector<int> vlist = temp2.terms[l].vars();
						found = false;
						for (int m = 1; m <= vlist.size() && !found; m++)
						{
							svector<int> comb = first_combination(m);
							do
							{
								canonical temp4;
								for (int n = 0; n < comb.size(); n++)
									temp4 = (n == 0 ? canonical(vlist[comb[n]], temp2.terms[l].val(vlist[comb[n]]))  :
											  temp4 & canonical(vlist[comb[n]], temp2.terms[l].val(vlist[comb[n]])) );

								if (!is_mutex(&temp4, &temp_guards[k].first, &temp_guards[k].second) && is_mutex(&temp4, &temp, &net->vars->enforcements))
								{
									temp3 |= temp4;
									found = true;
								}
							} while (next_combination(vlist.size(), &comb) && !found);
							comb.clear();
						}
						vlist.clear();
					}

					if (!found)
					{
						cerr << "Error: Conflict created during guard strengthening for " << net->vars->get_name(uid) << (t == 0 ? "-" : "+") << "." << endl;
						temp_guards[k].first &= temp_guards[j].first;
					}
					else
						temp_guards[k].first &= temp3;
				}
				else
				{
					canonical temp3 = 0;
					bool found = true;
					for (int l = 0; l < temp.terms.size() && found; l++)
					{
						svector<int> vlist = temp.terms[l].vars();
						found = false;
						for (int m = 1; m <= vlist.size() && !found; m++)
						{
							svector<int> comb = first_combination(m);
							do
							{
								canonical temp4;
								for (int n = 0; n < comb.size(); n++)
									temp4 = (n == 0 ? canonical(vlist[comb[n]], temp.terms[l].val(vlist[comb[n]]))  :
											  temp4 & canonical(vlist[comb[n]], temp.terms[l].val(vlist[comb[n]])) );

								if (!is_mutex(&temp4, &temp_guards[j].first, &temp_guards[j].second) && is_mutex(&temp4, &temp2, &net->vars->enforcements))
								{
									temp3 |= temp4;
									found = true;
								}
							} while (next_combination(vlist.size(), &comb) && !found);
							comb.clear();
						}
						vlist.clear();
					}

					if (!found)
					{
						cerr << "Error: Conflict created during guard strengthening for " << net->vars->get_name(uid) << (t == 0 ? "-" : "+") <<  "." << endl;
						temp_guards[j].first &= temp_guards[k].first;
					}
					else
						temp_guards[j].first &= temp3;

					temp3 = 0;
					found = true;
					for (int l = 0; l < temp2.terms.size() && found; l++)
					{
						svector<int> vlist = temp2.terms[l].vars();
						found = false;
						for (int m = 1; m <= vlist.size() && !found; m++)
						{
							svector<int> comb = first_combination(m);
							do
							{
								canonical temp4;
								for (int n = 0; n < comb.size(); n++)
									temp4 = (n == 0 ? canonical(vlist[comb[n]], temp2.terms[l].val(vlist[comb[n]]))  :
											  temp4 & canonical(vlist[comb[n]], temp2.terms[l].val(vlist[comb[n]])) );

								if (!is_mutex(&temp4, &temp_guards[k].first, &temp_guards[k].second) && is_mutex(&temp4, &temp, &net->vars->enforcements))
								{
									temp3 |= temp4;
									found = true;
								}
							} while (next_combination(vlist.size(), &comb) && !found);
							comb.clear();
						}
						vlist.clear();
					}

					if (!found)
					{
						cerr << "Error: Conflict created during guard strengthening for " << net->vars->get_name(uid) << (t == 0 ? "-" : "+") << "." <<  endl;
						temp_guards[k].first &= temp_guards[j].first;
					}
					else
						temp_guards[k].first &= temp3;
				}
			}

	for (int j = 0; j < temp_guards.size(); j++)
		guards[t] |= temp_guards[j].first;
}

/* gen_minterms produces the weakest set of implicants that cannot reduce
 * the conflict firing space by adding another variable to a given implicant.
 * This information is stored in the implicants field of rule's guards[1] and guards[0].
 * Note that the implicants are generated in a greedy manner: Each variable
 * added to a given implicant is selected based on which would reduce the number
 * of conflict states the most.
 */
void rule::gen_minterms()
{
	guards[1] = 0;
	guards[0] = 0;
	svector<petri_index> ia;
	svector<int> vl;
	int i, j;
	canonical t;
	svector<bool> covered;
	canonical outside = 0;
	for (i = 0; i < net->S.size(); i++)
		outside |= net->S[i].index;

	outside = ~outside;

	guards[0] |= outside;
	guards[1] |= outside;

	for (i = 0; i < net->T.size(); i++)
	{
		vl = net->T[i].index.vars().unique();
		if (net->T[i].active && vl.find(uid) != vl.end())
		{
			if (net->T[i].index(uid, 1) != 0)
			{
				ia = net->prev(petri_index(i, false));
				for (j = 0, t = 1; j < ia.size(); j++)
				{
					implicants[1].push_back(ia[j]);
					t &= net->at(ia[j]).index;
				}

				t = t.hide(vl);
				guards[1] |= t;

				/*covered.clear();
				cout << "Start " << net->T[i].index.print(vars) <<  " ";
				for (j = 0; j < (int)net->T[i].tail.size(); j++)
					cout << net->T[i].tail[j] << " ";
				cout << endl;
				for (j = 0; j < (int)net->arcs.size(); j++)
					if (net->arcs[j].second == net->trans_id(i))
						guards[1] |= strengthen(j, &covered, canonical(1), t, 1, net->T[i].tail).second;
				cout << endl;*/
			}

			if (net->T[i].index(uid, 0) != 0)
			{
				ia = net->prev(petri_index(i, false));
				for (j = 0, t = 1; j < ia.size(); j++)
				{
					implicants[0].push_back(ia[j]);
					t &= net->at(ia[j]).index;
				}
				t = t.hide(vl);
				guards[0] |= t;

				/*covered.clear();
				cout << "Start " << net->T[i].index.print(vars) <<  " ";
				for (j = 0; j < (int)net->T[i].tail.size(); j++)
					cout << net->T[i].tail[j] << " ";
				cout << endl;
				for (j = 0; j < (int)net->arcs.size(); j++)
					if (net->arcs[j].second == net->trans_id(i))
						guards[0] |= strengthen(j, &covered, canonical(1), t, 0, net->T[i].tail).second;
				cout << endl;*/
			}
		}
	}
}

void rule::gen_bubbleless_minterms()
{
	guards[1] = canonical(0);
	guards[0] = canonical(0);
	svector<petri_index> ia;
	svector<int> vl;
	int i, j;
	canonical t;
	for (i = 0; i < (int)net->T.size(); i++)
	{
		vl.clear();
		net->T[i].index.vars(&vl);
		if (net->T[i].active && find(vl.begin(), vl.end(), uid) != vl.end())
		{
			if (net->T[i].index(uid, 1) != 0)
			{
				ia = net->prev(petri_index(i, false));
				for (j = 0, t = 1; j < (int)ia.size(); j++)
					t = t & net->at(ia[j]).negative;

				t = t.hide(vl);
				guards[1] = guards[1] | t;
			}

			if (net->T[i].index(uid, 0) != 0)
			{
				ia = net->prev(petri_index(i, false));
				for (j = 0, t = 1; j < (int)ia.size(); j++)
					t = t & net->at(ia[j]).positive;

				t = t.hide(vl);
				guards[0] = guards[0] | t;
			}
		}
	}
}

canonical &rule::up()
{
	return guards[1];
}

canonical &rule::down()
{
	return guards[0];
}

// This check assumes that this rule is valid
bool rule::is_combinational()
{
	return ((guards[0] | guards[1]) == 1);
}

void rule::invert()
{
	canonical temp = guards[0];
	guards[0] = guards[1];
	guards[1] = temp;
	temp = explicit_guards[0];
	explicit_guards[0] = explicit_guards[1];
	explicit_guards[1] = temp;
	svector<petri_index> tempimp = implicants[0];
	implicants[0] = implicants[1];
	implicants[1] = tempimp;
}

//Print the rule in the following format:
//guards[1] left -> guards[1] right+
//guards[0] left -> guards[0] right-
void rule::print(ostream &os, sstring prefix)
{
	svector<sstring> names = net->vars->get_names();
	for (int i = 0; i < (int)names.size(); i++)
		names[i] = prefix + names[i];

	if (guards[1] != -1)
		os << guards[1].print_with_quotes(net->vars, prefix) << " -> \"" << names[uid] << "\"+" << endl;
	if (guards[0] != -1)
		os << guards[0].print_with_quotes(net->vars, prefix) << " -> \"" << names[uid] << "\"-" << endl;
}

//Print the rule in the following format:
//guards[1] left -> guards[1] right+
//guards[0] left -> guards[0] right-
ostream &operator<<(ostream &os, rule r)
{
	svector<sstring> names = r.net->vars->get_names();
	if (r.guards[1] != -1)
		os << r.guards[1].print_with_quotes(r.net->vars) << " -> \"" << names[r.uid] << "\"+" << endl;
	if (r.guards[0] != -1)
		os << r.guards[0].print_with_quotes(r.net->vars) << " -> \"" << names[r.uid] << "\"-" << endl;

    return os;
}
