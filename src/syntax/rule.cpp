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
	vars = NULL;
	net = NULL;
	flags = NULL;
}

rule::rule(int uid)
{
	this->uid = uid;
	guards[0] = 0;
	guards[1] = 0;
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

	if (bubble)
		gen_minterms();
	else
		gen_bubbleless_minterms();
}

rule::rule(string u, string d, string v, variable_space *vars, petri *net, flag_space *flags)
{
	this->net = net;
	this->vars = vars;
	this->flags = flags;

	gen_variables(u, vars, flags);
	gen_variables(d, vars, flags);

	if ((uid = vars->get_uid(v)) < 0)
		uid = vars->insert(variable(v, "node", 1, false, flags));

	this->guards[1] = logic(u, vars);
	this->guards[0] = logic(d, vars);
	cout << u << endl << this->guards[1].print(vars) << endl << endl;
	cout << d << endl << this->guards[0].print(vars) << endl << endl << endl;
}

rule::~rule()
{
	this->uid = -1;
	guards[0] = 0;
	guards[1] = 0;
	vars = NULL;
	net = NULL;
	flags = NULL;
}

rule &rule::operator=(rule r)
{
	uid = r.uid;
	guards[1] = r.guards[1];
	guards[0] = r.guards[0];
	vars = r.vars;
	net = r.net;
	flags = r.flags;
	return *this;
}

pair<int, logic> rule::closest_transition(int p, logic c, logic g, logic mg, vector<int> tail, vector<bool> *covered, int i)
{
	vector<int> next;
	vector<int> curr;
	map<int, pair<int, int> >::iterator cpi;
	pair<int, logic> ret;
	int j, k;
	bool immune = false;
	logic temp;

	if (covered->size() < net->arcs.size())
		covered->resize(net->arcs.size(), false);

	//cout << string(i, '\t') << "Closest Transition " << p << endl;

	next.push_back(p);

	while (1)
	{
		//cout << string(i, '\t') << next[0] << " ";

		if (!immune && i != 0 && net->is_trans(net->arcs[next[0]].first) && net->input_arcs(net->arcs[next[0]].first).size() > 1)
		{
			//cout << "pmerge" << endl;
			return pair<int, logic>(next[0], g);
		}

		for (cpi = net->conditional_places.begin(); !immune && i != 0 && cpi != net->conditional_places.end(); cpi++)
			if (net->arcs[next[0]].first == cpi->second.first)
			{
				//cout << "cmerge" << endl;
				return pair<int, logic>(next[0], g);
			}

		if (net->is_trans(net->arcs[next[0]].second))
		{
			temp = net->T[net->index(net->arcs[next[0]].second)].index.hide(uid) & g;
			if ((temp & mg) != 0 && ((temp & c) == 0 || find(tail.begin(), tail.end(), net->arcs[next[0]].first) != tail.end()))
			{
				//cout << "end" << endl;
				return pair<int, logic>(-1, temp);
			}
		}

		/*if ((*covered)[next[0]])
		{
			cout << "covered" << endl;
			return pair<int, logic>(-1, g);
		}

		(*covered)[next[0]] = true;*/

		//cout << endl;

		curr = next;
		next.clear();
		for (j = 0; j < (int)net->arcs.size(); j++)
			for (k = 0; k < (int)curr.size(); k++)
				if (net->arcs[curr[k]].second == net->arcs[j].first)
					next.push_back(j);
		unique(&next);

		immune = false;
		if (next.size() > 1 && net->is_place(net->arcs[curr[0]].second))
		{
			curr = next;
			temp = g;
			g = 0;

			next.clear();
			for (j = 0; j < (int)curr.size(); j++)
			{
				ret = closest_transition(curr[j], c, temp, mg, tail, covered, i+1);
				next.push_back(ret.first);
				g |= ret.second;
				if (next.back() == -1)
					next.pop_back();
				else
					immune = true;
			}
			if (next.size() != 0)
				g = temp;

			unique(&next);
		}
		else if (next.size() > 1 && net->is_trans(net->arcs[curr[0]].second))
		{
			curr = next;

			next.clear();
			for (j = 0; j < (int)curr.size(); j++)
			{
				ret = closest_transition(curr[j], c, g, mg, tail, covered, i+1);
				next.push_back(ret.first);
				if (next.back() == -1)
					return ret;
				else
					immune = true;
			}

			unique(&next);
		}

		if (next.size() < 1)
		{
			//cout << "kill" << endl;
			return pair<int, logic>(-1, g);
		}
	}
}


