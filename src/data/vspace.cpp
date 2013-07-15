/*
 * vspace.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: nbingham
 */

#include "vspace.h"
#include "../type/channel.h"

int vspace::size()
{
	return (int)global.size();
}

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

	if (v->type == "node" && s == name.npos)
		return v->type + "<" + to_string(v->width) + ">";
	else if (v->type == "node")
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

vector<string> vspace::get_driven()
{
	vector<string> result;
	map<string, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.driven)
			result.push_back(i->first);
	return result;
}

vector<string> vspace::get_names()
{
	vector<string> ret(global.size());
	map<string, variable>::iterator gi;

	for (gi = global.begin(); gi != global.end(); gi++)
		ret[gi->second.uid] = gi->first;

	return ret;
}

/* x_channel()
 *
 * av is a list of indices to one bit variables located in the globals list
 * node is an index to the place in which we want to existentially quantify these variables
 */
vector<int> vspace::x_channel(vector<int> av)
{
	map<string, variable>::iterator vi;
	string name, vkind;
	size_t n;
	variable *v;
	variable *g;
	keyword *k;
	channel *c;
	int id;
	vector<int> result;
	for (int i = 0; i < (int)av.size(); i++)
	{
		name = get_name(av[i]);

		for (n = name.find_last_of("."); n != name.npos; n = name.substr(0, n).find_last_of("."))
		{
			v = find(name.substr(0, n));
			if (v != NULL)
			{
				k = find_type(v->type);

				if (k != NULL && k->kind() == "channel")
				{
					c = (channel*)k;

					for (vi = c->vars.global.begin(); vi != c->vars.global.end(); vi++)
					{
						g = find(v->name + "." + vi->second.name);
						if (g == NULL)
							cout << "Error: Could not find " << v->name + "." + vi->second.name << endl;
						else if (!g->driven && std::find(av.begin(), av.end(), id) == av.end())
							result.push_back(g->uid);
					}
				}
			}
		}
	}

	unique(&result);

	return result;
}

void vspace::x_channel(vector<int> av, map<int, int> *result)
{
	vector<int> temp = x_channel(av);
	map<int, int>::iterator ri;
	int i;
	for (i = 0; i < (int)temp.size(); i++)
	{
		ri = result->find(temp[i]);
		if (ri == result->end())
			result->insert(pair<int, int>(temp[i], 0));
		else
			ri->second = 0;
	}
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

map<string, string> vspace::instantiate(string parent, bool parent_arg, vspace* s, bool arg)
{
	if (s == NULL)
		return map<string, string>();

	map<string, string> rename;

	variable v;
	map<string, variable>::iterator i;
	for (i = s->global.begin(); i != s->global.end(); i++)
	{
		if (i->second.arg == arg)
		{
			v = i->second;
			rename.insert(pair<string, string>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = global.size();
			v.arg = parent_arg;
			global.insert(pair<string, variable>(v.name, v));
		}
	}

	for (i = s->label.begin(); i != s->label.end(); i++)
	{
		if (i->second.arg == arg)
		{
			v = i->second;
			rename.insert(pair<string, string>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = label.size();
			v.arg = parent_arg;
			label.insert(pair<string, variable>(v.name, v));
		}
	}

	return rename;
}

map<string, string> vspace::call(string parent, bool parent_arg, vspace* s)
{
	if (s == NULL)
		return map<string, string>();

	map<string, string> rename;

	variable v;
	map<string, variable>::iterator i;
	for (i = s->global.begin(); i != s->global.end(); i++)
	{
		if (i->second.name.find("call") != string::npos)
		{
			v = i->second;
			rename.insert(pair<string, string>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = global.size();
			v.arg = parent_arg;
			global.insert(pair<string, variable>(v.name, v));
		}
	}

	for (i = s->label.begin(); i != s->label.end(); i++)
	{
		if (i->second.name.find("call") != string::npos)
		{
			v = i->second;
			rename.insert(pair<string, string>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = label.size();
			v.arg = parent_arg;
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

	if (v.type == "node" && v.width > 1)
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
	else if (v.type == "node" && v.width == 1)
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

void vspace::print(string t)
{
	size_t i;
	variable *v;
	for (i = 0; i < global.size(); i++)
	{
		v = find((int)i);
		cout << t << v->type << " " << v->name << " UID:" << v->uid << " Arg:" << v->arg << "\n";
	}

	map<string, variable>::iterator vi;
	for (vi = label.begin(); vi != label.end(); vi++)
		cout << t << vi->second.type << " " << vi->first << "\n";
}

ostream &operator<<(ostream &os, vspace s)
{
	size_t i;
	variable *v;
	for (i = 0; i < s.global.size(); i++)
	{
		v = s.find((int)i);
		os << v->type << " " << v->name << " UID:" << v->uid << " Arg:" << v->arg << "\n";
	}

	map<string, variable>::iterator vi;
	for (vi = s.label.begin(); vi != s.label.end(); vi++)
		os << vi->second.type << " " << vi->first << "\n";

	return os;
}
