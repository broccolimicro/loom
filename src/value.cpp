/*
 * value.cpp
 *
 *  Created on: Oct 30, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "value.h"

value::value()
{
	data = "";
}

value::value(string d)
{
	data = d;
}

value::~value()
{
	data = "";
}

value &value::operator=(value s)
{
	data = s.data;
	return *this;
}

value &value::operator=(string s)
{
	data = s;

	return *this;
}

value &value::operator+=(value s)
{
	*this = *this + s;
	return *this;
}

value &value::operator-=(value s)
{
	*this = *this - s;
	return *this;
}

value &value::operator*=(value s)
{
	*this = *this * s;
	return *this;
}

value &value::operator/=(value s)
{
	*this = *this / s;
	return *this;
}

value &value::operator&=(value s)
{
	*this = *this & s;
	return *this;
}

value &value::operator|=(value s)
{
	*this = *this | s;
	return *this;
}

value &value::operator<<=(value s)
{
	*this = *this << s;
	return *this;
}

value &value::operator>>=(value s)
{
	*this = *this >> s;
	return *this;
}

value &value::operator<<=(int n)
{
	*this = *this << n;
	return *this;
}

value &value::operator>>=(int n)
{
	*this = *this >> n;
	return *this;
}

value value::operator[](size_t i)
{
	stringstream str;
	string s;

	if (i < data.length())
		str << data[i];
	else
		str << '0';

	str >> s;
	return value(s);
}

bool subset(value s1, value s2)
{
	string::reverse_iterator j, k;
	char a, b;

	for (j = s1.data.rbegin(), k = s2.data.rbegin(); j != s1.data.rend() || k != s2.data.rend();)
	{
		a = j != s1.data.rend() ? *j++ : '0';
		b = k != s2.data.rend() ? *k++ : '0';

		if ((a == '1' && !(b == '1' || b == '_')) || (a == '0' && !(b == '0' || b == '_')) || (a == '_' && b != '_'))
			return false;
	}

	return true;
}

ostream &operator<<(ostream &os, value s)
{
	os << s.data;
	return os;
}

value operator+(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	string::reverse_iterator j, k;
	value result;
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
			result.data = "1" + result.data;
		else if (x == 0)
			result.data = "0" + result.data;
		else
			result.data = "X" + result.data;

		if (i >= 2)
			carry = '1';
		else if (o >= 2)
			carry = '0';
		else
			carry = 'X';
	}

	if(carry!='0'){
		result.data = carry + result.data;
	}

	return result;
}

value operator-(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	return s1 + ~s2 + value("1");
}

value operator*(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	value result;
	value mult = s1;

	string::reverse_iterator j;

	for (j = s2.data.rbegin(); j != s2.data.rend(); j++)
	{
		if (*j == '1')
			result += mult;
		else if (*j == 'X')
			result += (mult & value(string(mult.data.length(), 'X')));

		mult += mult;
	}

	if (result.data.empty())
		result.data = "0";

	return result;
}

value operator/(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	return value("");
}

value operator-(value s)
{
	if (s.data.find_first_of("_") != s.data.npos)
		return value("_");

	return ~s + value("1");
}

value operator&(value s1, value s2)
{
	string::reverse_iterator j, k;
	value result;
	char a, b;

	for (j = s1.data.rbegin(), k = s2.data.rbegin(); j != s1.data.rend() || k != s2.data.rend();)
	{
		a = j != s1.data.rend() ? *j++ : '0';
		b = k != s2.data.rend() ? *k++ : '0';

		if (a == '0' || b == '0')
			result.data = "0" + result.data;
		else if (a == 'X' || b == 'X')
			result.data = "X" + result.data;
		else
			result.data = "1" + result.data;
	}

	return result;
}

value operator|(value s1, value s2)
{
	string::reverse_iterator j, k;
	value result;
	char a, b;

	for (j = s1.data.rbegin(), k = s2.data.rbegin(); j != s1.data.rend() || k != s2.data.rend();)
	{
		a = j != s1.data.rend() ? *j++ : '0';
		b = k != s2.data.rend() ? *k++ : '0';

		if (a == '1' || b == '1')
			result.data = "1" + result.data;
		else if (a == 'X' || b == 'X')
			result.data = "X" + result.data;
		else
			result.data = "0" + result.data;
	}

	return result;
}

value operator~(value s)
{
	string::iterator j;
	value result;

	for (j = s.data.begin(); j != s.data.end(); j++)
	{
		if (*j == '0')
			result.data += "1";
		else if (*j == '1')
			result.data += "0";
		else if (*j == 'X')
			result.data += "X";
		else
			result.data += "_";
	}

	return result;
}

value operator<<(value s, int n)
{
	return value(s.data + string(n, '0'));
}

value operator>>(value s, int n)
{
	return value((string(n, '0') + s.data).substr(0, s.data.length()));
}

// DOES NOT HANDLE X VALUES
/* To handle X values, unroll the X values, do a separate
 * shift operation for every unrolled element, then union
 * all of the results together.
 */
