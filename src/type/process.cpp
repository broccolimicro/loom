/*
 * process.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../syntax.h"
#include "../data.h"

#include "process.h"
#include "record.h"
#include "channel.h"

process::process()
{
	name = "";
	_kind = "process";
	chp = "";
	is_inline = false;
	def.parent = NULL;
	flags = NULL;
	net.vars = &vars;
	net.prs = &prs;
	prs.vars = &vars;
}

process::process(string raw, type_space *types, flag_space *flags)
{
	_kind = "process";
	vars.types = types;
	this->flags = flags;
	is_inline = false;
	def.parent = NULL;
	net.vars = &vars;
	net.prs = &prs;
	prs.vars = &vars;
	net.flags = flags;

	parse(raw);

	types->insert(pair<string, process*>(name, this));
}

process::~process()
{
	name = "";
	_kind = "process";
	chp = "";
	def.parent = NULL;

	vars.clear();
}

process &process::operator=(process p)
{
	def = p.def;
	prs = p.prs;
	vars = p.vars;
	flags = p.flags;
	return *this;
}

void process::parse(string raw)
{
	if (raw.compare(0, 7, "inline ") == 0)
	{
		is_inline = true;
		raw = raw.substr(7);
	}
	else
		is_inline = false;

	chp = raw;

	size_t name_start = chp.find_first_of(" ")+1;
	size_t name_end = chp.find_first_of("(");
	size_t input_start = chp.find_first_of("(")+1;
	size_t input_end = chp.find_first_of(")");
	size_t sequential_start = chp.find_first_of("{")+1;
	size_t sequential_end = chp.length()-1;
	string io_sequential;
	string::iterator i, j;

	map<string, variable> temp;
	map<string, variable>::iterator vi, vj;
	type_space::iterator ti;

	name = chp.substr(name_start, name_end - name_start);
	io_sequential = chp.substr(input_start, input_end - input_start);

	if (flags->log_base_hse())
	{
		(*flags->log_file) << "Process:\t" << chp << endl;
		(*flags->log_file) << "\tName:\t" << name << endl;
		(*flags->log_file) << "\tArgs:\t" << io_sequential << endl;
	}

	vars.insert(variable("reset", "node", 1, true, flags));

	for (i = io_sequential.begin(), j = io_sequential.begin(); i != io_sequential.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_sequential.end())
		{
			expand_instantiation(NULL, io_sequential.substr(j-io_sequential.begin(), i+1 - j), &vars, &args, flags, false);
			j = i+2;
		}
	}

	string sequential = chp.substr(sequential_start, sequential_end - sequential_start);

	if (!is_inline)
	{
		expand_instantiation(NULL, "__sync call", &vars, &args, flags, false);
		sequential = "[call.r];call.a+;(" + sequential + ");[~call.r];call.a-";
	}

	def.init(sequential, &vars, flags);

	if (flags->log_base_hse())
	{
		(*flags->log_file) << "\tVariables:" << endl;
		vars.print("\t\t", flags->log_file);
		(*flags->log_file) << "\tHSE:" << endl;
		def.print_hse("\t\t", flags->log_file);
		(*flags->log_file) << endl << endl;
	}
}

void process::simulate()
{
	def.simulate();
}

void process::rewrite()
{
	def.rewrite();

	if (flags->log_merged_hse())
	{
		def.print_hse("", flags->log_file);
		(*flags->log_file) << endl;
	}
}

// TODO Projection algorithm - when do we need to do projection? when shouldn't we do projection?
void process::project()
{
	if (flags->log_projected_hse())
	{
		def.print_hse("", flags->log_file);
		(*flags->log_file) << endl;
	}
}

// TODO Process decomposition - How big should we make processes?
void process::decompose()
{
	if (flags->log_decomposed_hse())
	{
		def.print_hse("", flags->log_file);
		(*flags->log_file) << endl;
	}
}

// TODO Handshaking Reshuffling
void process::reshuffle()
{
	// Build a dependency tree
	// Examine synchronization actions as a dependency relation
	// Reorder instructions to mass information before active assignments
	if (flags->log_reshuffled_hse())
	{
		def.print_hse("", flags->log_file);
		(*flags->log_file) << endl;
	}
}

// TODO There is a problem with the interaction of scribe variables with bubbleless reshuffling because scribe variables insert bubbles
void process::generate_states()
{
	(*flags->log_file) << "Process" << endl;

	net.vars = &vars;
	net.prs = &prs;
	prs.vars = &vars;
	net.flags = flags;

	vector<int> start, end;
	start.push_back(net.insert_place(start, map<int, int>(), map<int, int>(), NULL));
	net.M0.push_back(start[0]);
	end = def.generate_states(&net, &prs, start,  map<int, int>(), map<int, int>());
	if (is_inline)
		net.connect(end, net.M0);

	for (map<string, variable>::iterator i = vars.global.begin(); i != vars.global.end(); i++)
		if (!i->second.driven)
			vars.reset = vars.reset.hide(i->second.uid);

	cout << "EXCL" << endl;
	for (int i = 0; i < (int)prs.excl.size(); i++)
	{
		for (int j = 0; j < (int)prs.excl[i].first.size(); j++)
			cout << vars.get_name(prs.excl[i].first[j]) << " ";
		cout << prs.excl[i].second << endl;
	}

	do
	{
		net.gen_mutables();
		net.print_mutables();
		net.update();
		print_dot(flags->log_file);
	} while(net.trim());

	net.check_assertions();

	(*flags->log_file) << "Branches: " << name << endl;
	net.print_branch_ids(flags->log_file);
}

bool process::insert_state_vars()
{
	map<int, list<vector<int> > >::iterator i, l;
	list<vector<int> >::iterator lj;
	map<int, int>::iterator m, n;
	list<path>::iterator li;
	vector<int> arcs, uptrans, downtrans, ia, oa, jstart, istart;
	vector<pair<vector<int>, vector<int> > > ip;
	int ium, idm, vid, j, k;
	int ind_max;
	string vname;

	net.print_dot(flags->log_file, name);
	net.gen_tails();
	net.gen_conflicts();
	net.gen_conditional_places();

	(*flags->log_file) << "Conflicts: " << name << endl;
	for (i = net.conflicts.begin(); i != net.conflicts.end(); i++)
	{
		(*flags->log_file) << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			(*flags->log_file) << "{";
			for (j = 0; j < (int)lj->size(); j++)
				(*flags->log_file) << (*lj)[j] << " ";
			(*flags->log_file) << "} ";
		}
		(*flags->log_file) << endl;
	}

	ind_max = 0;
	(*flags->log_file) << "Indistinguishables: " << name << endl;
	for (i = net.indistinguishable.begin(); i != net.indistinguishable.end(); i++)
	{
		(*flags->log_file) << i->first << ": ";
		if ((int)i->second.size() > ind_max)
			ind_max = (int)i->second.size();
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			(*flags->log_file) << "{";
			for (j = 0; j < (int)lj->size(); j++)
				(*flags->log_file) << (*lj)[j] << " ";
			(*flags->log_file) << "} ";
		}
		(*flags->log_file) << endl;
	}

	if (net.conflicts.size() == 0)
		return false;

	path_space up_paths(net.arcs.size()), up_temp(net.arcs.size()), up_inv(net.arcs.size()), down_paths(net.arcs.size()), down_temp(net.arcs.size()), down_inv(net.arcs.size());
	path up_mask(net.arcs.size()), down_mask(net.arcs.size());
	(*flags->log_file) << "PROCESS " << name << endl;
	for (i = net.conflicts.begin(); i != net.conflicts.end(); i++)
	{
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			down_paths.clear();
			up_paths.clear();
			istart = net.start_path(i->first, *lj);
			jstart = net.start_path(*lj, vector<int>(1, i->first));

			//cout << "Up" << endl;
			net.get_paths(istart, *lj, &up_paths);
			//cout << "Down" << endl;
			net.get_paths(jstart, vector<int>(1, i->first), &down_paths);

			up_inv = up_paths.inverse();
			down_inv = down_paths.inverse();

			up_mask = up_inv.get_mask();
			down_mask = down_inv.get_mask();

			up_paths.apply_mask(down_mask);
			down_paths.apply_mask(up_mask);

			net.zero_outs(&up_paths, i->first);
			net.zero_ins(&down_paths, i->first);
			net.zero_ins(&up_paths, *lj);
			net.zero_outs(&down_paths, *lj);

			for (j = 0; j < (int)net.arcs.size(); j++)
				if (net.is_place(net.arcs[j].first) && net.output_nodes(net.arcs[j].first).size() > 1)
				{
					up_paths.zero(j);
					down_paths.zero(j);
				}

			(*flags->log_file) << "Up: {";
			for (j = 0; j < (int)up_paths.total.from.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << up_paths.total.from[j];
			}
			(*flags->log_file) << "} -> {";
			for (j = 0; j < (int)up_paths.total.to.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << up_paths.total.to[j];
			}
			(*flags->log_file) << "}" << endl;
			uptrans.clear();
			for (l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
			{
				for (j = 0; j < (int)net.arcs.size(); j++)
					if (up_paths.total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
						up_paths.total.nodes[j] += ind_max - (int)l->second.size();
			}
			(*flags->log_file) << up_paths.total << endl;
			while (up_paths.size() > 0 && (ium = net.closest_input(up_paths.total.maxes(), up_paths.total.to, path(net.arcs.size())).second) != -1)
			{
				uptrans.push_back(ium);
				up_paths = up_paths.avoidance(ium);
				for (l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
				{
					for (j = 0; j < (int)net.arcs.size(); j++)
						if (up_paths.total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
							up_paths.total.nodes[j] += ind_max - (int)l->second.size();
				}
				(*flags->log_file) << up_paths.total << endl;
			}

			(*flags->log_file) << "Down: {";
			for (j = 0; j < (int)down_paths.total.from.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << down_paths.total.from[j];
			}
			(*flags->log_file) << "} -> {";
			for (j = 0; j < (int)down_paths.total.to.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << down_paths.total.to[j];
			}
			(*flags->log_file) << "}" << endl;
			downtrans.clear();
			for (l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
			{
				for (j = 0; j < (int)net.arcs.size(); j++)
					if (down_paths.total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
						down_paths.total.nodes[j] += ind_max - (int)l->second.size();

			}
			(*flags->log_file) << down_paths.total << endl;
			while (down_paths.size() > 0 && (idm = net.closest_input(down_paths.total.maxes(), down_paths.total.to, path(net.arcs.size())).second) != -1)
			{
				downtrans.push_back(idm);
				down_paths = down_paths.avoidance(idm);
				for (l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
				{
					for (j = 0; j < (int)net.arcs.size(); j++)
						if (down_paths.total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
							down_paths.total.nodes[j] += ind_max - (int)l->second.size();

				}
				(*flags->log_file) << down_paths.total << endl;
			}
			(*flags->log_file) << endl;

			unique(&uptrans, &downtrans);

			if (uptrans.size() == 0 || downtrans.size() == 0)
			{
				cout << "Error: No solution for the conflict set: " << i->first << " -> {";
				for (j = 0; j < (int)lj->size(); j++)
					cout << (*lj)[j] << " ";
				cout << "}." << endl;
				net.print_dot(&cout, name);
			}
			else if (downtrans < uptrans)
				ip.push_back(pair<vector<int>, vector<int> >(downtrans, uptrans));
			else
				ip.push_back(pair<vector<int>, vector<int> >(uptrans, downtrans));
		}
	}

	unique(&ip);
	logic um, dm;
	for (j = 0; j < (int)ip.size(); j++)
	{
		vname = vars.unique_name("_sv");
		vid   = vars.insert(variable(vname, "node", 1, false, flags));
		vars.find(vname)->driven = true;

		um = logic(vid, 1);
		dm = logic(vid, 0);

		for (k = 0; k < (int)ip[j].first.size(); k++)
			net.insert_sv_at(ip[j].first[k], um);
		for (k = 0; k < (int)ip[j].second.size(); k++)
			net.insert_sv_at(ip[j].second[k], dm);
	}

	net.trim_branch_ids();
	net.gen_mutables();
	net.update();

	return (ip.size() > 0);
}

bool process::insert_bubbleless_state_vars()
{
	map<int, list<vector<int> > >::iterator i, l;
	list<vector<int> >::iterator lj;
	map<int, int>::iterator m, n;
	vector<int> li;

	map<int, pair<int, int> >::iterator ci;
	vector<int> arcs, uptrans, downtrans, ia, oa, istart, jstart, iend, jend;
	vector<pair<vector<int>, vector<int> > > ip;
	int ium, idm, vid, j, k;
	int ind_max;
	string vname;

	net.print_dot(flags->log_file, name);
	net.gen_tails();
	net.gen_senses();
	net.gen_bubbleless_conflicts();
	net.gen_conditional_places();

	(*flags->log_file) << "Conflicts: " << name << endl;
	for (i = net.conflicts.begin(); i != net.conflicts.end(); i++)
	{
		(*flags->log_file) << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			(*flags->log_file) << "{";
			for (j = 0; j < (int)lj->size(); j++)
				(*flags->log_file) << (*lj)[j] << " ";
			(*flags->log_file) << "} ";
		}
		(*flags->log_file) << endl;
	}

	ind_max = 0;
	(*flags->log_file) << "Indistinguishables: " << name << endl;
	for (i = net.indistinguishable.begin(); i != net.indistinguishable.end(); i++)
	{
		(*flags->log_file) << i->first << ": ";
		if ((int)i->second.size() > ind_max)
			ind_max = (int)i->second.size();
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			(*flags->log_file) << "{";
			for (j = 0; j < (int)lj->size(); j++)
				(*flags->log_file) << (*lj)[j] << " ";
			(*flags->log_file) << "} ";
		}
		(*flags->log_file) << endl;
	}

	(*flags->log_file) << "Positive Conflicts: " << name << endl;
	for (i = net.positive_conflicts.begin(); i != net.positive_conflicts.end(); i++)
	{
		(*flags->log_file) << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			(*flags->log_file) << "{";
			for (j = 0; j < (int)lj->size(); j++)
				(*flags->log_file) << (*lj)[j] << " ";
			(*flags->log_file) << "} ";
		}
		(*flags->log_file) << endl;
	}

	(*flags->log_file) << "Negative Conflicts: " << name << endl;
	for (i = net.negative_conflicts.begin(); i != net.negative_conflicts.end(); i++)
	{
		(*flags->log_file) << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			(*flags->log_file) << "{";
			for (j = 0; j < (int)lj->size(); j++)
				(*flags->log_file) << (*lj)[j] << " ";
			(*flags->log_file) << "} ";
		}
		(*flags->log_file) << endl;
	}

	(*flags->log_file) << "Positive Indistinguishables: " << name << endl;
	for (i = net.positive_indistinguishable.begin(); i != net.positive_indistinguishable.end(); i++)
	{
		(*flags->log_file) << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			(*flags->log_file) << "{";
			for (j = 0; j < (int)lj->size(); j++)
				(*flags->log_file) << (*lj)[j] << " ";
			(*flags->log_file) << "} ";
		}
		(*flags->log_file) << endl;
	}

	(*flags->log_file) << "Negative Indistinguishables: " << name << endl;
	for (i = net.negative_indistinguishable.begin(); i != net.negative_indistinguishable.end(); i++)
	{
		(*flags->log_file) << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			(*flags->log_file) << "{";
			for (j = 0; j < (int)lj->size(); j++)
				(*flags->log_file) << (*lj)[j] << " ";
			(*flags->log_file) << "} ";
		}
		(*flags->log_file) << endl;
	}

	if (net.positive_conflicts.size() == 0 && net.negative_conflicts.size() == 0 && net.conflicts.size() == 0)
		return false;

	path_space up_paths(net.arcs.size()), up_temp(net.arcs.size()), up_inv(net.arcs.size()), down_paths(net.arcs.size()), down_temp(net.arcs.size()), down_inv(net.arcs.size());
	path up_mask(net.arcs.size()), down_mask(net.arcs.size());
	(*flags->log_file) << "PROCESS " << name << endl;

	for (i = net.conflicts.begin(); i != net.conflicts.end(); i++)
	{
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			down_paths.clear();
			up_paths.clear();
			istart = net.start_path(i->first, *lj);
			jstart = net.start_path(*lj, vector<int>(1, i->first));

			net.get_paths(istart, *lj, &up_paths);
			net.get_paths(jstart, vector<int>(1, i->first), &down_paths);

			up_inv = up_paths.inverse();
			down_inv = down_paths.inverse();

			up_mask = up_inv.get_mask();
			down_mask = down_inv.get_mask();

			up_paths.apply_mask(down_mask);
			down_paths.apply_mask(up_mask);

			net.zero_outs(&up_paths, i->first);
			net.zero_ins(&down_paths, i->first);
			net.zero_ins(&up_paths, *lj);
			net.zero_outs(&down_paths, *lj);

			for (j = 0; j < (int)net.arcs.size(); j++)
				if (net.is_place(net.arcs[j].first) && net.output_nodes(net.arcs[j].first).size() > 1)
				{
					up_paths.zero(j);
					down_paths.zero(j);
				}

			(*flags->log_file) << "Up: {";
			for (j = 0; j < (int)up_paths.total.from.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << up_paths.total.from[j];
			}
			(*flags->log_file) << "} -> {";
			for (j = 0; j < (int)up_paths.total.to.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << up_paths.total.to[j];
			}
			(*flags->log_file) << "}" << endl;
			uptrans.clear();
			for (l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
			{
				for (j = 0; j < (int)net.arcs.size(); j++)
					if (up_paths.total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
						up_paths.total.nodes[j] += ind_max - (int)l->second.size();
			}
			(*flags->log_file) << up_paths.total << endl;
			while (up_paths.size() > 0 && (ium = net.closest_input(up_paths.total.maxes(), up_paths.total.to, path(net.arcs.size())).second) != -1)
			{
				uptrans.push_back(ium);
				up_paths = up_paths.avoidance(ium);
				for (l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
				{
					for (j = 0; j < (int)net.arcs.size(); j++)
						if (up_paths.total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
							up_paths.total.nodes[j] += ind_max - (int)l->second.size();
				}
				(*flags->log_file) << up_paths.total << endl;
			}

			(*flags->log_file) << "Down: {";
			for (j = 0; j < (int)down_paths.total.from.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << down_paths.total.from[j];
			}
			(*flags->log_file) << "} -> {";
			for (j = 0; j < (int)down_paths.total.to.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << down_paths.total.to[j];
			}
			(*flags->log_file) << "}" << endl;
			downtrans.clear();
			for (l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
			{
				for (j = 0; j < (int)net.arcs.size(); j++)
					if (down_paths.total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
						down_paths.total.nodes[j] += ind_max - (int)l->second.size();
			}
			(*flags->log_file) << down_paths.total << endl;
			while (down_paths.size() > 0 && (idm = net.closest_input(down_paths.total.maxes(), down_paths.total.to, path(net.arcs.size())).second) != -1)
			{
				downtrans.push_back(idm);
				down_paths = down_paths.avoidance(idm);
				for (l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
				{
					for (j = 0; j < (int)net.arcs.size(); j++)
						if (down_paths.total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
							down_paths.total.nodes[j] += ind_max - (int)l->second.size();
				}
				(*flags->log_file) << down_paths.total << endl;
			}
			(*flags->log_file) << endl;

			unique(&uptrans, &downtrans);

			if (uptrans.size() == 0 || downtrans.size() == 0)
			{
				cout << "Error: No solution for the conflict set: " << i->first << " -> {";
				for (j = 0; j < (int)lj->size(); j++)
					cout << (*lj)[j] << " ";
				cout << "}." << endl;
				net.print_dot(&cout, name);
			}
			else if (downtrans < uptrans)
				ip.push_back(pair<vector<int>, vector<int> >(downtrans, uptrans));
			else
				ip.push_back(pair<vector<int>, vector<int> >(uptrans, downtrans));
		}
	}

	for (i = net.positive_conflicts.begin(); i != net.positive_conflicts.end(); i++)
	{
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			down_paths.clear();
			up_paths.clear();
			jstart = net.start_path(*lj, vector<int>(1, i->first));
			istart = net.start_path(vector<int>(1, i->first), *lj);

			net.get_paths(jstart, vector<int>(1, i->first), &down_paths);
			net.get_paths(istart, *lj, &up_paths);

			uptrans = down_paths.total.to;
			downtrans = up_paths.total.to;

			(*flags->log_file) << "Down: {";
			for (j = 0; j < (int)down_paths.total.from.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << down_paths.total.from[j];
			}
			(*flags->log_file) << "} -> {";
			for (j = 0; j < (int)down_paths.total.to.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << down_paths.total.to[j];
			}
			(*flags->log_file) << "}" << endl;

			unique(&uptrans, &downtrans);
			if (uptrans.size() == 0 || downtrans.size() == 0)
			{
				cout << "Error: No solution for the conflict set: " << i->first << " -> {";
				for (j = 0; j < (int)lj->size(); j++)
					cout << (*lj)[j] << " ";
				cout << "}." << endl;
				cout << "Down Paths:" << endl;
				cout << down_paths << endl;
				net.print_dot(&cout, name);
			}
			else
				ip.push_back(pair<vector<int>, vector<int> >(uptrans, downtrans));
		}
	}

	for (i = net.negative_conflicts.begin(); i != net.negative_conflicts.end(); i++)
	{
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			down_paths.clear();
			up_paths.clear();
			jstart = net.start_path(*lj, vector<int>(1, i->first));
			istart = net.start_path(vector<int>(1, i->first), *lj);

			net.get_paths(jstart, vector<int>(1, i->first), &up_paths);
			net.get_paths(istart, *lj, &down_paths);

			uptrans = down_paths.total.to;
			downtrans = up_paths.total.to;

			(*flags->log_file) << "Up: {";
			for (j = 0; j < (int)up_paths.total.from.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << up_paths.total.from[j];
			}
			(*flags->log_file) << "} -> {";
			for (j = 0; j < (int)up_paths.total.to.size(); j++)
			{
				if (j != 0)
					(*flags->log_file) << ", ";
				(*flags->log_file) << up_paths.total.to[j];
			}
			(*flags->log_file) << "}" << endl;

			unique(&uptrans, &downtrans);

			if (uptrans.size() == 0 || downtrans.size() == 0)
			{
				cout << "Error: No solution for the conflict set: " << i->first << " -> {";
				for (j = 0; j < (int)lj->size(); j++)
					cout << (*lj)[j] << " ";
				cout << "}." << endl;
				cout << "Up Paths:" << endl;
				cout << up_paths << endl;
				net.print_dot(&cout, name);
			}
			else
				ip.push_back(pair<vector<int>, vector<int> >(uptrans, downtrans));
		}
	}

	unique(&ip);
	logic um, dm;
	for (j = 0; j < (int)ip.size(); j++)
	{
		vname = vars.unique_name("_sv");
		vid   = vars.insert(variable(vname, "node", 1, false, flags));
		vars.find(vname)->driven = true;

		um = logic(vid, 1);
		dm = logic(vid, 0);

		for (k = 0; k < (int)ip[j].first.size(); k++)
			net.insert_sv_at(ip[j].first[k], um);
		for (k = 0; k < (int)ip[j].second.size(); k++)
			net.insert_sv_at(ip[j].second[k], dm);
	}

	net.trim_branch_ids();
	net.gen_mutables();
	net.update();

	return (ip.size() > 0);
}

void process::generate_prs()
{
	//cout << "Generating " << name << endl;
	//print_dot(&cout);
	prs.generate_minterms(&net, flags);

	if (flags->log_base_prs() && kind() == "process")
	{
		(*flags->log_file) << "Production Rules: " << name << endl;
		print_prs(flags->log_file);
		//prs.print_enable_graph(flags->log_file, &net, name);
		prs.check(&net);
	}
}

void process::generate_bubbleless_prs()
{
	net.gen_senses();
	for (map<string, variable>::iterator vi = vars.global.begin(); vi != vars.global.end(); vi++)
		if (vi->second.driven)
			prs.insert(rule(vi->second.uid, &net, &vars, flags, false));

	if (flags->log_base_prs() && kind() == "process")
	{
		(*flags->log_file) << "Production Rules: " << name << endl;
		print_prs(flags->log_file);
	}
}

/* TODO: Factoring - production rules should be relatively short.
 * Look for common expressions between production rules and factor them
 * out into their own variable.
 */