pair<int, logic> rule::strengthen(int p, vector<bool> *covered, logic g, logic mg, int t, vector<int> tail, int i)
{
	vector<int> next;
	vector<int> curr;
	map<int, pair<int, int> >::iterator cpi;
	pair<int, logic> ret;
	int j, k;
	bool immune = false;
	bool needed;
	logic temp;
	vector<int> ia;
	vector<bool> covered2;

	if (covered->size() < net->arcs.size())
		covered->resize(net->arcs.size(), false);

	//cout << string(i, '\t') << "Strengthen " << p << endl;

	next.push_back(p);

	while (1)
	{
		//cout << string(i, '\t') << next[0] << " " << g.print(vars) << " " << mg.print(vars) << " ";

		if ((*covered)[next[0]])
		{
			//cout << "cov" << endl;
			return pair<int, logic>(-1, g);
		}

		if (!immune && i != 0 && net->is_trans(net->arcs[next[0]].second) && net->output_arcs(net->arcs[next[0]].second).size() > 1)
		{
			//cout << "pmerge" << endl;
			return pair<int, logic>(next[0], g);
		}

		for (cpi = net->conditional_places.begin(); !immune && i != 0 && cpi != net->conditional_places.end(); cpi++)
			if (net->arcs[next[0]].second == cpi->second.second)
			{
				//cout << "cmerge" << endl;
				return pair<int, logic>(next[0], g);
			}

		if (!immune && net->is_place(net->arcs[next[0]].first))
		{
			needed = false;
			ia = net->input_arcs(net->arcs[next[0]].first);
			for (j = 0; j < (int)ia.size() && !needed; j++)
				if (!(*covered)[ia[j]])
					needed = true;

			for (j = 0; j < (int)implicants[t].size() && needed; j++)
				if (net->psiblings(net->arcs[next[0]].first, implicants[t][j]) != -1)
					needed = false;

			covered2.clear();
			if (needed && (net->T[net->index(net->arcs[next[0]].second)].index & logic(uid, 1-t)) != 0 && (logic(uid, 1-t) & g & net->S[net->arcs[next[0]].first].index) != 0)
				g = closest_transition(next[0], net->S[net->arcs[next[0]].first].index, g, mg, tail, &covered2).second;
		}

		(*covered)[next[0]] = true;

		//cout << endl;

		curr = next;
		next.clear();
		for (j = 0; j < (int)net->arcs.size(); j++)
			for (k = 0; k < (int)curr.size(); k++)
				if (net->arcs[curr[k]].first == net->arcs[j].second)
					next.push_back(j);
		unique(&next);

		immune = false;
		temp = g;
		if (next.size() > 1 && net->is_place(net->arcs[curr[0]].first))
		{
			curr = next;
			g = 0;

			next.clear();
			for (j = 0; j < (int)curr.size(); j++)
			{
				ret = strengthen(curr[j], covered, temp, mg, t, tail, i+1);
				next.push_back(ret.first);
				g |= ret.second;
				if (next.back() == -1)
					next.pop_back();
				else
					immune = true;
			}

			unique(&next);
		}
		else if (next.size() > 1 && net->is_trans(net->arcs[curr[0]].first))
		{
			curr = next;
			g = 1;

			next.clear();
			for (j = 0; j < (int)curr.size(); j++)
			{
				ret = strengthen(curr[j], covered, temp, mg, t, tail, i+1);
				next.push_back(ret.first);
				g &= ret.second;
				if (next.back() == -1)
					next.pop_back();
				else
					immune = true;
			}

			unique(&next);
		}

		if (next.size() < 1)
		{
			//cout << "kill" << endl;
			return pair<int, logic>(-1, g);
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
	vector<int> ia;
	vector<int> vl;
	int i, j;
	logic t;
	vector<bool> covered;

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
				{
					implicants[1].push_back(ia[j]);
					t = t & net->S[ia[j]].index;
				}

				t = t.hide(vl);
				//guards[1] = guards[1] | t;

				covered.clear();
				cout << "Start " << net->T[i].index.print(vars) <<  " ";
				for (j = 0; j < (int)net->T[i].tail.size(); j++)
					cout << net->T[i].tail[j] << " ";
				cout << endl;
				for (j = 0; j < (int)net->arcs.size(); j++)
					if (net->arcs[j].second == net->trans_id(i))
						guards[1] |= strengthen(j, &covered, logic(1), t, 1, net->T[i].tail).second;
				cout << endl;
			}

			if (net->T[i].index(uid, 0) != 0)
			{
				ia = net->input_nodes(net->trans_id(i));
				for (j = 0, t = 1; j < (int)ia.size(); j++)
				{
					implicants[0].push_back(ia[j]);
					t = t & net->S[ia[j]].index;
				}
				t = t.hide(vl);
				//guards[0] = guards[0] | t;

				covered.clear();
				cout << "Start " << net->T[i].index.print(vars) <<  " ";
				for (j = 0; j < (int)net->T[i].tail.size(); j++)
					cout << net->T[i].tail[j] << " ";
				cout << endl;
				for (j = 0; j < (int)net->arcs.size(); j++)
					if (net->arcs[j].second == net->trans_id(i))
						guards[0] |= strengthen(j, &covered, logic(1), t, 0, net->T[i].tail).second;
				cout << endl;
			}
		}
	}
}

void rule::gen_bubbleless_minterms()
{
	guards[1] = logic(0);
	guards[0] = logic(0);
	vector<int> ia;
	vector<int> vl;
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

//Print the rule in the following format:
//guards[1] left -> guards[1] right+
//guards[0] left -> guards[0] right-
void rule::print(ostream &os, string prefix)
{
	vector<string> names = vars->get_names();
	for (int i = 0; i < (int)names.size(); i++)
		names[i] = prefix + names[i];

	if (guards[1] != -1)
		os << guards[1].print(vars, prefix) << " -> " << names[uid] << "+" << endl;
	if (guards[0] != -1)
		os << guards[0].print(vars, prefix) << " -> " << names[uid] << "-" << endl;
}

//Print the rule in the following format:
//guards[1] left -> guards[1] right+
//guards[0] left -> guards[0] right-
ostream &operator<<(ostream &os, rule r)
{
	vector<string> names = r.vars->get_names();
	if (r.guards[1] != -1)
		os << r.guards[1].print(r.vars) << " -> " << names[r.uid] << "+" << endl;
	if (r.guards[0] != -1)
		os << r.guards[0].print(r.vars) << " -> " << names[r.uid] << "-" << endl;

    return os;
}