value operator<<(value s1, value s2)
{
	string zeros;
	string mult = "0";
	string::reverse_iterator i;

	for (i = s2.data.rbegin(); i != s2.data.rend(); i++)
	{
		if (*i == '1')
			zeros += mult;

		mult += mult;
	}

	return value(s1.data + zeros);
}

// DOES NOT HANDLE X VALUES
/* To handle X values, unroll the X values, do a separate
 * shift operation for every unrolled element, then union
 * all of the results together.
 */
value operator>>(value s1, value s2)
{
	string zeros;
	string mult = "0";
	string::reverse_iterator i;

	for (i = s2.data.rbegin(); i != s2.data.rend(); i++)
	{
		if (*i == '1')
			zeros += mult;

		mult += mult;
	}

	return value((zeros + s1.data).substr(0, s1.data.length()));
}

value operator==(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	string::reverse_iterator j, k;
	char a, b;
	value result;

	result.data = "1";

	for (j = s1.data.rbegin(), k = s2.data.rbegin(); j != s1.data.rend() || k != s2.data.rend();)
	{
		a = j != s1.data.rend() ? *j++ : '0';
		b = k != s2.data.rend() ? *k++ : '0';

		if ((a == '1' && b == '0') || (a == '0' && b == '1'))
			return value("0");
		else if (a == 'X' || b == 'X')
			result.data = "X";
	}

	return result;
}

value operator!=(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	string::reverse_iterator j, k;
	char a, b;
	value result;

	result.data = "0";

	for (j = s1.data.rbegin(), k = s2.data.rbegin(); j != s1.data.rend() || k != s2.data.rend();)
	{
		a = j != s1.data.rend() ? *j++ : '0';
		b = k != s2.data.rend() ? *k++ : '0';

		if ((a == '1' && b == '0') || (a == '0' && b == '1'))
			return value("1");
		else if (a == 'X' || b == 'X')
			result.data = "X";
	}


	return result;
}

value operator<=(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	int j, k;
	int l0 = s1.data.length(), l1 = s2.data.length();
	string::iterator a, b;

	for (j = s1.data.find_first_of("1X"), k = s2.data.find_first_of("1X"); j >= 0 && k >= 0; j = s1.data.find_first_of("1X", j+1), k = s2.data.find_first_of("1X", k+1))
	{
		a = s1.data.begin() + j;
		b = s2.data.begin() + k;

		if (l0 - j < l1 - k)
		{
			if (*b == '1')
				return value("1");
			else if (*b == 'X')
				return value("X");
		}
		else if (l0 - j > l1 - k)
		{
			if (*a == '1')
				return value("0");
			else if (*a == 'X')
				return value("X");
		}
		else
			if (*a == 'X' || *b == 'X')
				return value("X");

	}

	if (j >= 0 && k < 0)
		return value("0");

	return value("1");
}

value operator>=(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	int j, k;
	int l0 = s1.data.length(), l1 = s2.data.length();
	string::iterator a, b;
	//cout << "jinit = " <<  s1.data.find_first_of("1X") << " and kinit = " << s2.data.find_first_of("1X") << endl;
	for (j = s1.data.find_first_of("1X"), k = s2.data.find_first_of("1X"); j >= 0 && k >= 0; j = s1.data.find_first_of("1X", j+1), k = s2.data.find_first_of("1X", k+1))
	{
		//cout << "j = " << j << " k = " << k <<endl;
		a = s1.data.begin() + j;
		b = s2.data.begin() + k;
		//cout << "a = " << *a << " b = " << *b << endl;
		if (l0 - j < l1 - k)
		{
			if (*b == '1')
				return value("0");
			else if (*b == 'X')
				return value("X");
		}
		else if (l0 - j > l1 - k)
		{
			if (*a == '1')
				return value("1");
			else if (*a == 'X')
				return value("X");
		}
		else
			if (*a == 'X' || *b == 'X')
				return value("X");

	}

	if (j < 0 && k >= 0)
		return value("0");

	return value("1");
}