void process::factor_prs()
{

	//Not as trivial as seemed on initial exploration.
	//Must find cost function for inserting state variable.
	//Must find benefit for inserting factor
	//Balance size of factored chunk vs how many rules factored from
	//Prioritize longer rules to reduce capacitive driving (i.e. not every 'factor removed' is equal)
	//MUST factor if over cap in series/parallel

	if (flags->log_factored_prs())
		print_prs(flags->log_file);
}

void process::parse_prs(string raw)
{
	int i, j, k;
	string r, e, v;
	map<string, pair<string, string> > pairs;
	map<string, pair<string, string> >::iterator pi;

	raw = remove_comments(raw);

	for (i = raw.find_first_of("\n\r"), j = 0; i != (int)raw.npos; j = i+1, i = raw.find_first_of("\n\r", j))
	{
		r = raw.substr(j, i-j);
		k = r.find(" -> ");
		if (k != (int)r.npos)
		{
			e = r.substr(0, k);
			v = r.substr(k+4);

			pi = pairs.find(v.substr(0, v.length()-1));
			if (pi != pairs.end())
			{
				if (v[v.length()-1] == '+')
					pi->second.first = e;
				else
					pi->second.second = e;
			}
			else
			{
				if (v[v.length()-1] == '+')
					pairs.insert(pair<string, pair<string, string> >(v.substr(0, v.length()-1), pair<string, string>(e, "")));
				else
					pairs.insert(pair<string, pair<string, string> >(v.substr(0, v.length()-1), pair<string, string>("", e)));
			}
		}
	}

	for (pi = pairs.begin(); pi != pairs.end(); pi++)
	{
		cout << pi->first << ",{" << pi->second.first << ", " << pi->second.second << "}" << endl;
		prs.insert(rule(pi->second.first, pi->second.second, pi->first, &vars, &net, flags));
	}
}

