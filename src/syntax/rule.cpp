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

void merge_guards(canonical &guard0, canonical implicant0, canonical &guard1, canonical implicant1)
{
	// If the two guards are derived from the same minterm then ORing them together will create a conflict
	// so we need to merge them.
	if ((guard0 & implicant0) == (guard1 & implicant1))
	{
		guard0 &= guard1;
		guard1 = guard0;
	}
	else if (guard0 != guard1 && mergible(&guard0, &guard1))
	{
		canonical temp = guard0;
		canonical temp2 = guard1;

		if ((guard0 | guard1) == guard0)
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

						if (!is_mutex(&temp4, &guard0, &implicant0) && is_mutex(&temp4, &temp2))
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
				guard0 &= guard1;
			else
				guard0 &= temp3;
		}
		else if ((guard0 | guard1) == guard1)
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

						if (!is_mutex(&temp4, &guard1, &implicant1) && is_mutex(&temp4, &temp))
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
				guard1 &= guard0;
			else
				guard1 &= temp3;
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

						if (!is_mutex(&temp4, &guard0, &implicant0) && is_mutex(&temp4, &temp2))
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
				guard0 &= guard1;
			else
				guard0 &= temp3;

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

						if (!is_mutex(&temp4, &guard1, &implicant1) && is_mutex(&temp4, &temp))
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
				guard1 &= guard0;
			else
				guard1 &= temp3;
		}
	}
}

reductionhdl::reductionhdl()
{

}

reductionhdl::reductionhdl(const reductionhdl &r)
{
	implicant = r.implicant;
	path = r.path;
	guard = r.guard;
	covered = r.covered;
}

reductionhdl::reductionhdl(petri_net *net, petri_index start)
{
	if (start.is_trans())
	{
		svector<petri_index> i = net->prev(start);
		implicant = 1;
		for (int j = 0; j < i.size(); j++)
		{
			implicant &= net->at(i[j]).index;
		}

		petri_state init(net, i, false);
		for (int k = 0; k < init.state.size(); k++)
			path.push_back(pair<svector<petri_index>, bool>(svector<petri_index>(1, init.state[k]), false));
	}
	else
	{
		implicant = net->at(start).index;
		petri_state init(net, svector<petri_index>(1, start), false);
		for (int k = 0; k < init.state.size(); k++)
			path.push_back(pair<svector<petri_index>, bool>(svector<petri_index>(1, init.state[k]), false));
	}
	guard = 1;
	covered.resize(net->S.size(), false);
}

reductionhdl::~reductionhdl()
{

}

reductionhdl &reductionhdl::operator=(reductionhdl r)
{
	implicant = r.implicant;
	path = r.path;
	guard = r.guard;
	covered = r.covered;
	return *this;
}

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

