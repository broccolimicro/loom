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
