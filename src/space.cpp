/*
 * space.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "space.h"
#include "state.h"
#include "common.h"

space::space()
{
	var = "";
}

space::space(string v, list<state> s)
{
	var = v;
	states = s;
}

space::~space()
{
	var = "";
}

space &space::operator=(space s)
{
	var = s.var;
	states = s.states;
	return *this;
}

space &space::operator+=(space s)
{
	*this = *this + s;
	return *this;
}

space &space::operator-=(space s)
{
	*this = *this - s;
	return *this;
}

space &space::operator*=(space s)
{
	*this = *this * s;
	return *this;
}

space &space::operator/=(space s)
{
	*this = *this / s;
	return *this;
}

space &space::operator<<=(space s)
{
	*this = *this << s;
	return *this;
}

space &space::operator>>=(space s)
{
	*this = *this >> s;
	return *this;
}

space &space::operator&=(space s)
{
	*this = *this & s;
	return *this;
}

space &space::operator|=(space s)
{
	*this = *this | s;
	return *this;
}

space &space::operator+=(state s)
{
	*this = *this + s;
	return *this;
}

space &space::operator-=(state s)
{
	*this = *this - s;
	return *this;
}

space &space::operator*=(state s)
{
	*this = *this * s;
	return *this;
}

space &space::operator/=(state s)
{
	*this = *this / s;
	return *this;
}

space &space::operator&=(state s)
{
	*this = *this & s;
	return *this;
}

space &space::operator|=(state s)
{
	*this = *this | s;
	return *this;
}

space &space::operator<<=(state s)
{
	*this = *this << s;
	return *this;
}

space &space::operator>>=(state s)
{
	*this = *this >> s;
	return *this;
}

space &space::operator<<=(int n)
{
	*this = *this << n;
	return *this;
}

space &space::operator>>=(int n)
{
	*this = *this >> n;
	return *this;
}

space space::operator[](int i)
{
	space result;
	result.var = var;
	if (states.begin()->data.length() > 1)
		result.var += "[" + to_string(i) + "]";

	list<state>::iterator j;
	for (j = states.begin(); j != states.end(); j++)
		result.states.push_back((*j)[i]);

	return result;
}

ostream &operator<<(ostream &os, space s)
{
    os << s.var << " : ";
    list<state>::iterator i;
    for (i = s.states.begin(); i != s.states.end(); i++)
    	os << *i << " ";

    return os;
}

space operator+(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " + " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a + b);
	}

	return result;
}

space operator-(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " - " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a - b);
	}

	return result;
}

space operator*(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " * " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a * b);
	}

	return result;
}

space operator/(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " / " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a / b);
	}

	return result;
}


space operator+(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " + " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i + s2);

	return result;
}

space operator-(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " - " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i - s2);

	return result;
}

space operator*(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " * " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i * s2);

	return result;
}

space operator/(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " / " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i / s2);

	return result;
}

space operator+(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " + " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 + *i);

	return result;
}

space operator-(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " - " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 - *i);

	return result;
}

space operator*(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " * " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 * *i);

	return result;
}

space operator/(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " / " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 / *i);

	return result;
}

space operator-(space s)
{
	space result;
	list<state>::iterator i;

	result.var =  "-" + s.var;
	for (i = s.states.begin(); i != s.states.end(); i++)
		result.states.push_back(-*i);

	return result;
}

space operator&(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " & " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a & b);
	}

	return result;
}

space operator|(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " | " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a | b);
	}

	return result;
}

space operator&(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " & " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i & s2);

	return result;
}

space operator|(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " | " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i | s2);

	return result;
}

space operator&(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " & " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 & *i);

	return result;
}

space operator|(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " | " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 | *i);

	return result;
}

space operator~(space s)
{
	space result;
	list<state>::iterator i;

	result.var =  "~" + s.var;
	for (i = s.states.begin(); i != s.states.end(); i++)
		result.states.push_back(~*i);

	return result;
}

space operator<<(space s1, space s2)
{
	space result;
	list<state>::iterator i, j;

	result.var = s1.var + " << " + s2.var;
	for (i = s1.states.begin(), j = s2.states.begin(); i != s1.states.end() && j != s2.states.end(); i++, j++)
		result.states.push_back(*i << *j);

	return result;
}

space operator>>(space s1, space s2)
{
	space result;
	list<state>::iterator i, j;

	result.var = s1.var + " >> " + s2.var;
	for (i = s1.states.begin(), j = s2.states.begin(); i != s1.states.end() && j != s2.states.end(); i++, j++)
		result.states.push_back(*i >> *j);

	return result;
}

space operator<<(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var = s1.var + " << " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i << s2);

	return result;
}

space operator>>(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var = s1.var + " >> " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i >> s2);

	return result;
}

space operator<<(state s1, space s2)
{
	space result;
	list<state>::iterator i, j;

	result.var = s1.data + " << " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 << *i);

	return result;
}

space operator>>(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var = s1.data + " >> " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 >> *i);

	return result;
}

space operator<<(space s, int n)
{
	space result;
	list<state>::iterator i;

	result.var = s.var + " << " + to_string(n);
	for (i = s.states.begin(); i != s.states.end(); i++)
		result.states.push_back(*i << n);

	return result;
}

space operator>>(space s, int n)
{
	space result;
	list<state>::iterator i;

	result.var = s.var + " >> " + to_string(n);
	for (i = s.states.begin(); i != s.states.end(); i++)
		result.states.push_back(*i >> n);

	return result;
}

space operator<(space s1, int n)
{
	s1.states.push_back(state("0", false));
	s1.states.pop_front();
	return s1;
}

space operator>(space s1, int n)
{
	s1.states.push_front(state("0", false));
	s1.states.pop_back();
	return s1;
}

space operator==(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " == " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a == b);
	}

	return result;
}

space operator!=(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " ~= " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a != b);
	}

	return result;
}

space operator<=(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " <= " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a <= b);
	}

	return result;
}

space operator>=(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " >= " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a >= b);
	}

	return result;
}

space operator<(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " < " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a < b);
	}

	return result;
}

space operator>(space s1, space s2)
{
	list<state>::iterator j, k;
	state a, b;
	space result;

	result.var = s1.var + " > " + s2.var;

	for (j = s1.states.begin(), k = s2.states.begin(); j != s1.states.end() || k != s2.states.end();)
	{
		a = j != s1.states.end() ? *j++ : state("0", false);
		b = k != s2.states.end() ? *k++ : state("0", false);

		result.states.push_back(a > b);
	}

	return result;
}

space operator==(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " == " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i == s2);

	return result;
}

space operator!=(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " ~= " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i != s2);

	return result;
}

space operator<=(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " <= " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i <= s2);

	return result;
}

space operator>=(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " >= " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i >= s2);

	return result;
}

space operator<(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " < " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i < s2);

	return result;
}

space operator>(space s1, state s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.var + " > " + s2.data;
	for (i = s1.states.begin(); i != s1.states.end(); i++)
		result.states.push_back(*i > s2);

	return result;
}

space operator==(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " == " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 == *i);

	return result;
}

space operator!=(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " ~= " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 != *i);

	return result;
}

space operator<=(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " <= " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 <= *i);

	return result;
}

space operator>=(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " >= " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 >= *i);

	return result;
}

space operator<(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " < " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 < *i);

	return result;
}

space operator>(state s1, space s2)
{
	space result;
	list<state>::iterator i;

	result.var =  s1.data + " > " + s2.var;
	for (i = s2.states.begin(); i != s2.states.end(); i++)
		result.states.push_back(s1 > *i);

	return result;
}

int count(space s)
{
	int result = 0;
	list<state>::iterator i;
	for (i = s.states.begin(); i != s.states.end(); i++)
	{
		if (i->data == "1" || i->data == "X")
			result++;
	}

	return result;
}

int strict_count(space s)
{
	int result = 0;
	list<state>::iterator i;
	for (i = s.states.begin(); i != s.states.end(); i++)
	{
		if (i->data == "1")
			result++;
	}

	return result;
}

int delta_count(space s)
{
	list<state>::iterator i;
	string last = "";
	int cnt = 0;

	for (i = s.states.begin(); i != s.states.end(); i++)
	{
		if (i->data == "1" && last != "1" && i->prs)
			cnt++;
		last = i->data;
	}

	return cnt;
}

space up(space s)
{
	list<state>::iterator i, j;
	space result;
	string str;
	string::iterator si, sj;
	result.var = s.var + "+";

	j = s.states.begin();
	j++;

	for (i = s.states.begin(); j != s.states.end(); i++, j++)
	{
		str = "";
		for (si = i->data.begin(), sj = j->data.begin(); si != i->data.end() && sj != j->data.end(); si++, sj++)
		{
			if (*sj == '1' && *si != '1' && j->prs)
				str = str + "1";
			else if (*sj == '1' && *si == '1')
				str = str + "X";
			else
				str = str + "0";
		}
		result.states.push_back(state(str, j->prs));
	}

	return result;
}

space up(space s, int idx)
{
	list<state>::iterator i, j;
	space result;
	string str;
	string::iterator si, sj;
	int cnt = 0;
	bool one = false;

	result.var = s.var + "+";

	j = s.states.begin();
	j++;

	for (i = s.states.begin(); j != s.states.end(); i++, j++)
	{
		str = "";
		for (si = i->data.begin(), sj = j->data.begin(); si != i->data.end() && sj != j->data.end(); si++, sj++)
		{
			if (*sj == '1' && *si != '1' && j->prs && cnt == idx)
				str = str + "1";
			else if (*sj == '1' && *si != '1' && j->prs && cnt != idx)
				str = str + "X";
			else if (*sj == '1' && *si == '1')
				str = str + "X";
			else
				str = str + "0";

			if (*sj == '1')
				one = true;

			if (*sj == '0' && one)
			{
				cnt++;
				one = false;
			}
		}
		result.states.push_back(state(str, j->prs));
	}

	return result;
}

space down(space s)
{
	list<state>::iterator i, j;
	space result;
	string str;
	string::iterator si, sj;
	result.var = s.var + "-";

	j = s.states.begin();
	j++;

	for (i = s.states.begin(); j != s.states.end(); i++, j++)
	{
		str = "";
		for (si = i->data.begin(), sj = j->data.begin(); si != i->data.end() && sj != j->data.end(); si++, sj++)
		{
			if (*sj == '0' && *si != '0' && j->prs)
				str = str + "1";
			else if (*sj == '0' && *si == '0')
				str = str + "X";
			else
				str = str + "0";
		}
		result.states.push_back(state(str, j->prs));
	}

	return result;
}

space down(space s, int idx)
{
	list<state>::iterator i, j;
	space result;
	string str;
	string::iterator si, sj;
	int cnt = 0;
	bool one = false;
	result.var = s.var + "-";

	j = s.states.begin();
	j++;

	for (i = s.states.begin(); j != s.states.end(); i++, j++)
	{
		str = "";
		for (si = i->data.begin(), sj = j->data.begin(); si != i->data.end() && sj != j->data.end(); si++, sj++)
		{
			if (*sj == '0' && *si != '0' && j->prs && cnt == idx)
				str = str + "1";
			else if (*sj == '0' && *si != '0' && j->prs && cnt != idx)
				str = str + "X";
			else if (*sj == '0' && *si == '0')
				str = str + "X";
			else
				str = str + "0";

			if (*sj == '1')
				one = true;

			if (*sj == '0' && one)
			{
				cnt++;
				one = false;
			}
		}
		result.states.push_back(state(str, j->prs));
	}

	return result;
}

bool drive(space s)
{
	bool result = false;
	list<state>::iterator i;
	for (i = s.states.begin(); i != s.states.end(); i++)
		result = result || i->prs;

	return result;
}
