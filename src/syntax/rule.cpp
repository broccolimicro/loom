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
	implicants[0] = svector<int>();
	implicants[1] = svector<int>();
	vars = NULL;
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
	implicants[0] = svector<int>();
    implicants[1] = svector<int>();
	vars = NULL;
	net = NULL;
	flags = NULL;
}

rule::rule(int uid, petri *g, variable_space *v, flag_space *flags, bool bubble)
{
	this->uid = uid;
	this->vars = v;
	this->net = g;
	this->flags = flags;
	implicants[0] = svector<int>();
    implicants[1] = svector<int>();

	if (bubble)
		gen_minterms();
	else
		gen_bubbleless_minterms();
}

rule::rule(sstring u, sstring d, sstring v, variable_space *vars, petri *net, flag_space *flags)
{
	this->net = net;
	this->vars = vars;
	this->flags = flags;
	implicants[0] = svector<int>();
    implicants[1] = svector<int>();

	gen_variables(u, vars, flags);
	gen_variables(d, vars, flags);

	if ((uid = vars->get_uid(v)) < 0)
		uid = vars->insert(variable(v, "node", 1, 0, false, flags));

	this->guards[1] = logic(u, vars);
	this->guards[0] = logic(d, vars);
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
	implicants[0] = svector<int>();
    implicants[1] = svector<int>();
	vars = NULL;
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
	vars = r.vars;
	net = r.net;
	flags = r.flags;
	return *this;
}

