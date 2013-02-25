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
	prs = false;
}

state::~state()
{
}

state::state(vector<value> v)
{
	values = v;
	prs = false;
}

state::state(value v, int c)
{
	prs = false;
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

value &state::operator[](int i)
{
	return values[i];
}

state &state::operator=(state s)
{
	values = s.values;
	prs = s.prs;
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

state &state::operator<<=(state s)
{
	*this = *this << s;
	return *this;
}

state &state::operator>>=(state s)
{
	*this = *this >> s;
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

state &state::operator+=(value s)
{
	*this = *this + s;
	return *this;
}

state &state::operator-=(value s)
{
	*this = *this - s;
	return *this;
}

state &state::operator*=(value s)
{
	*this = *this * s;
	return *this;
}

state &state::operator/=(value s)
{
	*this = *this / s;
	return *this;
}

state &state::operator&=(value s)
{
	*this = *this & s;
	return *this;
}

state &state::operator|=(value s)
{
	*this = *this | s;
	return *this;
}

state &state::operator<<=(value s)
{
	*this = *this << s;
	return *this;
}

state &state::operator>>=(value s)
{
	*this = *this >> s;
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

bool is_all_x(state s1)
{
	for(int i = 0; i < s1.size(); i++)
		if(s1[i].data != "X")
			return false;

	return true;

}
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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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

//TODO: Switch the other operators too. I am stealing this one and turning it into a boolean compare.
/*state operator==(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "==" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

		result.values.push_back(a != b);
	}

	return result;
}*/
bool operator==(state s1, state s2)
{
	if(s1.size() != s2.size())
		return false;
	for(int i = 0;i<s1.size();i++)
	{
		if(s1[i].data != s2[i].data)
			return false;
	}
	return true;

}

bool operator!=(state s1, state s2)
{
	if(s1.size() != s2.size())
		return true;
	for(int i = 0;i<s1.size();i++)
	{
		if(s1[i].data != s2[i].data)
			return true;
	}
	return false;

}

state operator<=(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "<=" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

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
		a = j != s1.values.end() ? *j++ : value("X");
		b = k != s2.values.end() ? *k++ : value("X");

		result.values.push_back(a > b);
	}

	return result;
}

state operator||(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "|" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : value("?");
		b = k != s2.values.end() ? *k++ : value("?");

		result.values.push_back(a || b);
	}

	return result;
}

state operator&&(state s1, state s2)
{
	vector<value>::iterator j, k;
	value a, b;
	state result;

	//result.var = s1.var + "|" + s2.var;

	for (j = s1.values.begin(), k = s2.values.begin(); j != s1.values.end() || k != s2.values.end();)
	{
		a = j != s1.values.end() ? *j++ : value("?");
		b = k != s2.values.end() ? *k++ : value("?");

		result.values.push_back(a && b);
	}

	return result;

}

state operator!(state s)
{
	state result;
	vector<value>::iterator i;

	//result.var =  "~" + s.var;
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(!*i);

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

/*
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
/*
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
*/

