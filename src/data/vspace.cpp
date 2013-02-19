/*
 * vspace.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: nbingham
 */

#include "vspace.h"

variable *vspace::find(int uid)
{
	map<string, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
			return &(i->second);
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
			return &(i->second);
	return NULL;
}

variable *vspace::find(string name)
{
	map<string, variable>::iterator i;
	i = global.find(name);
	if (i != global.end());
		return &(i->second);
	i = label.find(name);
	if (i != label.end())
		return &(i->second);

	return NULL;
}

string vspace::get_name(int uid)
{
	map<string, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
			return i->second.name;
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
			return i->second.name;
	return "";
}



string vspace::get_type(int uid)
{
	map<string, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
			return i->second.type;
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
			return i->second.type;
	return "";
}

string vspace::get_type(string name)
{
	map<string, variable>::iterator i;
	i = global.find(name);
	if (i != global.end())
		return i->second.type;
	i = label.find(name);
	if (i != label.end())
		return i->second.type;

	return "Error";
}

int vspace::get_uid(string name)
{
	map<string, variable>::iterator i;
	i = global.find(name);
	if (i != global.end());
		return i->second.uid;
	i = label.find(name);
	if (i != label.end())
		return i->second.uid;

	return -1;
}

int vspace::get_width(string name)
{
	map<string, variable>::iterator i;
	i = global.find(name);
	if (i != global.end())
		return i->second.width;
	i = label.find(name);
	if (i != label.end())
		return i->second.width;

	return 0;
}

string vspace::unique_name(string prefix)
{
	int id = 0;
	while (global.find(prefix + to_string(id)) != global.end() || label.find(prefix + to_string(id)) != label.end())
		id++;

	return prefix + to_string(id);
}

map<string, string> vspace::instantiate(string parent, vspace* s)
{
	if (s == NULL)
		return map<string, string>();

	map<string, string> rename;

	variable v;
	map<string, variable>::iterator i;
	for (i = s->global.begin(); i != s->global.end(); i++)
	{
		if (!i->second.io)
		{
			v = i->second;
			rename.insert(pair<string, string>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = global.size();
			global.insert(pair<string, variable>(v.name, v));
		}
	}

	for (i = s->label.begin(); i != s->label.end(); i++)
	{
		if (!i->second.io)
		{
			v = i->second;
			rename.insert(pair<string, string>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = label.size();
			label.insert(pair<string, variable>(v.name, v));
		}
	}

	return rename;
}

void vspace::insert(variable v)
{
	if (global.find(v.name) != global.end() || label.find(v.name) != label.end())
	{
		cout << "Error: Variable " + v.name + " redefined." << endl;
		return;
	}

	if (v.type == "int")
	{
		v.uid = global.size();
		global.insert(pair<string, variable>(v.name, v));
	}
	else
	{
		v.uid = label.size();
		label.insert(pair<string, variable>(v.name, v));
	}
}

void vspace::clear()
{
	global.clear();
	label.clear();
}

vspace &vspace::operator=(vspace s)
{
	clear();
	global.insert(s.global.begin(), s.global.end());
	label.insert(s.label.begin(), s.label.end());
	return *this;
}

ostream &operator<<(ostream &os, vspace s)
{
	map<string, variable>::iterator i;
	for (i = s.global.begin(); i != s.global.end(); i++)
		os << i->second.type << " " << i->first << ": " << i->second.uid << "\n";

	for (i = s.label.begin(); i != s.label.end(); i++)
		os << i->second.type << " " << i->first << ": " << i->second.uid << "\n";

	return os;
}
