#include "state.h"

/*
 * We defined a whole suite of state operators, but it got too beefy.
 * Many of them were thus moved here.
 */
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

	result.prs = s1.prs;

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

	result.prs = s1.prs;

	return result;
}

state operator&(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "&" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i & s2);

	result.prs = s1.prs;

	return result;
}

state operator|(state s1, value s2)
{
	state result;
	vector<value>::iterator i;

	//result.var =  s1.var + "|" + s2.data;
	for (i = s1.values.begin(); i != s1.values.end(); i++)
		result.values.push_back(*i | s2);

	result.prs = s1.prs;

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

	result.prs = s.prs;

	return result;
}

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

	result.prs = s1.prs;

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

	result.prs = s1.prs;

	return result;
}

state operator!(state s)
{
	state result;
	vector<value>::iterator i;

	//result.var =  "~" + s.var;
	for (i = s.values.begin(); i != s.values.end(); i++)
		result.values.push_back(!*i);

	result.prs = s.prs;

	return result;
}