void process::elaborate_prs()
{
	int ufrom, dfrom;
	int utrans, dtrans;
	int uto, dto;
	vector<int> ot, op;
	vector<int> vl;
	vector<map<int, uint32_t> > sat;
	vector<map<int, uint32_t> >::iterator si;
	int i, j;

	for (i = 0; i < (int)prs.size(); i++)
	{
		sat = (prs[i].up() & logic(prs[i].uid, 0)).allsat();
		//sat = net.values.allsat(prs[i].up);
		for (si = sat.begin(); si != sat.end(); si++)
		{
			ufrom  = net.new_place(logic(*si), map<int, int>(), map<int, int>(), NULL);
			utrans = net.insert_transition(ufrom, logic(prs[i].uid, 1), map<int, int>(), map<int, int>(), NULL);
			uto    = net.insert_place(utrans, map<int, int>(), map<int, int>(), NULL);
			net.S[uto].index = net.S[ufrom].index >> net.T[utrans].index;
			net.S[ufrom].active = true;
			net.T[utrans].active = true;
		}

		sat = (prs[i].down() & logic(prs[i].uid, 1)).allsat();
		//sat = net.values.allsat(prs[i].down);
		for (si = sat.begin(); si != sat.end(); si++)
		{
			dfrom  = net.new_place(logic(*si), map<int, int>(), map<int, int>(), NULL);
			dtrans = net.insert_transition(dfrom, logic(prs[i].uid, 0), map<int, int>(), map<int, int>(), NULL);
			dto    = net.insert_place(dtrans, map<int, int>(), map<int, int>(), NULL);
			net.S[dto].index = net.S[dfrom].index >> net.T[dtrans].index;
			net.S[dfrom].active = true;
			net.T[dtrans].active = true;
		}
	}

	net.merge_conflicts();

	// Try to guess the environment's behavior
	/*vector<int> inactive;
	vector<int> active;
	map<string, variable>::iterator vi;
	for (vi = vars.global.begin(); vi != vars.global.end(); vi++)
	{
		if (!vi->second.driven)
			inactive.push_back(vi->second.uid);
		else
			active.push_back(vi->second.uid);
	}

	for (i = 0; i < (int)net.S.size(); i++)
		for (j = 0; j < (int)net.S.size(); j++)
			if (i != j && !net.connected(i, j) && net.values.apply_and(net.values.hide(net.S[i].index, inactive), net.S[j].index) > 1)
			{
				ufrom = net.values.hide(net.S[j].index, active);
				utrans = net.insert_transition(i, ufrom, map<int, int>(), NULL);
				net.connect(utrans, j);
			}*/

	//net.zip();

	for (int i = 0; i < (int)net.S.size(); i++)
		if (net.output_nodes(i).size() == 0)
		{
			net.remove_place(i);
			i--;
		}
}

