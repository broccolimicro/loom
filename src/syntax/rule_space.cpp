/*
 * rule_space.cpp
 *
 *  Created on: Aug 23, 2013
 *      Author: nbingham
 */

#include "rule_space.h"

rule_space::rule_space()
{

}

rule_space::~rule_space()
{

}

void rule_space::insert(rule r)
{
	if ((int)rules.size() <= r.uid)
		rules.resize(r.uid+1);
	rules[r.uid] = r;
}

int rule_space::size()
{
	return (int)rules.size();
}

rule &rule_space::operator[](int i)
{
	return rules[i];
}

void rule_space::apply_one_of(logic *s, vector<int> a, int v)
{
	logic temp;
	minterm ex;
	int j;
	bool first = true;

	for (j = 0; j < (int)a.size(); j++)
		if (rules[a[j]].vars != NULL)
		{
			if (first)
			{
				ex = minterm(rules[a[j]].uid, 1-v);
				first = false;
			}
			else
				ex &= minterm(rules[a[j]].uid, 1-v);
		}

	if (((*s) & ex) != 0)
		for (j = 0; j < (int)a.size(); j++)
			if (rules[a[j]].vars != NULL && (rules[a[j]].guards[v] & (*s)) != 0)
				temp |= ((rules[a[j]].guards[v] & ex & (*s)) >> logic(rules[a[j]].uid, v)) | ((~rules[a[j]].guards[v] | ~ex) & (*s));

	if (temp.terms.size() != 0)
		*s = temp;
}

logic rule_space::apply(logic s)
{
	/*logic test;

	do
	{
		cout << "LOOK " << s.print(vars) << endl;
		test = s;
		for (int i = 0; i < (int)excl.size(); i++)
			apply_one_of(&s, excl[i].first, excl[i].second);
	} while (test != s);

	return s;*/

	return s;
}
