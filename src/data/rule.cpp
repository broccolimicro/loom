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
	this->up = "";
	this->down = "";
	this->up_implicants.clear();
	this->down_implicants.clear();
}

rule::rule(int uid)
{
	this->uid = uid;
	this->up = "";
	this->down = "";
	this->up_implicants.clear();
	this->down_implicants.clear();
}

rule::rule(int uid, graph *g, vspace *v)
{
	this->uid = uid;
	this->up = "";
	this->down = "";
	this->up_implicants.clear();
	this->down_implicants.clear();

	gen_minterms(g);
	gen_primes();
	gen_essentials();
	gen_output(v);
}

rule::~rule()
{
	this->uid = -1;
	this->up = "";
	this->down = "";
	this->up_implicants.clear();
	this->down_implicants.clear();
}

rule &rule::operator=(rule r)
{
	uid = r.uid;
	up = r.up;
	down = r.down;
	up_implicants = r.up_implicants;
	down_implicants = r.down_implicants;
	return *this;
}

void rule::gen_minterms(graph *g)
{
	cout << "Generating Minterms for " << uid << endl;
	list<int> invars;
	list<int>::iterator vk;

	state implier;
	state implicant;
	trace implicant_output;
	trace proposal_output;

	int vj, ii;
	int count, mcount, var;

	for (ii = 0; ii < (int)g->up_firings[uid].size(); ii++)
	{
		implier			 = g->states[g->up_firings[uid][ii]];
		implicant		 = state(value("X"), g->width());
		implicant_output = trace(value("1"), g->size());

		invars.clear();
		for (vj = 0; vj < g->width(); vj++)
			if (((BUBBLELESS && implier[vj].data == "0") || !BUBBLELESS) && vj != uid)
				invars.push_back(vj);

		mcount = 9999999;
		var = 0;
		while (invars.size() > 0 && var != -1)
		{
			var = -1;
			for (vk = invars.begin(); vk != invars.end(); vk++)
			{
				proposal_output = implicant_output & ~g->delta[*vk];

				count = conflict_count(proposal_output, g->up[uid]);
				if (count < mcount)
				{
					mcount = count;
					var = *vk;
				}
			}

			if (var != -1)
			{
				implicant[var] = value("0");
				implicant_output = implicant_output & ~g->delta[var];
				invars.remove(var);
			}
		}

		implicant.tag = g->up_firings[uid][ii];
		up_implicants.push_back(implicant);
	}

	for (ii = 0; ii < (int)g->down_firings[uid].size(); ii++)
	{
		implier			 = g->states[g->down_firings[uid][ii]];
		implicant		 = state(value("X"), g->width());
		implicant_output = trace(value("1"), g->size());

		invars.clear();
		for (vj = 0; vj < g->width(); vj++)
			if (((BUBBLELESS && implier[vj].data == "1") || !BUBBLELESS) && vj != uid)
				invars.push_back(vj);

		mcount = 9999999;
		var = 0;
		while (invars.size() > 0 && var != -1)
		{
			var = -1;
			for (vk = invars.begin(); vk != invars.end(); vk++)
			{
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
				implicant[var] = value("1");
				implicant_output = implicant_output & g->delta[var];
				invars.remove(var);
			}
		}

		implicant.tag = g->down_firings[uid][ii];
		down_implicants.push_back(implicant);
	}
	cout << endl;
}

