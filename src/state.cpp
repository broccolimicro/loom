/*
 * state.cpp
 *
 *  Created on: Oct 30, 2012
 *      Author: Ned Bingham
 */

#include "state.h"

state::state()
{
	data = "";
	prs = false;
}

state::state(string d, bool p)
{
	data = d;
	prs = p;
}

state::~state()
{
	data = "";
	prs = false;
}

state &state::operator=(state s)
{
	data = s.data;
	prs = s.prs;
	return *this;
}

state &state::operator=(string s)
{
	if (s[0] == 'o')
		prs = true;
	else
		prs = false;
	data = s.substr(1);

	return *this;
}

state &state::operator+=(state s)
{
	*this = *this + s;
	return *this;
}

state &state::operator-=(state s)
{
	*this = *this - s;
	return *this;
}

state &state::operator*=(state s)
{
	*this = *this * s;
	return *this;
}

state &state::operator/=(state s)
{
	*this = *this / s;
	return *this;
}

state &state::operator&=(state s)
{
	*this = *this & s;
	return *this;
}

state &state::operator|=(state s)
{
	*this = *this | s;
	return *this;
}

state &state::operator<<=(int n)
{
	*this = *this << n;
	return *this;
}

state &state::operator>>=(int n)
{
	*this = *this >> n;
	return *this;
}

state operator+(state s1, state s2)
{
	string::reverse_iterator j, k;
	state result;
	char carry = '0';
	char a, b;

	int x, i, o;

	for (j = s1.data.rbegin(), k = s2.data.rbegin(); j != s1.data.rend() || k != s2.data.rend();)
	{
		a = j != s1.data.rend() ? *j++ : '0';
		b = k != s2.data.rend() ? *k++ : '0';

		x = (a == 'X') + (b == 'X') + (carry == 'X');
		i = (a == '1') + (b == '1') + (carry == '1');
		o = (a == '0') + (b == '0') + (carry == '0');

		if (x == 0 && 2*(i/2) != i)
			result.data += "1";
		else if (x == 0)
			result.data += "0";
		else
			result.data += "X";

		if (i >= 2)
			carry = '1';
		else if (o >= 2)
			carry = '0';
		else
			carry = 'X';
	}

	reverse(result.data.begin(), result.data.end());
	result.prs = false;

	return result;
}

state operator-(state s1, state s2)
{
	return s1 + ~s2 + state("1", false);
}

state operator*(state s1, state s2)
{

}

state operator/(state s1, state s2)
{

}

state operator-(state s)
{

}

state operator&(state s1, state s2)
{

}

state operator|(state s1, state s2)
{

}

state operator~(state s)
{
	string::iterator j;
	state result;

	for (j = s.data.begin(); j != s.data.end(); j++)
	{
		if (*j == '0')
			result.data += "1";
		else if (*j == '1')
			result.data += "0";
		else
			result.data += "X";
	}

	result.prs = false;

	return result;
}

state operator<<(state s, int n)
{
	string str = "";
	for (int i = 0; i < n; i++)
		str += "0";
	return state(s.data.substr(n) + str, false);
}

state operator>>(state s, int n)
{
	return state(s.data.substr(0, s.data.length()-n), false);
}

state operator==(state s1, state s2)
{

}

state operator!=(state s1, state s2)
{

}

state operator<=(state s1, state s2)
{

}

state operator>=(state s1, state s2)
{

}

state operator<(state s1, state s2)
{

}

state operator>(state s1, state s2)
{

}

