/*
 * state.cpp
 *
 *  Created on: Oct 30, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "state.h"
#include "value.h"
#include "../common.h"

state::state()
{
	prs.clear();
}

state::~state()
{
}

state::state(vector<value> v)
{
	values = v;
	prs.clear();
}

state::state(value v, int c)
{
	prs.clear();
	for (int i = 0; i < c; i++)
		values.push_back(v);
}

void state::clear()
{
	values.clear();
}

vector<value>::iterator state::begin()
{
	return values.begin();
}

vector<value>::iterator state::end()
{
	return values.end();
}

void state::assign(int i, value v, value r)
{
	if (i >= (int)values.size())
		values.resize(i+1, r);
	values[i] = v;
}

int state::size()
{
	return values.size();
}

bool state::fire(int uid)
{
	if (prs.size() <= uid/8)
		prs.resize(uid/8 + 1, 0);
	return (bool)((prs[uid/8] >> (uid%8)) & 0x01);
}

void state::drive(int uid)
{
	if (prs.size() <= uid/8)
		prs.resize(uid/8 + 1, 0);
	prs[uid/8] |= (0x01 << (uid%8));
}

void state::drive(int uid, value v, value r)
{
	if (uid >= (int)values.size())
		values.resize(uid+1, r);
	values[uid] = v;

	if (prs.size() <= uid/8)
		prs.resize(uid/8 + 1, 0);
	prs[uid/8] |= (0x01 << (uid%8));
}

state null(int s)
{
	return state(value("_"), s);
}

state full(int s)
{
	return state(value("X"), s);
}

bool is_all_x(state s1)
{
	for(int i = 0; i < s1.size(); i++)
		if(s1[i].data != "X")
			return false;

	return true;

}

/* Returns true if s2 is a subset of s1
 * Returns false otherwise
 */
bool subset(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;

	//result.var = s1.var + "==" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

		if (!subset(a, b))
			return false;
	}

	return true;
}

bool conflict(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;

	//result.var = s1.var + "==" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

		if (!conflict(a, b))
			return false;
	}

	return true;
}

bool up_conflict(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;

	//result.var = s1.var + "==" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

		if (!up_conflict(a, b))
			return false;
	}

	return true;
}

bool down_conflict(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;

	//result.var = s1.var + "==" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

		if (!down_conflict(a, b))
			return false;
	}

	return true;
}

ostream &operator<<(ostream &os, state s)
{
    vector<value>::iterator i;
    for (i = s.values.begin(); i != s.values.end(); i++)
    	os << *i << " ";

    return os;
}
//Calculates when, due to a state change, something must fire.
state diff(state s1, state s2)
{
	state result;
	if (s1.size() != s2.size())
	{
		cout << "state diff run on states with differing sizes. Returning the s1" << endl;
		return s1;
	}
	for( int i =0; i < s1.size(); i++)
	{
		result.values.push_back(diff(s1[i],s2[i]));

	}
	return result;
}


int which_index_unneeded(state s1, state s2)
{
	int result = -1;
	if (s1.size() != s2.size())
	{
		cout << "which_index_unneeded run on states with differing sizes. Return -1" << endl;
		return -1;
	}
	int how_many_diff = 0;
	//Count the number of indices that differ
	for( int i = 0; i < s1.size(); i++)
	{
		if (s1[i].data != s2[i].data)
		{
			how_many_diff++;
			result = i;
		}
	}
	if (how_many_diff == 1)
		return result;

	return -1;
}
//Takes two states. Returns 0 if neither is weaker, 1 if s1 is weaker, 2 if s2 is weaker.
int who_weaker(state s1, state s2)
{
	int result = 0;
	bool same = true;
	if (s1.size() != s2.size())
	{
		cout << "who weaker run on states with differing sizes. 0" << endl;
		return 0;
	}

	for( int i = 0; i < s1.size(); i++)
	{
		if (s1[i].data != s2[i].data)
			same = false;
	}

	//If they are the same, return -1
	if(same)
		return -1;

	if((same != (s1 == s2) )||(same == (s1 != s2)  ) )
		cout << "DUDE WE HAVE BIG PROBLEMS WITH STATE OPERATORS" << endl;

	for( int i = 0; i < s1.size(); i++)
	{
		//The have a definite disagreement. Neither is weaker. Return.
		if ((s1[i].data == "1" && s2[i].data == "0")||(s1[i].data == "1" && s2[i].data == "0"))
			return 0;
		if((s1[i].data == "X") && ((s2[i].data == "0")||(s2[i].data == "1")))
		{
			//x<y and x>y
			if (result == 2)
				return 0;
			result = 1;
		}

		if((s2[i].data == "X") && ((s1[i].data == "0")||(s1[i].data == "1")))
		{
			//x>y and x<y
			if (result == 1)
				return 0;
			result = 2;
		}
	}
	return result;
}

int diff_count(state s1, state s2)
{
	vector<value>::iterator i, j;
	int count = 0;

	for (i = s1.begin(), j = s2.begin(); i != s1.end() && j != s2.end(); i++, j++)
		count += diff_count(*i, *j);

	return count;
}

