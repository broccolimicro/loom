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
	smap<sstring, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
			return &(i->second);
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
			return &(i->second);
	return NULL;
}

variable *variable_space::find(sstring name)
{
	smap<sstring, variable>::iterator i;
	i = global.find(name);
	if (i != global.end())
		return &(i->second);
	i = label.find(name);
	if (i != label.end())
		return &(i->second);

	name = "this." + name;
	i = global.find(name);
	if (i != global.end())
		return &(i->second);
	i = label.find(name);
	if (i != label.end())
		return &(i->second);

	return NULL;
}

keyword	 *variable_space::find_type(sstring name)
{
	type_space::iterator i = types->find(name);
	if (i == types->end())
		return NULL;

	return i->second;
}

sstring variable_space::get_name(int uid)
{
	smap<sstring, variable>::iterator i;
	int idx;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
		{
			if ((idx = i->second.name.find("this.")) != i->second.name.npos)
				return i->second.name.substr(0, idx) + i->second.name.substr(idx+5);
			else
				return i->second.name;
		}
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
		{
			if ((idx = i->second.name.find("this.")) != i->second.name.npos)
				return i->second.name.substr(0, idx) + i->second.name.substr(idx+5);
			else
				return i->second.name;
		}
	return "";
}

sstring variable_space::get_type(int uid)
{
	smap<sstring, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
			return i->second.type;
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
			return i->second.type;
	return "";
}

sstring variable_space::get_type(sstring name)
{
	smap<sstring, variable>::iterator i;
	i = global.find(name);
	if (i != global.end())
		return i->second.type;
	i = label.find(name);
	if (i != label.end())
		return i->second.type;

	name = "this." + name;
	i = global.find(name);
	if (i != global.end())
		return i->second.type;
	i = label.find(name);
	if (i != label.end())
		return i->second.type;

	return "Error";
}

sstring variable_space::get_kind(sstring name)
{
	sstring type = get_type(name);
	type_space::iterator i;
	i = types->find(type);
	if (i == types->end())
		return "";

	return i->second->kind();
}

sstring variable_space::get_info(sstring name)
{
	int s = name.npos;
	int d = name.npos;
	sstring min, max;
	int l = 0, h = 0;
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
		return v->type + "<" + sstring(v->width) + ">";
	else if (v->type == "node")
		return v->type + "<" + sstring(h-l) + ">";

	// TODO Possible bug?

	return v->type;
}

int variable_space::get_uid(sstring name)
{
	smap<sstring, variable>::iterator i;
	i = global.find(name);
	if (i != global.end())
		return i->second.uid;
	i = label.find(name);
	if (i != label.end())
		return i->second.uid;

	name = "this." + name;
	i = global.find(name);
	if (i != global.end())
		return i->second.uid;
	i = label.find(name);
	if (i != label.end())
		return i->second.uid;

	return -1;
}

int variable_space::get_width(sstring name)
{
	smap<sstring, variable>::iterator i;
	i = global.find(name);
	if (i != global.end())
		return i->second.width;
	i = label.find(name);
	if (i != label.end())
		return i->second.width;

	name = "this." + name;
	i = global.find(name);
	if (i != global.end())
		return i->second.width;
	i = label.find(name);
	if (i != label.end())
		return i->second.width;
	return 0;
}

svector<sstring> variable_space::get_driven()
{
	svector<sstring> result;
	smap<sstring, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.driven)
			result.push_back(i->first);
	return result;
}

svector<sstring> variable_space::get_names()
{
	svector<sstring> ret(global.size());
	smap<sstring, variable>::iterator gi;
	int idx;

	for (gi = global.begin(); gi != global.end(); gi++)
	{
		ret[gi->second.uid] = gi->first;
		if ((idx = ret[gi->second.uid].find("this.")) != ret[gi->second.uid].npos)
			ret[gi->second.uid] = ret[gi->second.uid].substr(0, idx) + ret[gi->second.uid].substr(idx+5);
	}

	return ret;
}

bool variable_space::part_of_channel(int uid)
{
	sstring name, vkind;
	int n;
	variable *v;
	keyword *k;

	smap<sstring, variable>::iterator j;
	for (j = global.begin(); j != global.end() && name == ""; j++)
		if (j->second.uid == uid)
			name =  j->second.name;
	for (j = label.begin(); j != label.end() && name == ""; j++)
		if (j->second.uid == uid)
			name = j->second.name;

	for (n = name.find_last_of("."); n != name.npos; n = name.substr(0, n).find_last_of("."))
	{
		v = find(name.substr(0, n));
		if (v != NULL)
		{
			k = find_type(v->type);

			if (k != NULL && k->kind() == "channel")
				return true;
		}
	}

	return false;
}

/* x_channel()
 *
 * av is a list of indices to one bit variables located in the globals list
 * node is an index to the place in which we want to existentially quantify these variables
 */
svector<int> variable_space::x_channel(svector<int> av)
{
	smap<sstring, variable>::iterator vi;
	sstring name, vkind;
	int n;
	variable *v;
	variable *g;
	keyword *k;
	channel *c;
	svector<int> result;
	for (int i = 0; i < (int)av.size(); i++)
	{
		name = "";
		smap<sstring, variable>::iterator j;
		for (j = global.begin(); j != global.end() && name == ""; j++)
			if (j->second.uid == av[i])
				name =  j->second.name;
		for (j = label.begin(); j != label.end() && name == ""; j++)
			if (j->second.uid == av[i])
				name = j->second.name;

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
							cerr << "Error: Could not find " << v->name + "." + vi->second.name << endl;
						else if (!g->driven && std::find(av.begin(), av.end(), g->uid) == av.end())
							result.push_back(g->uid);
					}
				}
			}
		}
	}

	return result.unique();
}

