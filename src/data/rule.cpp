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
}

void rule::gen_primes()
{
	size_t i, j;
	vector<state> primes;
	vector<state> temp;

	for (i = 0; i < up_implicants.size(); i++)
	{

		for (j = i+1; j < up_implicants.size(); j++)
		{

		}
	}
}

void rule::gen_essentials()
{

}

void rule::gen_output(vspace *v)
{
	vector<state>::iterator i;
	int j;
	bool first;

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
}

void rule::clear()
{
	up = "";
	down = "";
	up_implicants.clear();
	down_implicants.clear();
}

/* This function returns the nth necessary firing of a production rule.
 *
 */
/*int rule::index(int n)
{
	vector<value>::iterator i;
	int j;
	for (i = desired.begin(), j = 0; i != desired.end() && n > 0; i++, j++)
		if (i->data == "1")
			n--;

	return j;
}*/


/*//Reduce all implicants to prime
rule reduce_to_prime(rule pr)
{
	rule result = pr;
	if (result.implicants.size() < 2)
		return result;
	//Reduce to prime guards
	//Totally is a more efficient/logical way to do this
	size_t i = 0;
	size_t j = 0;
	bool removed_junk;
	removed_junk = true;
	while(i != result.implicants.size()-1)
	{
		removed_junk = false;
		for (i = 0; (i < result.implicants.size()-1) && !removed_junk; i++)
		{
			for (j = i+1; j < result.implicants.size() && !removed_junk; j++)
			{
				int unneeded_index;
				//cout << i << ": " << result.implicants[i] << endl;
				//cout << j << ": " << result.implicants[j] << endl;
				unneeded_index = which_index_unneeded(result.implicants[i], result.implicants[j]);
				//cout << "Between " << i << " and " << j <<" Unneeded = " << unneeded_index << endl;

				if(unneeded_index != -1)
				{
					result.implicants[i][unneeded_index].data = "X";
					result.implicants[j][unneeded_index].data = "X";
					removed_junk = true;
				}
			}//inner for
		}//Outer for
	} // while
	return result;
}

rule remove_too_strong(rule pr)
{
	rule result = pr;
	if (result.implicants.size() < 2)
		return result;
	//Eliminate all 'unneccisarily strong' guards
	//Totally is a more efficient/logical way to do this
	size_t i = 0;
	size_t j = 0;
	bool removed_junk;
	removed_junk = true;
	while(i != result.implicants.size()-1)
	{
		removed_junk = false;
		for (i = 0; (i < result.implicants.size()-1) && !removed_junk; i++)
		{
			for (j = i+1; j < result.implicants.size() && !removed_junk; j++)
			{
				int weaker_result;
				//cout << i << ": " << result.implicants[i] << endl;
				//cout << j << ": " << result.implicants[j] << endl;
				weaker_result = who_weaker(result.implicants[i], result.implicants[j]);
				//cout << "Between " << i << " and " << j <<" who_weaker = " << weaker_result << endl;
				if(weaker_result == -1)
				{
					vector<state>::iterator vi = result.implicants.begin();
					for(size_t counter = 0; counter < i; counter++)
						vi++;
					result.implicants.erase(vi);
					removed_junk = true;
				}
				else if(weaker_result == 1)
				{
					vector<state>::iterator vi = result.implicants.begin();
					for(size_t counter = 0; counter < j; counter++)
						vi++;
					result.implicants.erase(vi);
					removed_junk = true;
				}
				else if(weaker_result == 2)
				{
					vector<state>::iterator vi = result.implicants.begin();
					for(size_t counter = 0; counter < i; counter++)
						vi++;
					result.implicants.erase(vi);
					removed_junk = true;
				}
			}//inner for
		}//Outer for
	} // while
	return result;
}

//Given a single rule, minimize the implicants to that rule
rule minimize_rule(rule pr)
{
	rule result = pr;
	result = remove_too_strong(result);
	result = reduce_to_prime(result);
	result = remove_too_strong(result);
	//cout << "finished " << pr.right << endl;
	return result;
}

//Given a vector of rules, minimize every implicant in that vector
vector<rule> minimize_rule_vector(vector<rule> prs)
{
	vector<rule> result = prs;

	for (int i = 0; i < (int)result.size(); i++)
	{
		//cout << "trying " << prs[i].right << " ("<< i << ")" << endl;
		result[i] = minimize_rule(result[i]);
	}
	return result;

}*/

void print_implicant_tags(vector<state> implicants)
{
	int size = implicants.size();
	for(int i = 0; i < size; i++){
		if(i != 0)
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
