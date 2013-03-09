/*
 * trace.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "trace.h"
#include "value.h"
#include "../common.h"

trace::trace()
{
}

trace::trace(value v, int s)
{
	for (int i = 0; i < s; i++)
		values.push_back(v);
}

trace::trace(vector<value> v)
{
	values = v;
}

trace::trace(string s)
{
	values.push_back(value(s));
}

trace::~trace()
{
}

void trace::clear()
{
	values.clear();
}

int trace::size()
{
	return values.size();
}

void trace::assign(int i, value v)
{
	if (i >= (int)values.size())
		values.resize(i+1, value("X"));
	values[i] = v;
}

vector<value>::iterator trace::begin()
{
	return values.begin();
}

vector<value>::iterator trace::end()
{
	return values.end();
}

void trace::push_back(value v)
{
	values.push_back(v);
}

trace &trace::operator=(trace s)
{
	values = s.values;
	return *this;
}

trace &trace::operator+=(trace s)
{
	*this = *this + s;
	return *this;
}

trace &trace::operator-=(trace s)
{
	*this = *this - s;
	return *this;
}

trace &trace::operator*=(trace s)
{
	*this = *this * s;
	return *this;
}

trace &trace::operator/=(trace s)
{
	*this = *this / s;
	return *this;
}

trace &trace::operator<<=(trace s)
{
	*this = *this << s;
	return *this;
}

trace &trace::operator>>=(trace s)
{
	*this = *this >> s;
	return *this;
}

trace &trace::operator&=(trace s)
{
	*this = *this & s;
	return *this;
}

trace &trace::operator|=(trace s)
{
	*this = *this | s;
	return *this;
}

trace &trace::operator+=(value s)
{
	*this = *this + s;
	return *this;
}

trace &trace::operator-=(value s)
{
	*this = *this - s;
	return *this;
}

trace &trace::operator*=(value s)
{
	*this = *this * s;
	return *this;
}

trace &trace::operator/=(value s)
{
	*this = *this / s;
	return *this;
}

trace &trace::operator&=(value s)
{
	*this = *this & s;
	return *this;
}

trace &trace::operator|=(value s)
{
	*this = *this | s;
	return *this;
}

trace &trace::operator<<=(value s)
{
	*this = *this << s;
	return *this;
}

trace &trace::operator>>=(value s)
{
	*this = *this >> s;
	return *this;
}

trace &trace::operator<<=(int n)
{
	*this = *this << n;
	return *this;
}

trace &trace::operator>>=(int n)
{
	*this = *this >> n;
	return *this;
}


value &trace::operator[](int i)
{
	return values[i];
}

trace trace::operator()(int i)
{
	trace result;
	//result.var = var;
	//if (values.begin()->data.length() > 1)
	//	result.var += "[" + to_string(i) + "]";

	vector<value>::iterator j;
	for (j = values.begin(); j != values.end(); j++)
		result.values.push_back((*j)[(size_t)i]);

	return result;
}

ostream &operator<<(ostream &os, trace s)
{
    vector<value>::iterator i;
    for (i = s.values.begin(); i != s.values.end(); i++)
    	os << *i << " ";

    return os;
}

trace operator+(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "+" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a + b);
	}

	return result;
}

trace operator-(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "-" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a - b);
	}

	return result;
}

trace operator*(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "*" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a * b);
	}

	return result;
}

trace operator/(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "/" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a / b);
	}

	return result;
}


trace operator+(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "+" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i + s2);

	return result;
}

trace operator-(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "-" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i - s2);

	return result;
}

trace operator*(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "*" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i * s2);

	return result;
}

trace operator/(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "/" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i / s2);

	return result;
}

trace operator+(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "+" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 + *i);

	return result;
}

trace operator-(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "-" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 - *i);

	return result;
}

trace operator*(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "*" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 * *i);

	return result;
}

trace operator/(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "/" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 / *i);

	return result;
}

trace operator-(trace s)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  "-" + s.var;
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(-*i);

	return result;
}

trace operator&(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "&" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a & b);
	}

	return result;
}

trace operator|(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "|" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a | b);
	}

	return result;
}

trace operator||(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "||" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a || b);
	}

	return result;
}

trace operator&&(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "||" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a && b);
	}

	return result;
}

trace operator&(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "&" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i & s2);

	return result;
}

trace operator|(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "|" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i | s2);

	return result;
}

trace operator&(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "&" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 & *i);

	return result;
}

trace operator|(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "|" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 | *i);

	return result;
}

trace operator~(trace s)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  "~" + s.var;
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(~*i);

	return result;
}

trace operator<<(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "<<" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a << b);
	}

	return result;
}

trace operator>>(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + ">>" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a >> b);
	}

	return result;
}

trace operator<<(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var = s1.var + "<<" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i << s2);

	return result;
}

trace operator>>(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var = s1.var + ">>" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i >> s2);

	return result;
}

trace operator<<(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i, j;

	//result.var = s1.data + "<<" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 << *i);

	return result;
}

trace operator>>(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var = s1.data + ">>" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 >> *i);

	return result;
}

trace operator<<(trace s, int n)
{
	trace result;
	vector<value>::iterator i;

	//result.var = s.var + "<<" + to_string(n);
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(*i << n);

	return result;
}

trace operator>>(trace s, int n)
{
	trace result;
	vector<value>::iterator i;

	//result.var = s.var + ">>" + to_string(n);
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(*i >> n);

	return result;
}

/*trace operator<(trace s1, int n)
{
	s1.values.push_back(value("0", false));
	s1.values.pop_front();
	return s1;
}

trace operator>(trace s1, int n)
{
	s1.values.push_front(value("0", false));
	s1.values.pop_back();
	return s1;
}*/

