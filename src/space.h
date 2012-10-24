/*
 * space.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
 */

#include "common.h"

#ifndef space_h
#define space_h

struct space
{
	space()
	{
		var = "";
	}
	space(string v, list<string> s)
	{
		var = v;
		states = s;
	}
	~space()
	{
		var = "";
	}

	string			var;
	list<string>	states;

	space &operator=(space s)
	{
		var = s.var;
		states = s.states;
		return *this;
	}
};

ostream &operator<<(ostream &os, space s)
{
    os << s.var << " : ";
    list<string>::iterator i;
    for (i = s.states.begin(); i != s.states.end(); i++)
    	os << *i << " ";

    return os;
}

space operator==(space s1, space s2)
{
	space result;
	list<string>::iterator i, j;
	string state;

	result.var = s1.var + " == " + s2.var;

	for (i = s1.states.begin(), j = s2.states.begin(); i != s1.states.end() && j != s2.states.end(); i++, j++)
	{
		if (i->substr(1, 1) == "X" || j->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) == atoi(j->substr(1).c_str())));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator!=(space s1, space s2)
{
	space result;
	list<string>::iterator i, j;
	string state;

	result.var = s1.var + " ~= " + s2.var;

	for (i = s1.states.begin(), j = s2.states.begin(); i != s1.states.end() && j != s2.states.end(); i++, j++)
	{
		if (i->substr(1, 1) == "X" || j->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) != atoi(j->substr(1).c_str())));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator>(space s1, space s2)
{
	space result;
	list<string>::iterator i, j;
	string state;

	result.var = s1.var + " > " + s2.var;

	for (i = s1.states.begin(), j = s2.states.begin(); i != s1.states.end() && j != s2.states.end(); i++, j++)
	{
		if (i->substr(1, 1) == "X" || j->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) > atoi(j->substr(1).c_str())));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator<(space s1, space s2)
{
	space result;
	list<string>::iterator i, j;
	string state;

	result.var = s1.var + " < " + s2.var;

	for (i = s1.states.begin(), j = s2.states.begin(); i != s1.states.end() && j != s2.states.end(); i++, j++)
	{
		if (i->substr(1, 1) == "X" || j->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) < atoi(j->substr(1).c_str())));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator>=(space s1, space s2)
{
	space result;
	list<string>::iterator i, j;
	string state;

	result.var = s1.var + " >= " + s2.var;

	for (i = s1.states.begin(), j = s2.states.begin(); i != s1.states.end() && j != s2.states.end(); i++, j++)
	{
		if (i->substr(1, 1) == "X" || j->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) >= atoi(j->substr(1).c_str())));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator<=(space s1, space s2)
{
	space result;
	list<string>::iterator i, j;
	string state;

	result.var = s1.var + " <= " + s2.var;

	for (i = s1.states.begin(), j = s2.states.begin(); i != s1.states.end() && j != s2.states.end(); i++, j++)
	{
		if (i->substr(1, 1) == "X" || j->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) <= atoi(j->substr(1).c_str())));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator==(space s1, int s2)
{
	space result;
	list<string>::iterator i;
	string state;

	result.var = s1.var + " == " + to_string(s2);

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) == s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator==(int s2, space s1)
{
	space result;
	list<string>::iterator i;
	string state;

	result.var = to_string(s2) + " == " + s1.var;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) == s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator!=(space s1, int s2)
{
	space result;
	result.var = s1.var + " ~= " + to_string(s2);

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) != s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator!=(int s2, space s1)
{
	space result;
	result.var = to_string(s2) + " ~= " + s1.var;

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) != s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator<(space s1, int s2)
{
	space result;
	result.var = s1.var + " < " + to_string(s2);

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) < s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator<(int s2, space s1)
{
	space result;
	result.var = to_string(s2) + " < " + s1.var;

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) > s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator>(space s1, int s2)
{
	space result;
	result.var = s1.var + " > " + to_string(s2);

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) > s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator>(int s2, space s1)
{
	space result;
	result.var = to_string(s2) + " > " + s1.var;

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) < s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator<=(space s1, int s2)
{
	space result;
	result.var = s1.var + " <= " + to_string(s2);

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) <= s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator<=(int s2, space s1)
{
	space result;
	result.var = to_string(s2) + " <= " + s1.var;

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) >= s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator>=(space s1, int s2)
{
	space result;
	result.var = s1.var + " >= " + to_string(s2);

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) >= s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

space operator>=(int s2, space s1)
{
	space result;
	result.var = to_string(s2) + " >= " + s1.var;

	list<string>::iterator i;
	string state;

	for (i = s1.states.begin(); i != s1.states.end(); i++)
	{
		if (i->substr(1, 1) == "X")
			state = "X";
		else
			state = to_string((int)(atoi(i->substr(1).c_str()) <= s2));
		result.states.push_back(i->substr(0, 1) + state);
	}

	return result;
}

#endif