pair<svector<int>, logic> rule::closest_transition(int curr_place, int transition, int conflicting_state, logic rule_guard, logic implicant_state, svector<int> tail, svector<bool> *covered, int i)
{
	svector<int> next;
	svector<int> curr;
	svector<int> vlist;
	svector<int> comb;
	smap<int, pair<int, int> >::iterator cpi;
	pair<svector<int>, logic> ret;
	int j, k, l;
	bool immune = false;
	bool found;
	logic temp, temp2, t, effective_conflict;
	smap<int, logic>::iterator ji;
	svector<logic> conflict_check;

	cout << "\n" << sstring(i, '\t') << "Closest Transition " << curr_place << endl;

	next.push_back(curr_place);

	while (1)
	{
		cout << sstring(i, '\t') << next[0] << " ";

		if (!(*covered)[i])
			(*covered)[i] = true;
		else
		{
			cout << "kill" << endl;
			return pair<svector<int>, logic>(svector<int>(), rule_guard);
		}

		if (net->is_trans(net->arcs[next[0]].second))
		{
			t = net->T[net->index(net->arcs[next[0]].second)].index.hide(uid);

			effective_conflict = net->get_effective_state_encoding(conflicting_state, transition) & canonical(uid, 1-net->T[net->index(transition)].index.terms[0].val(uid));
			cout << effective_conflict.print(vars) << " ";

			/*temp = rule_guard & t;
			if ((temp & implicant_state) != 0 && (temp & effective_conflict) == 0)
			{
				cout << "end" << endl;
				return pair<svector<int>, logic>(svector<int>(), temp);
			}*/

			temp2 = 0;
			found = true;
			for (l = 0; l < t.terms.size() && found; l++)
			{
				vlist = t.terms[l].vars();
				found = false;
				for (j = 1; j <= vlist.size() && !found; j++)
				{
					comb = first_combination(j);
					do
					{
						temp = rule_guard;
						for (k = 0; k < comb.size(); k++)
							temp &= canonical(vlist[comb[k]], t.terms[l].val(vlist[comb[k]]));

						if ((temp & implicant_state) != 0 && (temp & effective_conflict) == 0)
						{
							temp2 |= temp;
							found = true;
						}
					} while (next_combination(vlist.size(), &comb) && !found);
					comb.clear();
				}
				vlist.clear();
			}
			if (found)
			{
				cout << "end" << endl;
				return pair<svector<int>, logic>(svector<int>(), temp2);
			}
		}

		curr = next;
		next.clear();
		for (j = 0; j < net->arcs.size(); j++)
			for (k = 0; k < curr.size(); k++)
				if (net->arcs[curr[k]].second == net->arcs[j].first)
					next.push_back(j);
		next.unique();


		if (!immune && i != 0 && net->is_trans(net->arcs[next[0]].first) && net->input_arcs(net->arcs[next[0]].first).size() > 1)
		{
			cout << "pmerge" << endl;
			return pair<svector<int>, logic>(next, logic());
		}

		for (cpi = net->conditional_places.begin(); !immune && i != 0 && cpi != net->conditional_places.end(); cpi++)
			if (net->arcs[next[0]].first == cpi->second.first)
			{
				cout << "cmerge" << endl;
				return pair<svector<int>, logic>(next, logic());
			}

		cout << endl;

		immune = false;
		while (next.size() > 1)
		{
			immune = false;
			if (net->is_place(net->arcs[curr[0]].second))
			{
				curr = next;
				temp = rule_guard;
				rule_guard = 0;

				next.clear();
				conflict_check.clear();
				for (j = 0; j < curr.size(); j++)
				{
					if (net->psiblings(net->arcs[curr[j]].second, transition) == -1 && net->csiblings(net->arcs[curr[j]].second, transition) == -1)
					{
						ret = closest_transition(curr[j], transition, conflicting_state, temp, implicant_state, tail, covered, i+1);
						next.merge(ret.first);
						rule_guard |= ret.second;
						conflict_check.push_back(ret.second);
						if (ret.first.size() != 0)
							immune = true;
					}
				}
				if (next.size() != 0)
					rule_guard = temp;
				else
					for (k = 0; k < conflict_check.size(); k++)
						for (j = k+1; j < conflict_check.size(); j++)
							if (conflict_check[k] != conflict_check[j])
							{
								if ((conflict_check[k] | conflict_check[j]) == conflict_check[k])
									cerr << "Warning: Conflict may have been created somewhere down arc " << curr[j] << " during guard strengthening for " << vars->get_name(uid) << (t == 0 ? "-" : "+") << " from state " << net->arcs[net->index(next[0])].second << "." << endl;
								else if ((conflict_check[k] | conflict_check[j]) == conflict_check[j])
									cerr << "Warning: Conflict may have been created somewhere down arc " << curr[k] << " during guard strengthening for " << vars->get_name(uid) << (t == 0 ? "-" : "+") << " from state " << net->arcs[net->index(next[0])].second << "." << endl;
							}

				next.unique();
			}
			else if (net->is_trans(net->arcs[curr[0]].second))
			{
				curr = next;

				next.clear();
				for (j = 0; j < curr.size(); j++)
				{
					ret = closest_transition(curr[j], transition, conflicting_state, rule_guard, implicant_state, tail, covered, i+1);
					next.merge(ret.first);
					if (ret.first.size() == 0)
						return ret;
					else
						immune = true;
				}

				next.unique();
			}
		}

		if (next.size() < 1)
		{
			cout << "kill" << endl;
			return pair<svector<int>, logic>(svector<int>(), rule_guard);
		}
	}
}


