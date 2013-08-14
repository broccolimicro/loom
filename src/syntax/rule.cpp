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
}

rule::rule(int uid)
{
	this->uid = uid;
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
	vars->find(uid)->driven = true;

	this->up = logic(u, vars);
	this->down = logic(d, vars);
	cout << u << endl << this->up.print(vars) << endl << endl;
	cout << d << endl << this->down.print(vars) << endl << endl << endl;
}

rule::~rule()
{
	this->uid = -1;
}

rule &rule::operator=(rule r)
{
	uid = r.uid;
	up = r.up;
	down = r.down;
	vars = r.vars;
	net = r.net;
	flags = r.flags;
	return *this;
}

/* gen_minterms produces the weakest set of implicants that cannot reduce
 * the conflict firing space by adding another variable to a given implicant.
 * This information is stored in the implicants field of rule's up and down.
 * Note that the implicants are generated in a greedy manner: Each variable
 * added to a given implicant is selected based on which would reduce the number
 * of conflict states the most.
 */
void rule::gen_minterms()
{
	up = 0;
	down = 0;
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
					t = t & net->S[ia[j]].index;

				t = t.smooth(vl);
				up = up | t;
			}

			if (net->T[i].index(uid, 0) != 0)
			{
				ia = net->input_nodes(net->trans_id(i));
				for (j = 0, t = 1; j < (int)ia.size(); j++)
					t = t & net->S[ia[j]].index;

				t = t.smooth(vl);
				down = down | t;
			}
		}
	}
}

void rule::gen_bubbleless_minterms()
{
	up = logic(0);
	down = logic(0);
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

				t = t.smooth(vl);
				up = up | t;
			}

			if (net->T[i].index(uid, 0) != 0)
			{
				ia = net->input_nodes(net->trans_id(i));
				for (j = 0, t = 1; j < (int)ia.size(); j++)
					t = t & net->S[ia[j]].positive;

				t = t.smooth(vl);
				down = down | t;
			}
		}
	}
}

//Print the rule in the following format:
//up left -> up right+
//down left -> down right-
void rule::print(ostream &os, string prefix)
{
	vector<string> names = vars->get_names();
	for (int i = 0; i < (int)names.size(); i++)
		names[i] = prefix + names[i];

	if (up != -1)
		os << up.print(vars) << " -> " << names[uid] << "+" << endl;
	if (down != -1)
		os << down.print(vars) << " -> " << names[uid] << "-" << endl;
}

//Print the rule in the following format:
//up left -> up right+
//down left -> down right-
ostream &operator<<(ostream &os, rule r)
{
	vector<string> names = r.vars->get_names();
	if (r.up != -1)
		os << r.up.print(r.vars) << " -> " << names[r.uid] << "+" << endl;
	if (r.down != -1)
		os << r.down.print(r.vars) << " -> " << names[r.uid] << "-" << endl;

    return os;
}