value operator<(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	int j, k;
	int l0 = s1.data.length(), l1 = s2.data.length();
	string::iterator a, b;

	for (j = s1.data.find_first_of("1X"), k = s2.data.find_first_of("1X"); j >= 0 && k >= 0; j = s1.data.find_first_of("1X", j+1), k = s2.data.find_first_of("1X", k+1))
	{
		a = s1.data.begin() + j;
		b = s2.data.begin() + k;

		if (l0 - j < l1 - k)
		{
			if (*b == '1')
				return value("1");
			else if (*b == 'X')
				return value("X");
		}
		else if (l0 - j > l1 - k)
		{
			if (*a == '1')
				return value("0");
			else if (*a == 'X')
				return value("X");
		}
		else
			if (*a == 'X' || *b == 'X')
				return value("X");

	}

	if (j < 0 && k >= 0)
		return value("1");

	return value("0");
}

value operator>(value s1, value s2)
{
	if (s1.data.find_first_of("_") != s1.data.npos || s2.data.find_first_of("_") != s2.data.npos)
		return value("_");

	int j, k;
	int l0 = s1.data.length(), l1 = s2.data.length();
	string::iterator a, b;

	for (j = s1.data.find_first_of("1X"), k = s2.data.find_first_of("1X"); j >= 0 && k >= 0; j = s1.data.find_first_of("1X", j+1), k = s2.data.find_first_of("1X", k+1))
	{
		a = s1.data.begin() + j;
		b = s2.data.begin() + k;

		if (l0 - j < l1 - k)
		{
			if (*b == '1')
				return value("0");
			else if (*b == 'X')
				return value("X");
		}
		else if (l0 - j > l1 - k)
		{
			if (*a == '1')
				return value("1");
			else if (*a == 'X')
				return value("X");
		}
		else
			if (*a == 'X' || *b == 'X')
				return value("X");

	}

	if (j >= 0 && k < 0)
		return value("1");

	return value("0");
}

value operator!(value s)
{
	string::iterator j;
	value result;

	for (j = s.data.begin(); j != s.data.end(); j++)
	{
		if (*j == '_')
			result.data += "X";
		else if (*j == '0')
			result.data += "1";
		else if (*j == '1')
			result.data += "0";
		else
			result.data += "_";
	}

	return result;
}

value operator||(value s1, value s2)
{
	string::reverse_iterator j, k;
	value result;
	string a, b;

	for (j = s1.data.rbegin(), k = s2.data.rbegin(); j != s1.data.rend() || k != s2.data.rend();)
	{
		a = j != s1.data.rend() ? *j++ : '0';
		b = k != s2.data.rend() ? *k++ : '0';

		if (a == "_")
			result.data = b + result.data;
		else if (b == "_")
			result.data = a + result.data;
		else if (a == b)
			result.data = a + result.data;
		else
			result.data = "X" + result.data;
	}

	return result;
}


//There can be four values: 1, 0, X, _ where underscore is empty set and X is full set.
value operator&&(value s1, value s2)
{
	string::reverse_iterator j, k;
	value result;
	string a, b;


	result.data = "";
	for (j = s1.data.rbegin(), k = s2.data.rbegin(); j != s1.data.rend() || k != s2.data.rend(); j++, k++)
	{

		a = j != s1.data.rend() ? *j : '0';
		b = k != s2.data.rend() ? *k : '0';

		if (a == "X")
			result.data = b + result.data;
		else if(b == "X")
			result.data = a + result.data;
		else if (a == b)
			result.data = a + result.data;
		else if( a == "_" || b == "_")
			result.data = "_" + result.data;
		else
			result.data = "_" + result.data;
	}

	return result;
}

