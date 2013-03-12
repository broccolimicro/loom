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
	trace final_output;
	trace proposal_output;

	int vj, ii;
	int count, mcount, var;

	cout << "Up Minterms" << endl;
	final_output = trace(value("1"), g->up[uid].size());
	for (ii = 0; ii < (int)g->up_firings[uid].size(); ii++)
	{
		implier			 = g->states[g->up_firings[uid][ii]];
		implicant		 = state(value("X"), g->width());
		implicant_output = trace(value("1"), g->up[uid].size());

		invars.clear();
		for (vj = 0; vj < g->width(); vj++)
			if (((BUBBLELESS && implier[vj].data == "0") || !BUBBLELESS) && vj != uid)
				invars.push_back(vj);

		mcount = conflict_count(implicant_output, g->up[uid]);
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

		cout << implicant << "\t" << implicant_output << endl;
		final_output = final_output | implicant_output;
		implicant.tag = g->up_firings[uid][ii];
		up_implicants.push_back(implicant);
	}
	cout << endl;
	cout << "Desired:  " << g->up[uid] << endl;
	cout << "Obtained: " << final_output << endl;

	cout << "Down Minterms" << endl;
	final_output = trace(value("1"), g->up[uid].size());
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

		cout << implicant << "\t" << implicant_output << endl;
		final_output = final_output | implicant_output;
		implicant.tag = g->down_firings[uid][ii];
		down_implicants.push_back(implicant);
	}
	cout << endl;
	cout << "Desired:  " << g->down[uid] << endl;
	cout << "Obtained: " << final_output << endl;
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
	map<size_t, vector<size_t> > cov, Tcov;
	vector<size_t>::iterator ci;
	size_t i, j, k;
	size_t max_count = up_implicants.size();
	size_t choice;

	// Up Implicants
	cout << "Up Prime Implicant Chart" << endl;
	cov.clear();
	for (j = 0; j < up_implicants.size(); j++)
		cov.insert(pair<size_t, vector<size_t> >(j, vector<size_t>()));
	for (j = 0; j < up_implicants.size(); j++)
	{
		cout << up_implicants[j] << " is covered by ";
		for (i = 0; i < up_primes.size(); i++)
			if (subset(up_primes[i], up_implicants[j]))
			{
				cout << "[" << up_primes[i] << "] ";
				cov[j].push_back(i);
			}

		if (cov[j].size() == 1)
			up_essentials.push_back(cov[j].front());

		cout << endl;
	}

	cout << endl;

	cout << "Up Essential Prime Implicants" << endl;
	for (j = 0; j < up_essentials.size(); j++)
		cout << "[" << up_primes[up_essentials[j]] << "]" << endl;
	cout << endl;

	Tcov.clear();
	for (j = 0; j < up_primes.size(); j++)
		Tcov.insert(pair<size_t, vector<size_t> >(j, vector<size_t>()));
	for (j = 0; j < cov.size(); j++)
	{
		for (i = 0; i < up_essentials.size(); i++)
			if (find(cov[j].begin(), cov[j].end(), up_essentials[i]) != cov[j].end())
				break;

		for (k = 0; i == up_essentials.size() && k < cov[j].size(); k++)
			Tcov[cov[j][k]].push_back(j);
	}

	cout << "Up Leftover Non-Essential Prime Implicants" << endl;
	for (i = 0; i < up_primes.size(); i++)
	{
		if (Tcov[i].size() > 0)
		{
			cout << up_primes[i] << " covers ";
			for (j = 0; j < Tcov[i].size(); j++)
				cout << "[" << up_implicants[Tcov[i][j]] << "] ";
			cout << endl;
		}
	}
	cout << endl;

	max_count = up_implicants.size();
	while (max_count > 0)
	{
		max_count = 0;
		for (i = 0; i < up_primes.size(); i++)
		{
			if (Tcov[i].size() > max_count)
			{
				max_count = Tcov.size();
				choice = i;
			}
		}

		if (max_count > 0)
		{
			up_essentials.push_back(choice);

			for (i = 0; i < up_primes.size(); i++)
				for (j = 0; i != choice && j < Tcov[choice].size(); j++)
				{
					ci = find(Tcov[i].begin(), Tcov[i].end(), Tcov[choice][j]);
					if (ci != Tcov[i].end())
						Tcov[i].erase(ci);
				}

			Tcov[choice].clear();
		}
	}

	cout << "Up Best Essential Prime Implicants" << endl;
	for (j = 0; j < up_essentials.size(); j++)
		cout << "[" << up_primes[up_essentials[j]] << "]" << endl;
	cout << endl;


	// Down Implicants
	cout << "Down Prime Implicant Chart" << endl;
	cov.clear();
	for (j = 0; j < down_implicants.size(); j++)
		cov.insert(pair<size_t, vector<size_t> >(j, vector<size_t>()));
	for (j = 0; j < down_implicants.size(); j++)
	{
		cout << down_implicants[j] << " is covered by ";
		for (i = 0; i < down_primes.size(); i++)
			if (subset(down_primes[i], down_implicants[j]))
			{
				cout << "[" << down_primes[i] << "] ";
				cov[j].push_back(i);
			}

		if (cov[j].size() == 1)
			down_essentials.push_back(cov[j].front());

		cout << endl;
	}

	cout << endl;

	cout << "Down Essential Prime Implicants" << endl;
	for (j = 0; j < down_essentials.size(); j++)
		cout << "[" << down_primes[down_essentials[j]] << "]" << endl;
	cout << endl;

	Tcov.clear();
	for (j = 0; j < down_primes.size(); j++)
		Tcov.insert(pair<size_t, vector<size_t> >(j, vector<size_t>()));
	for (j = 0; j < cov.size(); j++)
	{
		for (i = 0; i < down_essentials.size(); i++)
			if (find(cov[j].begin(), cov[j].end(), down_essentials[i]) != cov[j].end())
				break;

		for (k = 0; i == down_essentials.size() && k < cov[j].size(); k++)
			Tcov[cov[j][k]].push_back(j);
	}

	cout << "Down Leftover Non-Essential Prime Implicants" << endl;
	for (i = 0; i < down_primes.size(); i++)
	{
		if (Tcov[i].size() > 0)
		{
			cout << down_primes[i] << " covers ";
			for (j = 0; j < Tcov[i].size(); j++)
				cout << "[" << down_implicants[Tcov[i][j]] << "] ";
			cout << endl;
		}
	}
	cout << endl;

	max_count = down_implicants.size();
	while (max_count > 0)
	{
		max_count = 0;
		for (i = 0; i < down_primes.size(); i++)
		{
			if (Tcov[i].size() > max_count)
			{
				max_count = Tcov.size();
				choice = i;
			}
		}

		if (max_count > 0)
		{
			down_essentials.push_back(choice);

			for (i = 0; i < down_primes.size(); i++)
				for (j = 0; i != choice && j < Tcov[choice].size(); j++)
				{
					ci = find(Tcov[i].begin(), Tcov[i].end(), Tcov[choice][j]);
					if (ci != Tcov[i].end())
						Tcov[i].erase(ci);
				}

			Tcov[choice].clear();
		}
	}

	cout << "Down Best Essential Prime Implicants" << endl;
	for (j = 0; j < down_essentials.size(); j++)
		cout << "[" << down_primes[down_essentials[j]] << "]" << endl;
	cout << endl;

	cout << endl;
}