bool rule::separate(reductionhdl &reduction, int t)
{
	petri_state observer;
	petri_state conflict;

	for (int i = 0; i < reduction.path.size(); i++)
	{
		if (reduction.path[i].second)
			conflict.state.push_back(reduction.path[i].first.back());

		observer.state.push_back(reduction.path[i].first.front());
	}
	list<petri_state> executions(1, conflict);

	canonical encoding = net->get_effective_state_encoding(executions.back(), observer);

	cout << "Conflict " << observer << " <-- " << conflict << "{" << encoding.print(net->vars) << "}" << endl;
	bool found = false;
	while (!found && executions.size() > 0)
	{
		for (list<petri_state>::iterator exec = executions.begin(); !found && exec != executions.end(); )
		{
			if (exec->is_state())
			{
				canonical transitions = 1;
				for (int j = 0; j < exec->state.size(); j++)
				{
					svector<petri_index> o = net->next(exec->state[j]);
					svector<petri_index> valid_o;
					for (int l = 0; l < o.size(); l++)
					{
						bool valid = false;
						for (int k = 0; !valid && k < reduction.path.size(); k++)
							for (int m = reduction.path[k].first.size()-1; !valid && m >= 0; m--)
								if (reduction.path[k].first[m] == o[l])
									valid = true;

						if (valid)
						{
							valid_o.push_back(o[l]);
							cout << "\t\t" << o[l] << endl;
						}
					}

					if (valid_o.size() > 0)
					{
						canonical transition_term;
						for (int k = 0; k < valid_o.size(); k++)
							transition_term |= net->at(valid_o[k]).index;
						transitions &= transition_term;
					}
				}
				transitions = transitions.hide(uid);
				cout << "\t\tT=" << transitions.print(net->vars) << endl;

				canonical temp2 = 0;
				found = (transitions != 0);
				for (int l = 0; l < transitions.terms.size() && found; l++)
				{
					svector<int> vlist = transitions.terms[l].vars();
					found = false;
					for (int j = 1; j <= vlist.size() && !found; j++)
					{
						svector<int> comb = first_combination(j);
						do
						{
							canonical temp = reduction.guard;
							canonical legal_check = 0;
							for (int k = 0; k < comb.size(); k++)
							{
								temp &= canonical(vlist[comb[k]], transitions.terms[l].val(vlist[comb[k]]));
								legal_check |= canonical(vlist[comb[k]], 1-transitions.terms[l].val(vlist[comb[k]]));
							}

							bool legal = false;
							for (int k = 0; !legal && k < reduction.implicant.terms.size(); k++)
								if ((legal_check & reduction.guard & reduction.implicant.terms[k]) == 0)
									legal = true;

							cout << "Testing " << temp.print(net->vars) << " " << (temp & encoding).print(net->vars) << endl;
							if (legal && temp != 0 && (canonical(uid, 1-t) & temp & encoding) == 0)
							{
								temp2 |= temp;
								cout << legal << " " << temp.print(net->vars) << " " << temp2.print(net->vars) << " " << legal_check.print(net->vars) << endl;
								found = true;
							}
						} while (next_combination(vlist.size(), &comb) && !found);
						comb.clear();
					}
					vlist.clear();
				}

				if (found)
				{
					cout << "\tFound " << temp2.print(net->vars) << endl;
					reduction.guard = temp2;
				}
			}

			svector<pair<int, svector<petri_index> > > ready_places;
			svector<pair<int, svector<petri_index> > > ready_transitions;
			for (int i = 0; !found && i < exec->state.size(); i++)
			{
				svector<petri_index> n = net->next(exec->state[i]);
				if (observer.state.find(exec->state[i]) == observer.state.end())
				{
					if (exec->state[i].is_place())
					{
						int total = 0;
						for (int k = i; k < exec->state.size(); k++)
							if (net->next(exec->state[k])[0] == n[0])
								total++;

						if (total == net->prev(n).unique().size())
						{
							ready_places.push_back(pair<int, svector<petri_index> >(i, svector<petri_index>()));

							for (int j = 0; j < n.size(); j++)
							{
								bool valid = false;
								for (int k = 0; !valid && k < reduction.path.size(); k++)
									if (reduction.path[k].first.find(n[j]) != reduction.path[k].first.end())
										valid = true;

								if (valid)
									ready_places.back().second.push_back(n[j]);
							}

							if (ready_places.back().second.size() == 0)
								ready_places.pop_back();
						}
					}
					else
					{
						ready_transitions.push_back(pair<int, svector<petri_index> >(i, svector<petri_index>()));

						for (int j = 0; j < n.size(); j++)
						{
							bool valid = false;
							for (int k = 0; !valid && k < reduction.path.size(); k++)
								if (reduction.path[k].first.find(n[j]) != reduction.path[k].first.end())
									valid = true;

							if (valid)
								ready_transitions.back().second.push_back(n[j]);
						}

						if (ready_transitions.back().second.size() == 0)
							ready_transitions.pop_back();
					}
				}
			}

			//cout << exec->state << " " << exec->trans.print(vars) << endl;

			if (!found && ready_transitions.size() > 0)
			{
				cout << "\t" << *exec << endl;
				for (int i = ready_transitions.size()-1; i >= 0; i--)
				{
					for (int j = ready_transitions[i].second.size()-1; j >= 0; j--)
					{
						if (j > 0)
							exec->state.push_back(ready_transitions[i].second[j]);
						else
							exec->state[ready_transitions[i].first] = ready_transitions[i].second[j];
					}
				}
				exec++;
			}
			else if (!found && ready_places.size() > 0)
			{
				cout << "\t" << *exec << endl;
				for (int i = ready_places.size()-1; i >= 0; i--)
				{
					list<petri_state>::iterator curr = exec;
					if (i > 0)
					{
						executions.push_back(*exec);
						curr = prev(executions.end());
					}

					for (int k = ready_places[i].first+1; k < exec->state.size(); )
					{
						if (net->next(exec->state[k])[0] == net->next(exec->state[ready_places[i].first])[0])
						{
							for (int j = i+1; j < ready_places.size(); j++)
								if (ready_places[j].first > k)
									ready_places[j].first--;

							exec->state.erase(exec->state.begin() + k);
						}
						else
							k++;
					}

					for (int j = ready_places[i].second.size()-1; j >= 0; j--)
					{
						if (j > 0)
						{
							executions.push_back(*curr);
							executions.back().state[ready_places[i].first] = ready_places[i].second[j];
						}
						else
							curr->state[ready_places[i].first] = ready_places[i].second[j];
					}
				}
				exec++;
			}
			else
			{
				exec = executions.erase(exec);
				cout << "\tExecution Done" << endl;
			}
		}
	}

	if (!found)
	{
		cerr << "Error: Guard Strengthening found a state " << conflict << " that conflicts with the implicant state " << observer << ". Please separate these with a state variable." << endl;
		return false;
	}

	return true;
}

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
		bool stuck = false;
		while (!done)
		{
			cout << "{";
			for (int i = 0; i < reduction->path.size(); i++)
			{
				if (i != 0)
					cout << " ";
				cout << reduction->path[i].first.back();
			}
			cout << "} " << reduction->guard.print(net->vars) << endl;

			bool conflict = true;
			for (int i = 0; i < reduction->path.size(); i++)
				if (!reduction->path[i].second)
					conflict = false;

			conflict = conflict || stuck;

			bool success = true;
			if (conflict)
			{
				success = separate(*reduction, t);
				stuck = false;
			}

			/*
			 * This is the meat of the whole function. At this point,
			 * we have reached a new state (where all indices are at
			 * a place) and we need to determine whether or not we
			 * need to add a transition to separate this place from
			 * our initial implicant.
			 */
			done = true;
			for (int i = 0; i < reduction->path.size(); i++)
			{
				if (conflict && !success)
					reduction->path[i].second = false;
				else if (((conflict && success) || !reduction->path[i].second) && reduction->path[i].first.back().is_place())
				{
					/* Check to see if we need to separate this state from the
					 * implicant by looking for a transition to add in.
					 */
					svector<petri_index> o = net->next(reduction->path[i].first.back()).unique();
					bool is_implicant = false;
					for (int j = 0; !is_implicant && j < implicants[t].size(); j++)
					{
						if (net->are_parallel_siblings(implicants[t][j], reduction->path[i].first.back()) != -1)
							is_implicant = true;

						for (int k = 0; !is_implicant && k < o.size(); k++)
							if (o[k] == implicants[t][j])
								is_implicant = true;
					}

					canonical encoding = net->get_effective_place_encoding(reduction->path[i].first.back(), reduction->path[i].first.front());

					cout << "Checking " << reduction->path[i].first.back() << " " << conflict << " " << success << " " << is_implicant << " " << reduction->path[i].second << " " << encoding.print(net->vars) << " " << (canonical(uid, 1-t) & reduction->guard & encoding).print(net->vars) << endl;

					/* If this state needs to be separated from the implicant state,
					 * work backwards in the path until we find the closest set of
					 * transitions that separate them. Then, use greedy to get the
					 * least number of values needed to separate them.
					 */
					if (!is_implicant && (canonical(uid, 1-t) & reduction->guard & encoding) != 0)
						reduction->path[i].second = true;
					else
						reduction->path[i].second = false;
				}

				if (reduction->path[i].first.back().is_place() && !reduction->path[i].second)
				{
					/* Check to see if we are done (we have already seen all
					 * of these indices before) and then mark all indices as
					 * seen.
					 */
					if (!reduction->covered[reduction->path[i].first.back().idx()])
						done = false;

					reduction->covered[reduction->path[i].first.back().idx()] = true;
				}
				else
					done = false;
			}

			svector<int> ready_transitions;
			svector<int> ready_places;
			/* Figure out which indices are ready to be moved
			 * and separate them out into places and transitions.
			 */
			for (int i = 0; i < reduction->path.size(); i++)
			{
				svector<petri_index> n = net->prev(reduction->path[i].first.back());
				if (!reduction->path[i].second && reduction->path[i].first.back().is_place())
				{
					int total = 0;
					for (int k = i; k < reduction->path.size(); k++)
						if (!reduction->path[k].second && net->prev(reduction->path[k].first.back())[0] == n[0])
							total++;

					if (total == net->next(n).unique().size())
						ready_places.push_back(i);
				}
				else if (reduction->path[i].first.back().is_trans())
					ready_transitions.push_back(i);
			}

			/* Transitions are always handled first to ensure that
			 * we will reach a valid state.
			 */
			if (!done && ready_transitions.size() > 0)
			{
				for (int i = 0; i < ready_transitions.size(); i++)
				{
					svector<petri_index> p = net->prev(reduction->path[ready_transitions[i]].first.back());
					for (int j = p.size()-1; j >= 0; j--)
					{
						if (j > 0)
						{
							reduction->path.push_back(reduction->path[ready_transitions[i]]);
							reduction->path.back().first.push_back(p[j]);
						}
						else
							reduction->path[ready_transitions[i]].first.push_back(p[j]);
					}
				}
			}
			/* Then places are handled. Every ordering of moving
			 * from a place to a transition creates a new possible
			 * execution.
			 */
			else if (!done && ready_places.size() > 0)
			{
				for (int i = 0; i < ready_places.size(); i++)
				{
					for (int k = ready_places[i]+1; k < reduction->path.size(); )
					{
						if (!reduction->path[k].second && net->prev(reduction->path[k].first.back())[0] == net->prev(reduction->path[ready_places[i]].first.back())[0])
						{
							for (int j = i+1; j < ready_places.size(); j++)
								if (ready_places[j] > k)
									ready_places[j]--;

							reduction->path[ready_places[i]].first.merge(reduction->path[k].first);
							reduction->path.erase(reduction->path.begin() + k);
						}
						else
							k++;
					}

					svector<petri_index> p = net->prev(reduction->path[ready_places[i]].first.back());
					for (int j = p.size()-1; j >= 0; j--)
					{
						if (j > 0)
						{
							reductions.push_back(*reduction);
							reductions.back().path[ready_places[i]].first.push_back(p[j]);
						}
						else
							reduction->path[ready_places[i]].first.push_back(p[j]);
					}
				}
			}
			else
				stuck = true;
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
		if (reduction->guard != 1)
		{
			bool found = false;
			for (int i = 0; !found && i < temp_guards.size(); i++)
				if (temp_guards[i].first == reduction->guard)
				{
					temp_guards[i].second &= reduction->implicant;
					found = true;
				}

			if (!found)
			{
				temp_guards.push_back(pair<canonical, canonical>(reduction->guard, reduction->implicant));
				cout << reduction->guard.print(net->vars) << " -> " << net->vars->get_name(uid) << (t == 1 ? "+" : "-") << endl;
			}
		}
	}
	reductions.clear();
	cout << endl;

	for (int j = 0; j < temp_guards.size(); j++)
		for (int k = j+1; k < temp_guards.size(); k++)
			merge_guards(temp_guards[j].first, temp_guards[j].second, temp_guards[k].first, temp_guards[k].second);

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