void process::print_hse(ostream *fout)
{
	bool individual = false;
	if (fout == NULL)
	{
		individual = true;
		fout = new ofstream(name + ".hse");
	}

	(*fout) << "/* Process " << name << " */";
	def.print_hse("", fout);
	(*fout) << endl << endl;

	if (individual)
	{
		delete fout;
		fout = NULL;
	}
}

void process::print_dot(ostream *fout)
{
	bool individual = false;
	if (fout == NULL)
	{
		individual = true;
		fout = new ofstream(name + ".dot");
	}

	net.print_dot(fout, name);
	(*fout) << endl;

	if (individual)
	{
		delete fout;
		fout = NULL;
	}
}

void process::print_prs(ostream *fout)
{
	bool individual = false;
	if (fout == NULL)
	{
		individual = true;
		fout = new ofstream(name + ".prs");
	}

	(*fout) << "/* Process " << name << " */" << endl;
	prs.print(fout);
	map<string, variable>::iterator vi;
	type_space::iterator ki;

	(*fout) << "/* Environment */" << endl;
	for (vi = vars.label.begin(); vi != vars.label.end(); vi++)
	{
		ki = vars.types->find(vi->second.type);
		if (ki != vars.types->end() && ki->second != NULL && ki->second->kind() == "channel")
		{
			((channel*)ki->second)->send->print_prs(fout, vi->first + ".", vars.get_driven());
			((channel*)ki->second)->recv->print_prs(fout, vi->first + ".", vars.get_driven());
		}
	}
	(*fout) << endl;

	if (individual)
	{
		delete fout;
		fout = NULL;
	}
}
