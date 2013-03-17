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
	if (i != global.end())
		return &(i->second);
	i = label.find(name);
	if (i != label.end())
		return &(i->second);

	return NULL;
}

keyword	 *vspace::find_type(string name)
{
	map<string, keyword*>::iterator i = types->find(name);
	if (i == types->end())
		return NULL;

	return i->second;
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

string vspace::get_kind(string name)
{
	string type = get_type(name);
	map<string, keyword*>::iterator i;
	i = types->find(type);
	if (i == types->end())
		return "";

	return i->second->kind();
}

string vspace::get_info(string name)
{
	size_t s = name.npos;
	size_t d = name.npos;
	string min, max;
	int l, h;
	if (name.find("..") != name.npos)
	{
		s = name.find_first_of("[]");
		d = name.find("..");
	}

	if (s != name.npos)
	{
		if (d != name.npos)
		{
			min = name.substr(s+1, d - s - 1);
			max = name.substr(d+2, name.length() - d - 3);
			l = atoi(min.c_str());
			h = atoi(max.c_str())+1;
		}
		else
		{
			min = name.substr(s+1, name.length() - s - 2);
			max = min;

			l = atoi(min.c_str());
			h = l+1;
		}
		name = name.substr(0, s);
	}

	variable *v = find(name);
	if (v == NULL)
		return "";

	if (v->type == "int" && s == name.npos)
		return v->type + "<" + to_string(v->width) + ">";
	else if (v->type == "int")
		return v->type + "<" + to_string(h-l) + ">";

	// TODO Possible bug?

	return v->type;
}

int vspace::get_uid(string name)
{
	map<string, variable>::iterator i;
	i = global.find(name);
	if (i != global.end())
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

bool vspace::vdef(string s)
{
	map<string, keyword*>::iterator i;
	for (i = types->begin(); i != types->end(); i++)
		if (s.find(i->first) != s.npos)
			return true;

	return false;
}

map<string, string> vspace::instantiate(string parent, bool parent_io, vspace* s, bool io)
{
	if (s == NULL)
		return map<string, string>();

	map<string, string> rename;

	variable v;
	map<string, variable>::iterator i;
	for (i = s->global.begin(); i != s->global.end(); i++)
	{
		if (i->second.io == io)
		{
			v = i->second;
			rename.insert(pair<string, string>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = global.size();
			v.io = parent_io;
			global.insert(pair<string, variable>(v.name, v));
		}
	}

	for (i = s->label.begin(); i != s->label.end(); i++)
	{
		if (i->second.io == io)
		{
			v = i->second;
			rename.insert(pair<string, string>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = label.size();
			v.io = parent_io;
			label.insert(pair<string, variable>(v.name, v));
		}
	}

	return rename;
}

int vspace::insert(variable v)
{
	if (global.find(v.name) != global.end() || label.find(v.name) != label.end())
	{
		cout << "Error: Variable " + v.name + " redefined." << endl;
		return -1;
	}

	string n;
	int w;

	if (v.type == "int" && v.width > 1)
	{
		v.uid = label.size();
		label.insert(pair<string, variable>(v.name, v));
		n = v.name;
		w = v.width;
		for (int i = 0; i < w; i++)
		{
			v.name = n + "[" + to_string(i) + "]";
			v.width = 1;
			v.uid = global.size();
			global.insert(pair<string, variable>(v.name, v));
		}
	}
	else if (v.type == "int" && v.width == 1)
	{
		v.uid = global.size();
		global.insert(pair<string, variable>(v.name, v));
	}
	else
	{
		v.uid = label.size();
		label.insert(pair<string, variable>(v.name, v));
	}

	return v.uid;
}

void vspace::clear()
{
	global.clear();
	label.clear();
}

vspace &vspace::operator=(vspace s)
{
	global.clear();
	label.clear();
	global.insert(s.global.begin(), s.global.end());
	label.insert(s.label.begin(), s.label.end());
	types = s.types;
	return *this;
}

ostream &operator<<(ostream &os, vspace s)
{
	size_t i;
	variable *v;
	for (i = 0; i < s.global.size(); i++)
	{
		v = s.find((int)i);
		os << v->type << " " << v->name << ": " << v->uid << "," << v->io << "\n";
	}

	map<string, variable>::iterator vi;
	for (vi = s.label.begin(); vi != s.label.end(); vi++)
		os << vi->second.type << " " << vi->first << "\n";

	return os;
}