void rule::gen_primes()
{
	vector<state> t[2];
	state implicant;
	size_t i, j;

	vector<int> count;
	int count_sum;

	cout << "Generating Prime Implicants for " << uid << endl;

	// Up Implicants
	cout << "Up Minterms\t";
	t[1] = up_implicants;
	for (i = 0; i < t[1].size(); i++)
		cout << "[" << t[1][i] << "] ";
	cout << endl;

	count_sum = t[1].size();
	while (count_sum > 0)
	{
		t[0].clear();
		count.clear();
		count.resize(t[1].size(), 0);
		for (i = 0; i < t[1].size(); i++)
		{
			for (j = i+1; j < t[1].size(); j++)
			{
				if (diff_count(t[1][i], t[1][j]) == 1)
				{
					implicant = t[1][i] || t[1][j];
					count[i]++;
					count[j]++;
					if (find(t[0].begin(), t[0].end(), implicant) == t[0].end())
						t[0].push_back(implicant);
				}
			}
		}
		count_sum = 0;
		for (i = 0; i < t[1].size(); i++)
		{
			count_sum += count[i];
			if (count[i] == 0)
				up_primes.push_back(t[1][i]);
		}

		t[1] = t[0];
	}

	cout << "Up Primes\t";
	for (i = 0; i < up_primes.size(); i++)
		cout << "[" << up_primes[i] << "] ";
	cout << endl;

	// Cleanup
	t[0].clear();
	t[1].clear();
	count.clear();

	// Down Implicants
	cout << "Down Minterms\t";
	t[1] = down_implicants;
	for (i = 0; i < t[1].size(); i++)
		cout << "[" << t[1][i] << "] ";
	cout << endl;

	count_sum = t[1].size();
	while (count_sum > 0)
	{
		t[0].clear();
		count.clear();
		count.resize(t[1].size(), 0);
		for (i = 0; i < t[1].size(); i++)
		{
			for (j = i+1; j < t[1].size(); j++)
			{
				if (diff_count(t[1][i], t[1][j]) == 1)
				{
					implicant = t[1][i] || t[1][j];
					count[i]++;
					count[j]++;
					if (find(t[0].begin(), t[0].end(), implicant) == t[0].end())
						t[0].push_back(implicant);
				}
			}
		}
		count_sum = 0;
		for (i = 0; i < t[1].size(); i++)
		{
			count_sum += count[i];
			if (count[i] == 0)
				down_primes.push_back(t[1][i]);
		}

		t[1] = t[0];
	}

	cout << "Down Primes\t";
	for (i = 0; i < down_primes.size(); i++)
		cout << "[" << down_primes[i] << "] ";
	cout << endl << endl;
}

void rule::gen_essentials()
{
	cout << "Generating Essential Prime Implicants for " << uid << endl;
	size_t i, j;

	cout << "Up Essentials" << endl;
	up_essential.resize(up_implicants.size(), vector<state>());
	for (j = 0; j < up_implicants.size(); j++)
	{
		cout << up_implicants[j] << " is covered by ";
		for (i = 0; i < up_primes.size(); i++)
			if (subset(up_primes[i], up_implicants[j]))
			{
				cout << "[" << up_primes[i] << "] ";
				up_essential[j].push_back(up_primes[i]);
			}
		cout << endl;
	}

	cout << "Down Essentials" << endl;
	down_essential.resize(down_implicants.size(), vector<state>());
	for (j = 0; j < down_implicants.size(); j++)
	{
		cout << down_implicants[j] << " is covered by ";
		for (i = 0; i < down_primes.size(); i++)
			if (subset(down_primes[i], down_implicants[j]))
			{
				cout << "[" << down_primes[i] << "] ";
				down_essential[j].push_back(down_primes[i]);
			}
		cout << endl;
	}

	cout << endl;
}

void rule::gen_output(vspace *v)
{
	vector<state>::iterator i;
	int j;
	bool first;

	cout << "Combining Essential Prime Implicants for " << uid << endl;

	up = "";
	down = "";
	for (i = up_implicants.begin(); i != up_implicants.end(); i++)
	{
		if (i != up_implicants.begin())
			up += " | ";

		first = true;
		for (j = 0; j < i->size(); j++)
		{
			if (i->values[j].data == "0")
			{
				if (!first)
					up += "&";
				up += "~" + v->get_name(j);
				first = false;
			}
			else if (i->values[j].data == "1")
			{
				if (!first)
					up += "&";
				up += v->get_name(j);
				first = false;
			}
		}
	}
	if (up == "")
		up += "0";

	up += " -> " + v->get_name(uid) + "+";

	cout << up << endl;

	for (i = down_implicants.begin(); i != down_implicants.end(); i++)
	{
		if (i != down_implicants.begin())
			down += " | ";

		first = true;
		for (j = 0; j < i->size(); j++)
		{
			if (i->values[j].data == "0")
			{
				if (!first)
					down += "&";
				down += "~" + v->get_name(j);
				first = false;
			}
			else if (i->values[j].data == "1")
			{
				if (!first)
					down += "&";
				down += v->get_name(j);
				first = false;
			}
		}
	}
	if (down == "")
		down += "0";

	down += " -> " + v->get_name(uid) + "-";

	cout << down << endl;

	cout << endl << endl << endl << endl;
}

void rule::clear()
{
	up = "";
	down = "";
	up_implicants.clear();
	down_implicants.clear();
}

void print_implicant_tags(vector<state> implicants)
{
	int size = implicants.size();
	for (int i = 0; i < size; i++)
	{
		if (i != 0)
			cout << ", ";
		cout << implicants[i].tag;
	}
}

ostream &operator<<(ostream &os, rule r)
{
	list<state>::iterator i;

    os << r.up << endl << r.down << endl;

    return os;
}
