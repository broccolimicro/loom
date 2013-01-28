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
#include "common.h"

state::state()
{
}

state::~state()
{
}

state::state(vector<value> v)
{
	values = v;
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

void state::insert(int i, value v)
{
	values.insert(values.begin() + i, v);
}

ostream &operator<<(ostream &os, state s)
{
    vector<value>::iterator i;
    for (i = s.values.begin(); i != s.values.end(); i++)
    	os << *i << " ";

    return os;
}

state operator+(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "+" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a + b);
	}

	return result;
}

state operator-(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "-" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a - b);
	}

	return result;
}

state operator*(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "*" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a * b);
	}

	return result;
}

state operator/(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "/" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a / b);
	}

	return result;
}


state operator+(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "+" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i + s2);

	return result;
}

state operator-(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "-" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i - s2);

	return result;
}

state operator*(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "*" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i * s2);

	return result;
}

state operator/(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "/" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i / s2);

	return result;
}

state operator+(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "+" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 + *i);

	return result;
}

state operator-(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "-" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 - *i);

	return result;
}

state operator*(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "*" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 * *i);

	return result;
}

state operator/(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "/" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 / *i);

	return result;
}

state operator-(state s)
{
	state result;
	vector<value>::iterator i;

	//result.var =  "-" + s.var;
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(-*i);

	return result;
}

state operator&(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "&" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a & b);
	}

	return result;
}

state operator|(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "|" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a | b);
	}

	return result;
}

state operator&(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "&" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i & s2);

	return result;
}

state operator|(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "|" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i | s2);

	return result;
}

state operator&(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "&" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 & *i);

	return result;
}

state operator|(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "|" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 | *i);

	return result;
}

state operator~(state s)
{
	state result;
	vector<value>::iterator i;

	//result.var =  "~" + s.var;
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(~*i);

	return result;
}

state operator<<(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "<<" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a << b);
	}

	return result;
}

state operator>>(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + ">>" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a >> b);
	}

	return result;
}

state operator<<(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var = s1.var + "<<" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i << s2);

	return result;
}

state operator>>(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var = s1.var + ">>" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i >> s2);

	return result;
}

state operator<<(value s1, state s2)
{
	state result;
	vector<value>::iterator i, j;

	//result.var = s1.data + "<<" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 << *i);

	return result;
}

state operator>>(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var = s1.data + ">>" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 >> *i);

	return result;
}

state operator<<(state s, int n)
{
	state result;
	vector<value>::iterator i;

	//result.var = s.var + "<<" + to_string(n);
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(*i << n);

	return result;
}

state operator>>(state s, int n)
{
	state result;
	vector<value>::iterator i;

	//result.var = s.var + ">>" + to_string(n);
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(*i >> n);

	return result;
}

/*state operator<(state s1, int n)
{
	s1.values.push_back(value("0", false));
	s1.values.pop_front();
	return s1;
}

state operator>(state s1, int n)
{
	s1.values.push_front(value("0", false));
	s1.values.pop_back();
	return s1;
}*/

state operator==(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "==" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a == b);
	}

	return result;
}

state operator!=(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "~=" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a != b);
	}

	return result;
}

state operator<=(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "<=" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a <= b);
	}

	return result;
}

state operator>=(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + ">=" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a >= b);
	}

	return result;
}

state operator<(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "<" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a < b);
	}

	return result;
}

state operator>(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + ">" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : s1.values.back();
		b = k != s2.values.end() ? *k++ : s2.values.back();

		result.values.push_back(a > b);
	}

	return result;
}

state operator==(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "==" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i == s2);

	return result;
}

state operator!=(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "~=" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i != s2);

	return result;
}

state operator<=(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "<=" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i <= s2);

	return result;
}

state operator>=(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + ">=" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i >= s2);

	return result;
}

state operator<(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "<" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i < s2);

	return result;
}

state operator>(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + ">" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i > s2);

	return result;
}

state operator==(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "==" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 == *i);

	return result;
}

state operator!=(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "~=" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 != *i);

	return result;
}

state operator<=(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "<=" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 <= *i);

	return result;
}

state operator>=(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + ">=" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 >= *i);

	return result;
}

state operator<(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + "<" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 < *i);

	return result;
}

state operator>(value s1, state s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.data + ">" + s2.var;
	for (i = s2.values.begin(); i != s2.values.end(); i++)
		result.values.push_back(s1 > *i);

	return result;
}

int count(state s)
{
	int result = 0;
	vector<value>::iterator i;
	for (i = s.values.begin(); i != s.values.end(); i++)
	{
		if (i->data == "1" || i->data == "X")
			result++;
	}

	return result;
}

int strict_count(state s)
{
	int result = 0;
	vector<value>::iterator i;
	for (i = s.values.begin(); i != s.values.end(); i++)
	{
		if (i->data == "1")
			result++;
	}

	return result;
}

/*int delta_count(state s)
{
	vector<value>::iterator i;
	string last = "";
	int cnt = 0;

	for (i = s.values.begin(); i != s.values.end(); i++)
	{
		if (i->data == "1" && last != "1" && i->prs)
			cnt++;
		last = i->data;
	}

	return cnt;
}

state up(state s)
{
	vector<value>::iterator i, j;
	state result;
	string str;
	string::iterator si, sj;
	result.var = s.var + "+";

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

state up(state s, int idx)
{
	vector<value>::iterator i, j;
	state result;
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

state down(state s)
{
	vector<value>::iterator i, j;
	state result;
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
}

state down(state s, int idx)
{
	vector<value>::iterator i, j;
	state result;
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

/*bool drive(state s)
{
	bool result = false;
	vector<value>::iterator i;
	for (i = s.values.begin(); i != s.values.end(); i++)
		result = result || i->prs;

	return result;
}*/

/* This function compares the left state to the right state. The right
 * state is what we desire for this production rule, and the left state
 * is what we have managed to generate.
 *
 * Format for conflicts string:
 * . is 'allowable',
 * E is error,
 * C is conflict if no value variable
 * ! is necessary fire
 */
string conflicts(state left, state right)
{
	vector<value>::iterator i,j;
	string conflict = "";

	//Loop through all of the production rule values (left) and the corresponding desired functionality (right)
	for (i = left.values.begin(),j = right.values.begin() ; i != left.values.end() && j != right.values.end(); i++, j++)
	{
		if(i->data == "0" && j->data == "0" )
			conflict += ".";		// Doesn't fire, shouldn't fire. Good.
		else if(i->data == "0" && j->data == "1" )
		{
			cout << "Error: Production rule missing necessary firing." << endl;
			conflict += "E";		// Error fire! Our PRS aren't good enough.
		}
		else if(i->data == "1" && j->data == "0" )
			conflict += "C";		// Illegal fire (fires when it shouldn't)
		else if(i->data == "1" && j->data == "1" )
			conflict += "!";		// This fires, and it must keep firing after we after we add a value var
		else if(j->data == "X" )
			conflict += ".";		// Don't really care if it fires or not. Ambivalence.
		else if(i->data == "X" && j->data == "0")
			conflict += "C";
		else
		{
			cout << "Error: The value variable generation algorithm is very confused right now." << endl;
			conflict += "E";		// Error fire! Not quite sure how you got here...
		}
	}

	return conflict;
}