void variable_space::x_channel(svector<int> av, smap<int, logic> *result)
{
	svector<int> temp = x_channel(av);
	smap<int, logic>::iterator ri;
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

sstring variable_space::unique_name(sstring prefix)
{
	int id = 0;
	while (find(prefix + sstring(id)) != NULL)
		id++;

	return prefix + sstring(id);
}

bool variable_space::vdef(sstring s)
{
	type_space::iterator i;
	for (i = types->begin(); i != types->end(); i++)
		if (s.find(i->first) != s.npos)
			return true;
	return false;
}

smap<sstring, sstring> variable_space::instantiate(sstring parent, bool parent_arg, variable_space* s, bool arg)
{
	if (s == NULL)
		return smap<sstring, sstring>();

	smap<sstring, sstring> rename;
	svector<int> id_change(s->global.size(), 0);
	int idx;

	variable v;
	smap<sstring, variable>::iterator i;
	for (i = s->global.begin(); i != s->global.end(); i++)
	{
		if (i->second.arg == arg)
		{
			v = i->second;
			if ((idx = v.name.find("this.")) != v.name.npos)
				v.name = v.name.substr(0, idx) + v.name.substr(idx+5);
			rename.insert(pair<sstring, sstring>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = global.size();
			v.arg = parent_arg;
			v.reset = i->second.reset;
			v.driven = false;
			global.insert(pair<sstring, variable>(v.name, v));
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
			rename.insert(pair<sstring, sstring>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			if ((idx = v.name.find(".this")) != v.name.npos)
				v.name = v.name.substr(0, idx) + v.name.substr(idx+5);
			v.uid = label.size();
			v.arg = parent_arg;
			label.insert(pair<sstring, variable>(v.name, v));
		}
	}

	for (int j = 0; j < (int)s->requirements.size(); j++)
		requirements.push_back(s->requirements[j].refactor(id_change));

	enforcements = enforcements >> s->enforcements.refactor(id_change);

	return rename;
}

smap<sstring, sstring> variable_space::call(sstring parent, bool parent_arg, variable_space* s)
{
	if (s == NULL)
		return smap<sstring, sstring>();

	smap<sstring, sstring> rename;
	svector<int> id_change(s->global.size(), 0);
	int idx;

	variable v;
	smap<sstring, variable>::iterator i;
	for (i = s->global.begin(); i != s->global.end(); i++)
	{
		if (i->second.name.find("call") != sstring::npos)
		{
			v = i->second;
			if ((idx = v.name.find("this.")) != v.name.npos)
				v.name = v.name.substr(0, idx) + v.name.substr(idx+5);
			rename.insert(pair<sstring, sstring>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = global.size();
			v.arg = parent_arg;
			v.reset = i->second.reset;
			v.driven = false;
			global.insert(pair<sstring, variable>(v.name, v));
			id_change[i->second.uid] = get_uid(v.name);
			if (v.reset.size() > 0)
				reset &= logic(id_change[i->second.uid], v.reset[0]);
		}
	}

	for (i = s->label.begin(); i != s->label.end(); i++)
	{
		if (i->second.name.find("call") != sstring::npos)
		{
			v = i->second;
			if ((idx = v.name.find(".this")) != v.name.npos)
				v.name = v.name.substr(0, idx) + v.name.substr(idx+5);
			rename.insert(pair<sstring, sstring>(v.name, parent + "." + v.name));
			v.name = parent + "." + v.name;
			v.uid = label.size();
			v.arg = parent_arg;
			label.insert(pair<sstring, variable>(v.name, v));
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
		cerr << "Error: Variable " + v.name + " redefined." << endl;
		return -1;
	}

	sstring n;
	int w;

	if (v.type == "node" && v.width > 1)
	{
		v.uid = label.size();
		label.insert(pair<sstring, variable>(v.name, v));
		n = v.name;
		w = v.width;
		for (int i = 0; i < w; i++)
		{
			v.name = n + "[" + sstring(i) + "]";
			v.width = 1;
			v.uid = global.size();
			if (i < (int)v.reset.size())
				reset &= logic(v.uid, v.reset[i]);
			global.insert(pair<sstring, variable>(v.name, v));
		}
	}
	else if (v.type == "node" && v.width == 1)
	{
		v.uid = global.size();
		if (v.reset.size() > 0)
		reset &= logic(v.uid, v.reset[0]);
		global.insert(pair<sstring, variable>(v.name, v));
	}
	else
	{
		v.uid = label.size();
		label.insert(pair<sstring, variable>(v.name, v));
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

logic variable_space::increment_pcs(logic t, bool active)
{
	smap<sstring, variable>::iterator i;
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.type.find_first_of("?!") != i->second.type.npos)
			cout << "LOOK " << i->second.type << " " << i->second.name << endl;

	return logic();
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

void variable_space::print(sstring t, ostream *fout)
{
	int i;
	variable *v;
	for (i = 0; i < global.size(); i++)
	{
		v = find((int)i);
		(*fout) << t << v->type << " " << v->name << " UID:" << v->uid << " Arg:" << v->arg << endl;
	}

	smap<sstring, variable>::iterator vi;
	for (vi = label.begin(); vi != label.end(); vi++)
		(*fout) << t << vi->second.type << " " << vi->first << endl;
}

ostream &operator<<(ostream &os, variable_space s)
{
	int i;
	variable *v;
	for (i = 0; i < s.global.size(); i++)
	{
		v = s.find((int)i);
		os << v->type << " " << v->name << " UID:" << v->uid << " Arg:" << v->arg << " Driven: " << v->driven << endl;
	}

	smap<sstring, variable>::iterator vi;
	for (vi = s.label.begin(); vi != s.label.end(); vi++)
		os << vi->second.type << " " << vi->first << endl;

	return os;
}