pair<svector<int>, logic> rule::strengthen(int p, int tid, svector<bool> *covered, logic rule_guard, logic implicant_state, int t, svector<int> tail, int i)
{
	svector<int> next;
	svector<int> curr;
	svector<logic> conflict_check;
	smap<int, pair<int, int> >::iterator cpi;
	pair<svector<int>, logic> ret;
	int j, k;
	bool immune = false;
	bool needed, found;
	logic temp, temp2, temp3, temp4;
	svector<int> ia;
	svector<int> comb, vlist;
	svector<bool> covered2;
	logic effective_conflict;

	if (covered->size() < net->arcs.size())
		covered->resize(net->arcs.size(), false);

	cout << sstring(i, '\t') << "Strengthen " << p << endl;

	next.push_back(p);

	while (1)
	{
		cout << sstring(i, '\t') << next[0] << " " << rule_guard.print(vars) << " " << implicant_state.print(vars) << " ";
		fflush(stdout);

		if (!immune && net->is_place(net->arcs[next[0]].first))
		{
			needed = false;
			ia = net->input_arcs(net->arcs[next[0]].first);
			for (j = 0; j < ia.size() && !needed; j++)
				if (!(*covered)[ia[j]])
					needed = true;

			for (j = 0; j < implicants[t].size() && needed; j++)
				if (net->psiblings(net->arcs[next[0]].first, implicants[t][j]) != -1)
					needed = false;

			effective_conflict = net->get_effective_state_encoding(net->arcs[next[0]].first, tid);

			covered2.clear();
			covered2.resize(net->arcs.size(), false);
			if (needed && (net->T[net->index(net->arcs[next[0]].second)].index & logic(uid, 1-t)) != 0 && (logic(uid, 1-t) & rule_guard & effective_conflict) != 0)
				rule_guard = closest_transition(next[0], tid, net->arcs[next[0]].first, rule_guard, implicant_state, tail, &covered2).second;
		}

		(*covered)[next[0]] = true;

		curr = next;
		next.clear();
		for (j = 0; j < net->arcs.size(); j++)
			for (k = 0; k < curr.size(); k++)
				if (net->arcs[curr[k]].first == net->arcs[j].second)
					if (!(*covered)[j] && (net->is_place(net->arcs[j].first) || (net->T[net->index(net->arcs[j].first)].index & logic(uid, 1-t)) != 0))
						next.push_back(j);
		next.unique();

		if (!immune && i != 0 && net->is_trans(net->arcs[next[0]].second) && net->output_arcs(net->arcs[next[0]].second).size() > 1)
		{
			cout << "pmerge" << endl;
			return pair<svector<int>, logic>(next, rule_guard);
		}

		for (cpi = net->conditional_places.begin(); !immune && i != 0 && cpi != net->conditional_places.end(); cpi++)
			if (net->arcs[next[0]].second == cpi->second.second)
			{
				cout << "cmerge" << endl;
				return pair<svector<int>, logic>(next, rule_guard);
			}

		cout << endl;

		immune = false;
		temp = rule_guard;
		while (next.size() > 1)
		{
			immune = false;
			if (net->is_place(net->arcs[curr[0]].first))
			{
				curr = next;
				rule_guard = 0;

				next.clear();
				conflict_check.clear();
				for (j = 0; j < curr.size(); j++)
				{
					ret = strengthen(curr[j], tid, covered, temp, implicant_state, t, tail, i+1);
					next.merge(ret.first);
					conflict_check.push_back(ret.second);
					if (ret.first.size() != 0)
						immune = true;
				}

				next.unique();

				/*
				 * If you have a conditional merge
				 *        A -> T4- -> B -> T1+ ->   -> [~T4] -> G
				 * T4+ -> D -> T1+ -> E -> T3+ -> C -> [ T4] -> F -> T
				 * and you want to strengthen the guard for T which is currently T4 -> T,
				 * B is NOT in conflict with F, but A is, so when you run closest_transition
				 * it returns T1&T4 -> T. When you run closest_transition on the other branch,
				 * you get T1&T3&T4 -> T. Then you get here and you OR everything together.
				 * This gives you T1&T4, leaving a conflicting state in the second branch.
				 * If you were to XOR everything together, you would get T1&T4&~T3. Now T
				 * won't fire after going through the second branch. ANDing them together
				 * gets you what you want here, but what happens if you have
				 * A ->  T1 ->
				 * B -> ~T1 -> C -> T2 -> D -> T
				 * If you and them together here, your guard would end up being 0.
				 * If I do some kind of merge step where I delete minterms that aren't strengthened
				 * enough, then this breaks.
				 * A    ->     T1     ->
				 * B -> T1 -> C -> T2 -> D -> T3 -> E -> T
				 */
				for (j = 0; j < conflict_check.size(); j++)
					for (k = j+1; k < conflict_check.size(); k++)
						if (conflict_check[j] != conflict_check[k] && mergible(&conflict_check[j], &conflict_check[k]))
						{
							temp = (conflict_check[j] & net->S[net->arcs[curr[0]].second].index).hide(conflict_check[j].vars());
							temp2 = (conflict_check[k] & net->S[net->arcs[curr[0]].second].index).hide(conflict_check[k].vars());

							if ((conflict_check[j] | conflict_check[k]) == conflict_check[j])
							{
								temp3 = 0;
								found = true;
								for (int l = 0; l < temp.terms.size() && found; l++)
								{
									vlist = temp.terms[l].vars();
									found = false;
									for (int m = 1; m <= vlist.size() && !found; m++)
									{
										comb = first_combination(m);
										do
										{
											for (int n = 0; n < comb.size(); n++)
												temp4 = (n == 0 ? canonical(vlist[comb[n]], temp.terms[l].val(vlist[comb[n]]))  :
														  temp4 & canonical(vlist[comb[n]], temp.terms[l].val(vlist[comb[n]])) );

											if (!is_mutex(&temp4, &conflict_check[j], &implicant_state) && is_mutex(&temp4, &temp2, &vars->enforcements))
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
									cerr << "Error: Conflict created during guard strengthening for " << vars->get_name(uid) << (t == 0 ? "-" : "+") << " from state " << net->arcs[net->index(next[0])].second << "." << endl;
								else
									conflict_check[j] &= temp3;
							}
							else if ((conflict_check[j] | conflict_check[k]) == conflict_check[k])
							{
								temp3 = 0;
								found = true;
								for (int l = 0; l < temp2.terms.size() && found; l++)
								{
									vlist = temp2.terms[l].vars();
									found = false;
									for (int m = 1; m <= vlist.size() && !found; m++)
									{
										comb = first_combination(m);
										do
										{
											for (int n = 0; n < comb.size(); n++)
												temp4 = (n == 0 ? canonical(vlist[comb[n]], temp2.terms[l].val(vlist[comb[n]]))  :
														  temp4 & canonical(vlist[comb[n]], temp2.terms[l].val(vlist[comb[n]])) );

											if (!is_mutex(&temp4, &conflict_check[k], &implicant_state) && is_mutex(&temp4, &temp, &vars->enforcements))
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
									cerr << "Error: Conflict created during guard strengthening for " << vars->get_name(uid) << (t == 0 ? "-" : "+") << " from state " << net->arcs[net->index(next[0])].second << "." << endl;
								else
									conflict_check[k] &= temp3;
							}
							else
							{
								temp3 = 0;
								found = true;
								for (int l = 0; l < temp.terms.size() && found; l++)
								{
									vlist = temp.terms[l].vars();
									found = false;
									for (int m = 1; m <= vlist.size() && !found; m++)
									{
										comb = first_combination(m);
										do
										{
											for (int n = 0; n < comb.size(); n++)
												temp4 = (n == 0 ? canonical(vlist[comb[n]], temp.terms[l].val(vlist[comb[n]]))  :
														  temp4 & canonical(vlist[comb[n]], temp.terms[l].val(vlist[comb[n]])) );

											if (!is_mutex(&temp4, &conflict_check[j], &implicant_state) && is_mutex(&temp4, &temp2, &vars->enforcements))
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
									cerr << "Error: Conflict created during guard strengthening for " << vars->get_name(uid) << (t == 0 ? "-" : "+") << " from state " << net->arcs[net->index(next[0])].second << "." << endl;
								else
									conflict_check[j] &= temp3;

								temp3 = 0;
								found = true;
								for (int l = 0; l < temp2.terms.size() && found; l++)
								{
									vlist = temp2.terms[l].vars();
									found = false;
									for (int m = 1; m <= vlist.size() && !found; m++)
									{
										comb = first_combination(m);
										do
										{
											for (int n = 0; n < comb.size(); n++)
												temp4 = (n == 0 ? canonical(vlist[comb[n]], temp2.terms[l].val(vlist[comb[n]]))  :
														  temp4 & canonical(vlist[comb[n]], temp2.terms[l].val(vlist[comb[n]])) );

											if (!is_mutex(&temp4, &conflict_check[k], &implicant_state) && is_mutex(&temp4, &temp, &vars->enforcements))
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
									cerr << "Error: Conflict created during guard strengthening for " << vars->get_name(uid) << (t == 0 ? "-" : "+") << " from state " << net->arcs[net->index(next[0])].second << "." << endl;
								else
									conflict_check[k] &= temp3;
							}
						}

				for (j = 0; j < conflict_check.size(); j++)
					rule_guard |= conflict_check[j];
			}
			else if (net->is_trans(net->arcs[curr[0]].first))
			{
				curr = next;
				rule_guard = 1;

				next.clear();
				for (j = 0; j < curr.size(); j++)
				{
					ret = strengthen(curr[j], tid, covered, temp, implicant_state, t, tail, i+1);
					next.merge(ret.first);
					rule_guard &= ret.second;
					if (ret.first.size() != 0)
						immune = true;
				}

				next.unique();
			}
		}

		if (next.size() < 1)
		{
			cout << "kill" << endl;
			return pair<svector<int>, logic>(svector<int>(), rule_guard);
		}
	}
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
	svector<int> ia;
	svector<int> vl;
	int i, j;
	logic t;
	svector<bool> covered;
	logic outside = 0;
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
				ia = net->input_nodes(net->trans_id(i));
				for (j = 0, t = 1; j < ia.size(); j++)
				{
					implicants[1].push_back(ia[j]);
					t &= net->S[ia[j]].index;
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
						guards[1] |= strengthen(j, &covered, logic(1), t, 1, net->T[i].tail).second;
				cout << endl;*/
			}

			if (net->T[i].index(uid, 0) != 0)
			{
				ia = net->input_nodes(net->trans_id(i));
				for (j = 0, t = 1; j < ia.size(); j++)
				{
					implicants[0].push_back(ia[j]);
					t &= net->S[ia[j]].index;
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
						guards[0] |= strengthen(j, &covered, logic(1), t, 0, net->T[i].tail).second;
				cout << endl;*/
			}
		}
	}
}

void rule::gen_bubbleless_minterms()
{
	guards[1] = logic(0);
	guards[0] = logic(0);
	svector<int> ia;
	svector<int> vl;
	int i, j;
	logic t;
	for (i = 0; i < (int)net->T.size(); i++)
	{
		vl.clear();
		net->T[i].index.vars(&vl);
		if (net->T[i].active && find(vl.begin(), vl.end(), uid) != vl.end())
		{
			if (net->T[i].index(uid, 1) != 0)
			{
				ia = net->input_nodes(net->trans_id(i));
				for (j = 0, t = 1; j < (int)ia.size(); j++)
					t = t & net->S[ia[j]].negative;

				t = t.hide(vl);
				guards[1] = guards[1] | t;
			}

			if (net->T[i].index(uid, 0) != 0)
			{
				ia = net->input_nodes(net->trans_id(i));
				for (j = 0, t = 1; j < (int)ia.size(); j++)
					t = t & net->S[ia[j]].positive;

				t = t.hide(vl);
				guards[0] = guards[0] | t;
			}
		}
	}
}

logic &rule::up()
{
	return guards[1];
}

logic &rule::down()
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
	logic temp = guards[0];
	guards[0] = guards[1];
	guards[1] = temp;
	temp = explicit_guards[0];
	explicit_guards[0] = explicit_guards[1];
	explicit_guards[1] = temp;
	svector<int> tempimp = implicants[0];
	implicants[0] = implicants[1];
	implicants[1] = tempimp;
}

//Print the rule in the following format:
//guards[1] left -> guards[1] right+
//guards[0] left -> guards[0] right-
void rule::print(ostream &os, sstring prefix)
{
	svector<sstring> names = vars->get_names();
	for (int i = 0; i < (int)names.size(); i++)
		names[i] = prefix + names[i];

	if (guards[1] != -1)
		os << guards[1].print_with_quotes(vars, prefix) << " -> \"" << names[uid] << "\"+" << endl;
	if (guards[0] != -1)
		os << guards[0].print_with_quotes(vars, prefix) << " -> \"" << names[uid] << "\"-" << endl;
}

//Print the rule in the following format:
//guards[1] left -> guards[1] right+
//guards[0] left -> guards[0] right-
ostream &operator<<(ostream &os, rule r)
{
	svector<sstring> names = r.vars->get_names();
	if (r.guards[1] != -1)
		os << r.guards[1].print_with_quotes(r.vars) << " -> \"" << names[r.uid] << "\"+" << endl;
	if (r.guards[0] != -1)
		os << r.guards[0].print_with_quotes(r.vars) << " -> \"" << names[r.uid] << "\"-" << endl;

    return os;
}
