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
	//actual.clear();
	left = "";
	//desired.clear();
	right = "";
}

rule::~rule()
{
	//actual.clear();
	left = "";
	//desired.clear();
	right = "";
}

rule &rule::operator=(rule s)
{
	left = s.left;
	right = s.right;
	implicants = s.implicants;
	uid = s.uid;
	up = s.up;
	return *this;
}

void rule::clear(int n)
{
	//actual.clear();
	//for (int i = 0; i < n; i++)
	//	actual.push_back(value("1"));
	left = "";
	//desired.clear();
	right = "";
	implicants.clear();
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


//Reduce all implicants to prime
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

}

ostream &operator<<(ostream &os, rule r)
{
	list<state>::iterator i;

    os << r.left << " -> " << r.right;

    return os;
}
