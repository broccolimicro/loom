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
	this->name = "";
}

rule::rule(int uid)
{
	this->uid = uid;
	this->name = "";
}

rule::rule(int uid, graph *g, vspace *v, int verbosity)
{
	this->uid = uid;
	this->up.vars = v;
	this->down.vars = v;
	this->name = v->get_name(uid);
	this->verbosity = verbosity;

	gen_minterms(g);
	gen_primes();
	gen_essentials();
	gen_output(v);
}

rule::~rule()
{
	this->uid = -1;
}

rule &rule::operator=(rule r)
{
	uid = r.uid;
	name = r.name;
	up = r.up;
	down = r.down;
	return *this;
}

void rule::gen_minterms(graph *g)
{
	list<int> invars;
	list<int>::iterator vk;

	state implier;
	state implicant;
	trace implicant_output;
	trace final_output;
	trace proposal_output;

	int vj, ii;
	int count, mcount, var;

	if (verbosity & VERB_GENERATE_BASE_PRS)
	{
		cout << "Generating Minterms for " << uid << endl;
		cout << "Up Minterms" << endl;
	}

	// Up Production Rule Minterms (Duplicate Code)
	final_output = trace(value("0"), g->up[uid].size());
	for (ii = 0; ii < (int)g->up_firings[uid].size(); ii++)
	{
		implier			 = g->states[g->up_firings[uid][ii]];
		implicant		 = state(value("X"), g->width());
		implicant_output = trace(value("1"), g->up[uid].size());

		/* There are two ways we can do this. The first is to do the normal
		 * production rule generation algorithm where we are allowed to use
		 * any variable that is known in the implier. The second is to disallow
		 * the use of bubbles in the production rules, then let the state variable
		 * insertion algorithm handle bubble reshuffling.
		 */
		invars.clear();
		for (vj = 0; vj < g->width(); vj++)
			if (((BUBBLELESS && implier[vj].data == "0") || !BUBBLELESS) && vj != uid)
				invars.push_back(vj);

		/* Set the maximum possible conflict count to be the number of zeros
		 * in the desired trace so that we don't accidentally add an unnecessary
		 * variable to our implicant
		 */
		mcount = conflict_count(implicant_output, g->up[uid]);
		var = 0;
		while (invars.size() > 0 && var != -1)
		{
			var = -1;
			for (vk = invars.begin(); vk != invars.end(); vk++)
			{
				if (implier[*vk].data == "0")
					proposal_output = implicant_output & ~g->delta[*vk];
				else if (implier[*vk].data == "1")
					proposal_output = implicant_output & g->delta[*vk];
				count = conflict_count(proposal_output, g->up[uid]);
				if (count < mcount)
				{
					mcount = count;
					var = *vk;
				}
			}

			if (var != -1)
			{
				implicant[var] = implier[var];
				if (implier[var].data == "0")
					implicant_output = implicant_output & ~g->delta[var];
				else if (implier[var].data == "1")
					implicant_output = implicant_output & g->delta[var];
				invars.remove(var);
			}
		}

		if (verbosity & VERB_GENERATE_BASE_PRS)
			cout << implicant << "\t" << implicant_output << endl;
		final_output = final_output | implicant_output;
		up.implicants.push_back(implicant);
	}

	if (verbosity & VERB_GENERATE_BASE_PRS)
	{
		cout << endl;
		cout << "Desired:  " << g->up[uid] << endl;
		cout << "Obtained: " << final_output << endl;
		cout << "Down Minterms" << endl;
	}

	// Down Production Rule Minterms (Duplicate Code)
	final_output = trace(value("0"), g->up[uid].size());
	for (ii = 0; ii < (int)g->down_firings[uid].size(); ii++)
	{
		implier			 = g->states[g->down_firings[uid][ii]];
		implicant		 = state(value("X"), g->width());
		implicant_output = trace(value("1"), g->up[uid].size());

		invars.clear();
		for (vj = 0; vj < g->width(); vj++)
			if (((BUBBLELESS && implier[vj].data == "1") || !BUBBLELESS) && vj != uid)
				invars.push_back(vj);

		mcount = conflict_count(implicant_output, g->down[uid]);
		var = 0;
		while (invars.size() > 0 && var != -1)
		{
			var = -1;
			for (vk = invars.begin(); vk != invars.end(); vk++)
			{
				if (implier[*vk].data == "0")
					proposal_output = implicant_output & ~g->delta[*vk];
				else if (implier[*vk].data == "1")
					proposal_output = implicant_output & g->delta[*vk];
				count = conflict_count(proposal_output, g->down[uid]);
				if (count < mcount)
				{
					mcount = count;
					var = *vk;
				}
			}

			if (var != -1)
			{
				implicant[var] = implier[var];
				if (implier[var].data == "0")
					implicant_output = implicant_output & ~g->delta[var];
				else if (implier[var].data == "1")
					implicant_output = implicant_output & g->delta[var];
				invars.remove(var);
			}
		}

		if (verbosity & VERB_GENERATE_BASE_PRS)
			cout << implicant << "\t" << implicant_output << endl;
		final_output = final_output | implicant_output;
		down.implicants.push_back(implicant);
	}

	if (verbosity & VERB_GENERATE_BASE_PRS)
	{
		cout << endl;
		cout << "Desired:  " << g->down[uid] << endl;
		cout << "Obtained: " << final_output << endl;
		cout << endl;
	}
}

void rule::gen_primes()
{
	if (verbosity & VERB_GENERATE_BASE_PRS)
		cout << "Generating Prime Implicants for " << uid << endl;

	up.gen_primes();
	down.gen_primes();

	if (verbosity & VERB_GENERATE_BASE_PRS)
		cout << endl;
}

void rule::gen_essentials()
{
	if (verbosity & VERB_GENERATE_BASE_PRS)
		cout << "Generating Essential Prime Implicants for " << uid << endl;

	up.gen_essentials();
	down.gen_essentials();

	if (verbosity & VERB_GENERATE_BASE_PRS)
		cout << endl;
}

void rule::gen_output(vspace *v)
{
	up.gen_output();
	down.gen_output();

	if (verbosity & VERB_GENERATE_BASE_PRS)
		cout << endl << endl << endl << endl;
}

ostream &operator<<(ostream &os, rule r)
{
	list<state>::iterator i;

	if (r.up.implicants.size() > 0)
		os << r.up.simple << " -> " << r.name << "+" << endl;
	if (r.down.implicants.size() > 0)
		os << r.down.simple << " -> " << r.name << "-" << endl;

    return os;
}
