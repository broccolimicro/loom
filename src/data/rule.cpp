/*
 * rule.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "rule.h"

rule::rule()
{
	this->uid = -1;
}

rule::rule(int uid)
{
	this->uid = uid;
}

rule::rule(int uid, petri *g, vspace *v, int verbosity)
{
	this->uid = uid;
	this->vars = v;
	this->net = g;
	this->verbosity = verbosity;

	gen_minterms();
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
	up = -1;
	down = -1;
	vector<int> ia;
	vector<int> vl;
	int i, j, t;
	for (i = 0; i < (int)net->T.size(); i++)
	{
		vl.clear();
		net->values->variable_list(net->T[i].index, &vl);
		if (net->T[i].active && find(vl.begin(), vl.end(), uid) != vl.end())
		{
			if (((bdd*)net->values)->restrict(net->T[i].index, uid, 1) > 0)
			{
				ia = net->input_arcs(net->trans_id(i));
				t = net->S[ia[0]].index;
				for (j = 1; j < (int)ia.size(); j++)
					t = net->values->apply_and(t, net->S[ia[j]].index);

				up = (up == -1) ? t : net->values->apply_or(up, t);
			}

			if (((bdd*)net->values)->restrict(net->T[i].index, uid, 0) > 0)
			{
				ia = net->input_arcs(net->trans_id(i));
				t = net->S[ia[0]].index;
				for (j = 1; j < (int)ia.size(); j++)
					t = net->values->apply_and(t, net->S[ia[j]].index);

				down = (down == -1) ? t : net->values->apply_or(down, t);
			}
		}
	}

	if (up != -1)
		up = net->values->smooth(up, uid);
	if (down != -1)
		down = net->values->smooth(down, uid);
}

//Print the rule in the following format:
//up left -> up right+
//down left -> down right-
ostream &operator<<(ostream &os, rule r)
{
	if (r.up != -1)
		os << r.net->values->expr(r.up, r.vars->get_names()) << " -> " << r.vars->get_name(r.uid) << "+" << endl;
	if (r.down != -1)
		os << r.net->values->expr(r.down, r.vars->get_names()) << " -> " << r.vars->get_name(r.uid) << "-" << endl;

    return os;
}
