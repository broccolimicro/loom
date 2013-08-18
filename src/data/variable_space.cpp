/*
 * variable_space.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: nbingham
 */

#include "variable_space.h"
#include "../type/channel.h"

variable_space::variable_space()
{
	enforcements = 1;
	reset = 1;
}

variable_space::~variable_space()
{
}

int variable_space::size()
{
	return (int)global.size();
}

variable *variable_space::find(int uid)
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

variable *variable_space::find(string name)
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

keyword	 *variable_space::find_type(string name)
{
	type_space::iterator i = types->find(name);
	if (i == types->end())
		return NULL;

	return i->second;
}

string variable_space::get_name(int uid)
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



string variable_space::get_type(int uid)
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

string variable_space::get_type(string name)
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

string variable_space::get_kind(string name)
{
	string type = get_type(name);
	type_space::iterator i;
	i = types->find(type);
	if (i == types->end())
		return "";

	return i->second->kind();
}

string variable_space::get_info(string name)
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

int variable_space::get_uid(string name)
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

int variable_space::get_width(string name)
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

vector<string> variable_space::get_driven()
{
	vector<string> result;
	map<string, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.driven)
			result.push_back(i->first);
	return result;
}

vector<string> variable_space::get_names()
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
vector<int> variable_space::x_channel(vector<int> av)
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

void variable_space::x_channel(vector<int> av, map<int, logic> *result)
{
	vector<int> temp = x_channel(av);
	map<int, logic>::iterator ri;
	int i;
	for (i = 0; i < (int)temp.size(); i++)
	{
		ri = result->find(temp[i]);
		if (ri == result->end())
			result->insert(pair<int, logic>(temp[i], logic(0)));
		else
			ri->second = 0;
	}
}

string variable_space::unique_name(string prefix)
{
	int id = 0;
	while (global.find(prefix + to_string(id)) != global.end() || label.find(prefix + to_string(id)) != label.end())
		id++;

	return prefix + to_string(id);
}

bool variable_space::vdef(string s)
{
	type_space::iterator i;
	for (i = types->begin(); i != types->end(); i++)
		if (s.find(i->first) != s.npos)
			return true;

	return false;
}

map<string, string> variable_space::instantiate(string parent, bool parent_arg, variable_space* s, bool arg)
{
	if (s == NULL)
		return map<string, string>();

	map<string, string> rename;
	vector<int> id_change(s->global.size(), 0);

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
			v.reset = i->second.reset;
			global.insert(pair<string, variable>(v.name, v));
			id_change[i->second.uid] = get_uid(v.name);
			if (v.reset.size() > 0)
				reset &= logic(id_change[i->second.uid], v.reset[0]);
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

	for (int j = 0; j < (int)s->requirements.size(); j++)
		requirements.push_back(s->requirements[j].refactor(id_change));

	enforcements = enforcements >> s->enforcements.refactor(id_change);

	return rename;
}

map<string, string> variable_space::call(string parent, bool parent_arg, variable_space* s)
{
	if (s == NULL)
		return map<string, string>();

	map<string, string> rename;
	vector<int> id_change(s->global.size(), 0);

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
			v.reset = i->second.reset;
			global.insert(pair<string, variable>(v.name, v));
			id_change[i->second.uid] = get_uid(v.name);
			if (v.reset.size() > 0)
				reset &= logic(id_change[i->second.uid], v.reset[0]);
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

	for (int j = 0; j < (int)s->requirements.size(); j++)
		requirements.push_back(s->requirements[j].refactor(id_change));

	enforcements = enforcements >> s->enforcements.refactor(id_change);

	return rename;
}

int variable_space::insert(variable v)
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
			if (i < (int)v.reset.size())
				reset &= logic(v.uid, v.reset[i]);
			global.insert(pair<string, variable>(v.name, v));
		}
	}
	else if (v.type == "node" && v.width == 1)
	{
		v.uid = global.size();
		if (v.reset.size() > 0)
		reset &= logic(v.uid, v.reset[0]);
		global.insert(pair<string, variable>(v.name, v));
	}
	else
	{
		v.uid = label.size();
		label.insert(pair<string, variable>(v.name, v));
	}

	return v.uid;
}

void variable_space::clear()
{
	global.clear();
	label.clear();
	enforcements = 1;
	requirements.clear();
	reset = 1;
}

variable_space &variable_space::operator=(variable_space s)
{
	global = s.global;
	label = s.label;
	types = s.types;
	requirements = s.requirements;
	enforcements = s.enforcements;
	reset = s.reset;
	return *this;
}

void variable_space::print(string t, ostream *fout)
{
	size_t i;
	variable *v;
	for (i = 0; i < global.size(); i++)
	{
		v = find((int)i);
		(*fout) << t << v->type << " " << v->name << " UID:" << v->uid << " Arg:" << v->arg << "\n";
	}

	map<string, variable>::iterator vi;
	for (vi = label.begin(); vi != label.end(); vi++)
		(*fout) << t << vi->second.type << " " << vi->first << "\n";
}

ostream &operator<<(ostream &os, variable_space s)
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
