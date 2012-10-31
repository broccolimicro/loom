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

ostream &operator<<(ostream &os, state s)
{
	os << (s.prs ? "o" : "i") << s.data;
	return os;
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
	state result;
	state mult = s1;

	string::reverse_iterator j;

	for (j = s2.data.rbegin(); j != s2.data.rend(); j++)
	{
		if (*j == '1')
			result += mult;
		else if (*j == 'X')
			result += (mult & state(string(mult.data.length(), 'X'), false));

		mult += mult;
	}

	return result;
}

state operator/(state s1, state s2)
{

}

state operator-(state s)
{
	return ~s + state("1", false);
}

state operator&(state s1, state s2)
{
	string::iterator j, k;
	state result;
	char a, b;

	for (j = s1.data.begin(), k = s2.data.begin(); j != s1.data.end() || k != s2.data.end();)
	{
		a = j != s1.data.end() ? *j++ : '0';
		b = k != s2.data.end() ? *k++ : '0';

		if (a == '0' || b == '0')
			result.data += '0';
		else if (a == 'X' || b == 'X')
			result.data += 'X';
		else
			result.data += '1';
	}

	result.prs = false;

	return result;
}

state operator|(state s1, state s2)
{
	string::iterator j, k;
	state result;
	char a, b;

	for (j = s1.data.begin(), k = s2.data.begin(); j != s1.data.end() || k != s2.data.end();)
	{
		a = j != s1.data.end() ? *j++ : '0';
		b = k != s2.data.end() ? *k++ : '0';

		if (a == '1' || b == '1')
			result.data += '1';
		else if (a == 'X' || b == 'X')
			result.data += 'X';
		else
			result.data += '0';
	}

	result.prs = false;

	return result;
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
	return state(s.data + string(n, '0'), false);
}

state operator>>(state s, int n)
{
	return state(s.data.substr(0, s.data.length()-n), false);
}

state operator==(state s1, state s2)
{
	string::iterator j, k;
	char a, b;
	state result;

	result.data = "1";

	for (j = s1.data.begin(), k = s2.data.begin(); j != s1.data.end() || k != s2.data.end();)
	{
		a = j != s1.data.end() ? *j++ : '0';
		b = k != s2.data.end() ? *k++ : '0';

		if ((a == '1' && b == '0') || (a == '0' && b == '1'))
			return state("0", false);
		else if (a == 'X' || b == 'X')
			result.data = "X";
	}

	result.prs = false;

	return result;
}

state operator!=(state s1, state s2)
{
	string::iterator j, k;
	char a, b;
	state result;

	result.data = "0";

	for (j = s1.data.begin(), k = s2.data.begin(); j != s1.data.end() || k != s2.data.end();)
	{
		a = j != s1.data.end() ? *j++ : '0';
		b = k != s2.data.end() ? *k++ : '0';

		if ((a == '1' && b == '0') || (a == '0' && b == '1'))
			return state("1", false);
		else if (a == 'X' || b == 'X')
			result.data = "X";
	}

	result.prs = false;

	return result;
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