void rule::gen_output(vspace *v)
{
	vector<size_t>::iterator i;
	int j;
	bool first;

	cout << "Combining Essential Prime Implicants for " << uid << endl;

	up = "";
	down = "";
	for (i = up_essentials.begin(); i != up_essentials.end(); i++)
	{
		if (i != up_essentials.begin())
			up += " | ";

		first = true;
		for (j = 0; j < up_primes[*i].size(); j++)
		{
			if (up_primes[*i].values[j].data == "0")
			{
				if (!first)
					up += "&";
				up += "~" + v->get_name(j);
				first = false;
			}
			else if (up_primes[*i].values[j].data == "1")
			{
				if (!first)
					up += "&";
				up += v->get_name(j);
				first = false;
			}
		}
		if (first)
			up += "1";
	}
	if (up == "")
		up += "0";

	up += " -> " + v->get_name(uid) + "+";

	cout << up << endl;

	for (i = down_essentials.begin(); i != down_essentials.end(); i++)
	{
		if (i != down_essentials.begin())
			down += " | ";

		first = true;
		for (j = 0; j < down_primes[*i].size(); j++)
		{
			if (down_primes[*i].values[j].data == "0")
			{
				if (!first)
					down += "&";
				down += "~" + v->get_name(j);
				first = false;
			}
			else if (down_primes[*i].values[j].data == "1")
			{
				if (!first)
					down += "&";
				down += v->get_name(j);
				first = false;
			}
		}
		if (first)
			down += "1";
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