trace operator==(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "==" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a == b);
	}

	return result;
}

trace operator!=(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "~=" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a != b);
	}

	return result;
}

trace operator<=(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "<=" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a <= b);
	}

	return result;
}

trace operator>=(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + ">=" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a >= b);
	}

	return result;
}

trace operator<(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + "<" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a < b);
	}

	return result;
}

trace operator>(trace s1, trace s2)
{
	vector<value>::iterator j, k;
	value a, b;
	trace result;

	//result.var = s1.var + ">" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a > b);
	}

	return result;
}

trace operator==(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "==" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i == s2);

	return result;
}

trace operator!=(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "~=" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i != s2);

	return result;
}

trace operator<=(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "<=" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i <= s2);

	return result;
}

trace operator>=(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + ">=" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i >= s2);

	return result;
}

trace operator<(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + "<" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i < s2);

	return result;
}

trace operator>(trace s1, value s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.var + ">" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i > s2);

	return result;
}

trace operator==(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "==" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 == *i);

	return result;
}

trace operator!=(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "~=" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 != *i);

	return result;
}

trace operator<=(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "<=" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 <= *i);

	return result;
}

trace operator>=(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + ">=" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 >= *i);

	return result;
}

trace operator<(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + "<" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 < *i);

	return result;
}

trace operator>(value s1, trace s2)
{
	trace result;
	vector<value>::iterator i;

	//result.var =  s1.data + ">" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 > *i);

	return result;
}

int count(trace s)
{
	int result = 0;
	vector<value>::iterator i;
	for (i = s.values.begin(); i != s.values.end(); i++)
		if (i->data == "1" || i->data == "X")
			result++;

	return result;
}

int strict_count(trace s)
{
	int result = 0;
	vector<value>::iterator i;
	for (i = s.values.begin(); i != s.values.end(); i++)
		if (i->data == "1")
			result++;

	return result;
}

/*trace up(trace s)
{
	vector<value>::iterator i, j;
	trace result;
	string str;
	string::iterator si, sj;

	j = s.values.begin();

	j++;

	for (i = s.values.begin(); j != s.values.end(); i++, j++)
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
		result.values.push_back(value(str, j->prs));
	}

	result.values.push_back(value("X", false));

	return result;
}

trace down(trace s)
{
	vector<value>::iterator i, j;
	trace result;
	string str;
	string::iterator si, sj;
	result.var = s.var + "-";

	j = s.values.begin();

	j++;

	for (i = s.values.begin(); j != s.values.end(); i++, j++)
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
		result.values.push_back(value(str, j->prs));
	}

	result.values.push_back(value("X", false));

	return result;
}*/

/*trace up(trace s, int idx)
{
	vector<value>::iterator i, j;
	trace result;
	string str;
	string::iterator si, sj;
	int cnt = 0;
	bool one = false;

	result.var = s.var + "+";

	j = s.values.begin();

	j++;

	for (i = s.values.begin(); j != s.values.end(); i++, j++)
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

			if (*sj == '1' && j->prs)
				one = true;

			if (*sj == '0' && one)
			{
				cnt++;
				one = false;
			}
		}
		result.values.push_back(value(str, j->prs));
	}

	result.values.push_back(value("X", false));

	return result;
}



trace down(trace s, int idx)
{
	vector<value>::iterator i, j;
	trace result;
	string str;
	string::iterator si, sj;
	int cnt = 0;
	bool zero = false;
	result.var = s.var + "-";

	j = s.values.begin();

	j++;

	for (i = s.values.begin(); j != s.values.end(); i++, j++)
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

			if (*sj == '0' && j->prs)
				zero = true;

			if (*sj == '1' && zero)
			{
				cnt++;
				zero = false;
			}
		}
		result.values.push_back(value(str, j->prs));
	}

	result.values.push_back(value("X", false));

	return result;
}*/

/* This function compares the left trace to the right trace. The right
 * trace is what we desire for this production rule, and the left trace
 * is what we have managed to generate.
 *
 * Format for conflicts string:
 * . is 'allowable',
 * E is error,
 * C is conflict if no value variable
 * ! is necessary fire
 */
trace conflicts(trace left, trace right)
{
	vector<value>::iterator i,j;
	trace conflict;

	//Loop through all of the production rule values (left) and the corresponding desired functionality (right)
	for (i = left.values.begin(),j = right.values.begin() ; i != left.values.end() && j != right.values.end(); i++, j++)
	{
		if (i->data == "0" && j->data == "0")
			conflict.push_back(value("."));		// Doesn't fire, shouldn't fire. Good.
		else if (i->data == "0" && j->data == "1")
		{
			cout << "Error: Production rule missing necessary firing." << endl;
			conflict.push_back(value("E"));		// Error fire! Our PRS aren't good enough.
		}
		else if (i->data == "1" && j->data == "0")
			conflict.push_back(value("C"));		// Illegal fire (fires when it shouldn't)
		else if (i->data == "1" && j->data == "1")
			conflict.push_back(value("!"));		// This fires, and it must keep firing after we after we add a value var
		else if (j->data == "X" )
			conflict.push_back(value("."));		// Don't really care if it fires or not. Ambivalence.
		else if (i->data == "X" && j->data == "0")
			conflict.push_back(value("C"));
		else
		{
			cout << "Error: The value variable generation algorithm is very confused right now." << endl;
			conflict.push_back(value("E"));		// Error fire! Not quite sure how you got here...
		}
	}

	return conflict;
}
