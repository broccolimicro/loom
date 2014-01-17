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
	has_environment = false;
	def.parent = NULL;
	flags = NULL;
	net.vars = &vars;
	net.prs = &prs;
	prs.vars = &vars;
	env_net.vars = &env_vars;
	env_net.prs = &prs;
	env_def.parent = NULL;
}

process::process(sstring raw, type_space *types, flag_space *flags)
{
	_kind = "process";
	vars.types = types;
	env_vars.types = types;
	this->flags = flags;
	is_inline = false;
	has_environment = false;
	def.parent = NULL;
	env_def.parent = NULL;
	net.vars = &vars;
	env_net.vars = &env_vars;
	net.prs = &prs;
	env_net.prs = &prs;
	prs.vars = &vars;
	net.flags = flags;
	env_net.flags = flags;

	parse(raw);

	types->insert(pair<sstring, process*>(name, this));
}

process::~process()
{
	name = "";
	_kind = "process";
	chp = "";
	def.parent = NULL;
	env_def.parent = NULL;

	vars.clear();
}

process &process::operator=(process p)
{
	is_inline = p.is_inline;
	has_environment = p.has_environment;

	def = p.def;
	prs = p.prs;
	vars = p.vars;
	env_net = p.env_net;
	env_def = p.env_def;
	env_vars = p.env_vars;
	flags = p.flags;
	return *this;
}

void process::parse(sstring raw)
{
	if (raw.compare(0, 7, "inline ") == 0)
	{
		is_inline = true;
		raw = raw.substr(7);
	}
	else
		is_inline = false;

	has_environment = false;

	chp = raw;

	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("(");
	int input_start = chp.find_first_of("(")+1;
	int input_end = chp.find_first_of(")");
	int sequential_start = chp.find_first_of("{")+1;
	int sequential_end = chp.find_first_of_l0("}", sequential_start-1);
	int environment_start = chp.find_first_of("{", sequential_end);
	if (environment_start != chp.npos)
	{
		has_environment = true;
		environment_start++;
	}
	sstring io_sequential;
	sstring::iterator i, j;

	smap<sstring, variable> temp;
	smap<sstring, variable>::iterator vi, vj;
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
	if (has_environment)
		env_vars.insert(variable("reset", "node", 1, true, flags));

	for (i = io_sequential.begin(), j = io_sequential.begin(); i != io_sequential.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_sequential.end())
		{
			expand_instantiation(NULL, io_sequential.substr(j-io_sequential.begin(), i+1 - j), &vars, &args, flags, false);
			if (has_environment)
			{
				list<sstring> env_args;
				expand_instantiation(NULL, io_sequential.substr(j-io_sequential.begin(), i+1 - j), &env_vars, &env_args, flags, false);
			}
			j = i+2;
		}
	}

	sstring sequential = chp.substr(sequential_start, sequential_end - sequential_start);

	if (!is_inline)
	{
		expand_instantiation(NULL, "__sync call", &vars, &args, flags, false);
		sequential = "[call.r];call.a+;(" + sequential + ");[~call.r];call.a-";
	}

	def.init(sequential, &vars, flags);

	if (has_environment)
	{
		sstring environment = chp.substr(environment_start, chp.length()-1 - environment_start);
		env_def.init(environment, &env_vars, flags);
	}

	if (flags->log_base_hse())
	{
		(*flags->log_file) << "\tVariables: " << vars.reset.print(&vars) << endl;
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

	svector<int> end;
	net.M0.push_back(net.insert_place(svector<int>(), smap<int, int>(), smap<int, int>(), NULL));
	end = def.generate_states(&net, &prs, net.M0,  smap<int, int>(), smap<int, int>());
	if (is_inline)
		net.connect(end, net.M0);

	for (smap<sstring, variable>::iterator i = vars.global.begin(); i != vars.global.end(); i++)
		if (!i->second.driven)
			vars.reset = vars.reset.hide(i->second.uid);

	if (has_environment)
	{
		env_net.vars = &env_vars;
		env_net.prs = &prs;
		env_net.flags = flags;

		end.clear();
		env_net.M0.push_back(env_net.insert_place(svector<int>(), smap<int, int>(), smap<int, int>(), NULL));
		end = env_def.generate_states(&env_net, &prs, env_net.M0,  smap<int, int>(), smap<int, int>());
		env_net.connect(end, env_net.M0);

		for (smap<sstring, variable>::iterator i = env_vars.global.begin(); i != env_vars.global.end(); i++)
			if (!i->second.driven)
				env_vars.reset = env_vars.reset.hide(i->second.uid);
	}

	/*cout << "EXCL" << endl;
	for (int i = 0; i < (int)prs.excl.size(); i++)
	{
		for (int j = 0; j < (int)prs.excl[i].first.size(); j++)
			cout << vars.get_name(prs.excl[i].first[j]) << " ";
		cout << prs.excl[i].second << endl;
	}*/
}

void process::trim_states()
{
	do
	{
		print_dot(flags->log_file);
		env_net.print_dot(flags->log_file, "environment");

		net.env.execs.clear();
		net.env.execs.push_back(program_execution(&net));
		list<program_execution>::iterator exe = net.env.execs.begin();
		for (int k = 0; k < net.M0.size(); k++)
		{
			exe->pcs.push_front(program_counter(net.M0[k], &net));
			list<program_counter>::iterator pc = exe->pcs.begin();

			if (has_environment)
				pc->envs.push_back(program_environment(remote_program_counter("", &net, &env_net)));
			else
				for (smap<sstring, variable>::iterator i = vars.label.begin(); i != vars.label.end(); i++)
				{
					type_space::iterator j;
					if (i->second.type.find("operator?") != i->second.type.npos && (j = vars.types->find(i->second.type.substr(0, i->second.type.find_last_of(".")))) != vars.types->end() && j->second->kind() == "channel")
						pc->envs.push_back(program_environment(remote_program_counter(i->second.name.substr(0, i->second.name.find_last_of(".")), &net, &((channel*)j->second)->send->net)));
					else if (i->second.type.find("operator!") != i->second.type.npos && (j = vars.types->find(i->second.type.substr(0, i->second.type.find_last_of(".")))) != vars.types->end() && j->second->kind() == "channel")
						pc->envs.push_back(program_environment(remote_program_counter(i->second.name.substr(0, i->second.name.find_last_of(".")), &net, &((channel*)j->second)->recv->net)));
				}
		}

		net.update();
		print_dot(flags->log_file);
	} while(net.trim());

	net.check_assertions();
}

void process::direct_bubble_reshuffle()
{
	net.print_dot(flags->log_file, name);
	net.gen_tails();

	// Generate bubble reshuffle graph
	smap<pair<int, int>, pair<bool, bool> > bubble_graph = net.gen_isochronics();
	svector<bool> inverted(vars.global.size(), false);
	svector<pair<svector<int>, bool> > cycles;

	// Execute bubble reshuffling algorithm
	for (smap<pair<int, int>, pair<bool, bool> >::iterator i = bubble_graph.begin(); i != bubble_graph.end(); i++)
		cycles.merge(reshuffle_algorithm(i, true, &bubble_graph, svector<int>(), &inverted));
	cycles.unique();

	for (int i = 1; i < cycles.size(); i++)
		if (cycles[i].first == cycles[i-1].first)
		{
			cycles.erase(cycles.begin() + i);
			cycles.erase(cycles.begin() + i-1);
			i--;
		}

	for (int i = 0; i < cycles.size(); i++)
		if (cycles[i].second)
		{
			cycles.erase(cycles.begin() + i);
			i--;
		}

	// Remove Negative Cycles (currently negative cycles just throw an error message)
	for (int i = 0; i < cycles.size(); i++)
	{
		cerr << "Error: Negative cycle found in process " << name << " ";
		for (int j = 0; j < cycles[i].first.size(); j++)
		{
			if (j != 0)
				cerr << ", ";
			cerr << vars.get_name(cycles[i].first[j]);
		}
		cerr << endl;
	}

	// Apply global inversions to state space
	cout << "Inversions: " << name << endl;
	for (int i = 0; i < vars.global.size(); i++)
		cout << vars.get_name(i) << " " << inverted[i] << endl;

	for (smap<sstring, variable>::iterator i = vars.global.begin(); i != vars.global.end(); i++)
		if (inverted[i->second.uid])
		{
			for (int j = 0; j < net.T.size(); j++)
				for (int k = 0; k < net.T[j].index.terms.size(); k++)
					net.T[j].index.terms[k].sv_not(i->second.uid);

			for (int j = 0; j < net.S.size(); j++)
			{
				for (int k = 0; k < net.S[j].index.terms.size(); k++)
					net.S[j].index.terms[k].sv_not(i->second.uid);

				for (int k = 0; k < net.S[j].assumptions.terms.size(); k++)
					net.S[j].assumptions.terms[k].sv_not(i->second.uid);

				for (int k = 0; k < net.S[j].assertions.size(); k++)
					for (int l = 0; l < net.S[j].assertions[k].terms.size(); l++)
						net.S[j].assertions[k].terms[l].sv_not(i->second.uid);
			}

			for (int j = 0; j < vars.enforcements.terms.size(); j++)
				vars.enforcements.terms[j].sv_not(i->second.uid);

			for (int j = 0; j < vars.requirements.size(); j++)
				for (int k = 0; k < vars.requirements[j].terms.size(); k++)
					vars.requirements[j].terms[k].sv_not(i->second.uid);

			for (int j = 0; j < vars.reset.terms.size(); j++)
				vars.reset.terms[j].sv_not(i->second.uid);

			i->second.name = vars.invert_name(i->second.name);
			vars.global.insert(pair<sstring, variable>(vars.invert_name(i->first), i->second));
			i->second.name = i->first;
			i->second.uid = vars.global.size()-1;
			/*if (!i->second.driven)
				prs.insert(rule("~" + i->first, i->first, vars.invert_name(i->first), &vars, &net, flags));
			else
				prs.insert(rule("~" + vars.invert_name(i->first), vars.invert_name(i->first), i->first, &vars, &net, flags));*/
		}

	// At this point we assume that there are no negative cycles
	// Apply local inversions to state space
	svector<int> inputs, inputs2;
	int idx, uid;
	for (smap<pair<int, int>, pair<bool, bool> >::iterator i = bubble_graph.begin(); i != bubble_graph.end(); i++)
	{
		if (i->second.second)
		{
			smap<sstring, variable>::iterator vi;
			for (vi = vars.global.begin(); vi != vars.global.end() && vi->second.uid != i->first.first; vi++);
			smap<sstring, variable>::iterator vj = vars.global.find(vars.invert_name(vi->second.name));
			if (vj == vars.global.end())
			{
				vi->second.name = vars.invert_name(vi->second.name);
				vi->second.uid = vars.global.size();
				uid = vi->second.uid;
				vars.global.insert(pair<sstring, variable>(vars.invert_name(vi->first), vi->second));
				vi->second.name = vi->first;
				vi->second.uid = i->first.first;
				//prs.insert(rule("~" + vi->first, vi->first, vars.invert_name(vi->first), &vars, &net, flags));
			}
			else
				uid = vj->second.uid;

			for (int j = 0; j < net.T.size(); j++)
			{
				for (int k = 0; k < net.T[j].index.terms.size() && net.T[j].active; k++)
				{
					if (net.T[j].index.terms[k].val(i->first.second) != 2)
					{
						inputs = net.input_nodes(net.trans_id(j));

						for (int l = 0; l < inputs.size(); l++)
						{
							inputs2 = net.input_nodes(inputs[l]);
							for (int m = 0; m < inputs2.size(); m++)
								if (net.T[net.index(inputs2[m])].index == 1)
								{
									idx = inputs2[m];
									inputs2.erase(inputs2.begin() + m);
									m = 0;
									inputs2.merge(net.input_nodes(net.input_nodes(idx)));
									inputs2.unique();
								}

							for (int m = 0; m < inputs2.size(); m++)
								if (!net.T[net.index(inputs2[m])].active)
								{
									for (int n = 0; n < net.T[net.index(inputs2[m])].index.terms.size(); n++)
									{
										net.T[net.index(inputs2[m])].index.terms[n].set(uid, net.T[net.index(inputs2[m])].index.terms[n].get(i->first.first));
										net.T[net.index(inputs2[m])].index.terms[n].sv_not(uid);
										//net.T[net.index(inputs2[m])].index.terms[n].set(i->first.first, vX);
									}
								}
						}
					}
				}
			}
		}
	}

	cout << "digraph g3" << endl;
	cout << "{" << endl;

	for (smap<sstring, variable>::iterator i = vars.global.begin(); i != vars.global.end(); i++)
		cout << "\tV" << i->second.uid << " [label=\"" << i->second.name << "\"];" << endl;

	for (smap<pair<int, int>, pair<bool, bool> >::iterator i = bubble_graph.begin(); i != bubble_graph.end(); i++)
		cout << "\tV" << i->first.first << " -> V" << i->first.second << (i->second.first ? " [style=dashed]" : "") << (i->second.second ? " [arrowhead=odotnormal]" : "") << endl;

	cout << "}" << endl;

	net.update();

	prs.print(&cout);
	net.print_dot(&cout, name);
}

svector<pair<svector<int>, bool> > process::reshuffle_algorithm(smap<pair<int, int>, pair<bool, bool> >::iterator idx, bool forward, smap<pair<int, int>, pair<bool, bool> > *net, svector<int> cycle, svector<bool> *inverted)
{
	svector<pair<svector<int>, bool> > cycles;
	svector<int> negative_cycle;
	svector<int>::iterator found;

	cycle.push_back(forward ? idx->first.first : idx->first.second);

	found = cycle.find(forward ? idx->first.second : idx->first.first);
	if (found == cycle.end())
	{
		if (idx->second.first && idx->second.second && forward)
		{
			(*inverted)[idx->first.second] = !(*inverted)[idx->first.second];
			for (smap<pair<int, int>, pair<bool, bool> >::iterator j = net->begin(); j != net->end(); j++)
				if (j->first.first == idx->first.second || j->first.second == idx->first.second)
					j->second.second = !j->second.second;
		}
		else if (idx->second.first && idx->second.second && !forward)
		{
			(*inverted)[idx->first.first] = !(*inverted)[idx->first.first];
			for (smap<pair<int, int>, pair<bool, bool> >::iterator j = net->begin(); j != net->end(); j++)
				if (j->first.first == idx->first.first || j->first.second == idx->first.first)
					j->second.second = !j->second.second;
		}

		for (smap<pair<int, int>, pair<bool, bool> >::iterator i = net->begin(); i != net->end(); i++)
		{
			if (forward && (i->first.first == idx->first.second || i->first.second == idx->first.second) && i != idx)
				cycles.merge(reshuffle_algorithm(i, (i->first.first == idx->first.second), net, cycle, inverted));
			else if (!forward && (i->first.first == idx->first.first || i->first.second == idx->first.first) && i != idx)
				cycles.merge(reshuffle_algorithm(i, (i->first.first == idx->first.first), net, cycle, inverted));
		}
	}
	else if (idx->second.first && idx->second.second)
		cycles.push_back(pair<svector<int>, bool>(svector<int>(found, cycle.end()).unique(), false));
	else
		cycles.push_back(pair<svector<int>, bool>(svector<int>(found, cycle.end()).unique(), true));

	cycles.unique();

	for (int i = 1; i < cycles.size(); i++)
		if (cycles[i].first == cycles[i-1].first)
		{
			cycles.erase(cycles.begin() + i);
			cycles.erase(cycles.begin() + i-1);
			i--;
		}

	return cycles;
}

bool process::insert_state_vars()
{
	svector<pair<svector<int>, svector<int> > > ip;

	net.print_dot(flags->log_file, name);
	cout << "tails" << endl;
	net.gen_tails();
	cout << "conflicts" << endl;
	net.gen_conflicts();
	cout << "conditional places" << endl;
	net.gen_conditional_places();
	cout << "done" << endl;

	net.print_conflicts(*flags->log_file, name);
	net.print_indistinguishables(*flags->log_file, name);

	if (net.conflicts.size() == 0)
		return false;

	(*flags->log_file) << "PROCESS " << name << endl;
	for (smap<int, list<svector<int> > >::iterator i = net.conflicts.begin(); i != net.conflicts.end(); i++)
	{
		for (list<svector<int> >::iterator lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			path_space up_paths(net.arcs.size()), down_paths(net.arcs.size());
			pair<int, int> up_sense_count, down_sense_count;
			svector<int> uptrans, downtrans;

			generate_paths(&up_sense_count, svector<int>(1, i->first), &up_paths, &down_sense_count, *lj, &down_paths);
			remove_invalid_split_points(up_sense_count, svector<int>(1, i->first), &up_paths, down_sense_count, *lj, &down_paths);

			up_paths.print_bounds(*flags->log_file, "Up");
			uptrans = choose_split_points(&up_paths, false, false);

			down_paths.print_bounds(*flags->log_file, "Down");
			downtrans = choose_split_points(&down_paths, false, false);

			vector_symmetric_compliment(&uptrans, &downtrans);

			if (uptrans.size() == 0 || downtrans.size() == 0)
			{
				cerr << "Error: No solution for the conflict set: " << i->first << " -> {";
				for (int j = 0; j < (int)lj->size(); j++)
					cerr << (*lj)[j] << " ";
				cerr << "}." << endl;
				net.print_dot(&cout, name);
			}
			else if (uptrans <= downtrans)
				ip.push_back(pair<svector<int>, svector<int> >(uptrans, downtrans));
			else if (downtrans <= uptrans)
				ip.push_back(pair<svector<int>, svector<int> >(downtrans, uptrans));
		}
	}

	ip.unique();
	for (int j = 0; j < (int)ip.size(); j++)
	{
		sstring vname = vars.unique_name("_sv");
		int vid   = vars.insert(variable(vname, "node", 1, false, flags));
		vars.find(vname)->driven = true;

		logic um = logic(vid, 1);
		logic dm = logic(vid, 0);

		for (int k = 0; k < (int)ip[j].first.size(); k++)
			net.insert_sv_at(ip[j].first[k], um);
		for (int k = 0; k < (int)ip[j].second.size(); k++)
			net.insert_sv_at(ip[j].second[k], dm);
	}

	net.trim_branch_ids();
	net.update();

	return (ip.size() > 0);
}

bool process::insert_bubbleless_state_vars()
{
	svector<pair<svector<int>, svector<int> > > ip;

	net.print_dot(flags->log_file, name);
	net.gen_tails();
	net.gen_senses();
	net.gen_bubbleless_conflicts();
	net.gen_conditional_places();

	net.print_conflicts(*flags->log_file, name);
	net.print_indistinguishables(*flags->log_file, name);
	net.print_positive_conflicts(*flags->log_file, name);
	net.print_positive_indistinguishables(*flags->log_file, name);
	net.print_negative_conflicts(*flags->log_file, name);
	net.print_negative_indistinguishables(*flags->log_file, name);

	if (net.positive_conflicts.size() == 0 && net.negative_conflicts.size() == 0 && net.conflicts.size() == 0)
		return false;

	(*flags->log_file) << "PROCESS " << name << endl;
	for (smap<int, list<svector<int> > >::iterator i = net.conflicts.begin(); i != net.conflicts.end(); i++)
	{
		for (list<svector<int> >::iterator lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			path_space up_paths(net.arcs.size()), down_paths(net.arcs.size());
			pair<int, int> up_sense_count, down_sense_count;
			svector<int> uptrans, downtrans;

			generate_paths(&up_sense_count, svector<int>(1, i->first), &up_paths, &down_sense_count, *lj, &down_paths);
			remove_invalid_split_points(up_sense_count, svector<int>(1, i->first), &up_paths, down_sense_count, *lj, &down_paths);

			up_paths.print_bounds(*flags->log_file, "Up");
			uptrans = choose_split_points(&up_paths, false, false);

			down_paths.print_bounds(*flags->log_file, "Down");
			downtrans = choose_split_points(&down_paths, false, false);

			vector_symmetric_compliment(&uptrans, &downtrans);

			if (uptrans.size() == 0 || downtrans.size() == 0)
			{
				cerr << "Error: No solution for the conflict set: " << i->first << " -> {";
				for (int j = 0; j < (int)lj->size(); j++)
					cerr << (*lj)[j] << " ";
				cerr << "}." << endl;
				net.print_dot(&cout, name);
			}
			else
				ip.push_back(pair<svector<int>, svector<int> >(uptrans, downtrans));
		}
	}

	cout << "Positive Conflicts" << endl;
	for (smap<int, list<svector<int> > >::iterator i = net.positive_conflicts.begin(); i != net.positive_conflicts.end(); i++)
	{
		for (list<svector<int> >::iterator lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			path_space up_paths(net.arcs.size()), down_paths(net.arcs.size());
			pair<int, int> up_sense_count, down_sense_count;
			svector<int> uptrans, downtrans;

			generate_positive_paths(&up_sense_count, *lj, &up_paths, &down_sense_count, svector<int>(1, i->first), &down_paths);
			remove_invalid_split_points(up_sense_count, *lj, &up_paths, down_sense_count, svector<int>(1, i->first), &down_paths);

			up_paths.print_bounds(*flags->log_file, "Up");
			uptrans = choose_split_points(&up_paths, true, false);

			down_paths.print_bounds(*flags->log_file, "Down");
			downtrans = choose_split_points(&down_paths, false, true);

			vector_symmetric_compliment(&uptrans, &downtrans);

			if (uptrans.size() == 0 || downtrans.size() == 0)
			{
				cerr << "Error: No solution for the conflict set: " << i->first << " -> {";
				for (int j = 0; j < (int)lj->size(); j++)
					cerr << (*lj)[j] << " ";
				cerr << "}." << endl;
				net.print_dot(&cout, name);
			}
			else
				ip.push_back(pair<svector<int>, svector<int> >(uptrans, downtrans));
		}
	}

	cout << "Negative Conflicts" << endl;
	for (smap<int, list<svector<int> > >::iterator i = net.negative_conflicts.begin(); i != net.negative_conflicts.end(); i++)
	{
		for (list<svector<int> >::iterator lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			path_space up_paths(net.arcs.size()), down_paths(net.arcs.size());
			pair<int, int> up_sense_count, down_sense_count;
			svector<int> uptrans, downtrans;

			generate_negative_paths(&up_sense_count, svector<int>(1, i->first), &up_paths, &down_sense_count, *lj, &down_paths);
			remove_invalid_split_points(up_sense_count, svector<int>(1, i->first), &up_paths, down_sense_count, *lj, &down_paths);

			up_paths.print_bounds(*flags->log_file, "Up");
			uptrans = choose_split_points(&up_paths, true, false);

			down_paths.print_bounds(*flags->log_file, "Down");
			downtrans = choose_split_points(&down_paths, false, true);

			vector_symmetric_compliment(&uptrans, &downtrans);

			if (uptrans.size() == 0 || downtrans.size() == 0)
			{
				cerr << "Error: No solution for the conflict set: " << i->first << " -> {";
				for (int j = 0; j < (int)lj->size(); j++)
					cerr << (*lj)[j] << " ";
				cerr << "}." << endl;
				net.print_dot(&cout, name);
			}
			else
				ip.push_back(pair<svector<int>, svector<int> >(uptrans, downtrans));
		}
	}

	ip.unique();
	for (int j = 0; j < (int)ip.size(); j++)
	{
		sstring vname = vars.unique_name("_sv");
		int vid   = vars.insert(variable(vname, "node", 1, false, flags));
		vars.find(vname)->driven = true;

		logic um = logic(vid, 1);
		logic dm = logic(vid, 0);

		for (int k = 0; k < (int)ip[j].first.size(); k++)
			net.insert_sv_at(ip[j].first[k], um);
		for (int k = 0; k < (int)ip[j].second.size(); k++)
			net.insert_sv_at(ip[j].second[k], dm);
	}

	net.trim_branch_ids();
	net.update();

	return (ip.size() > 0);
}

void process::generate_prs()
{
	cout << "Generating " << name << endl;
	print_dot(&cout);
	net.gen_tails();
	prs.generate_minterms(&net, flags);

	if (flags->log_base_prs() && kind() == "process")
	{
		(*flags->log_file) << "Production Rules: " << name << endl;
		print_prs(flags->log_file);
		prs.check(&net);
	}
}

void process::generate_bubbleless_prs()
{
	net.gen_tails();
	net.gen_senses();
	for (smap<sstring, variable>::iterator vi = vars.global.begin(); vi != vars.global.end(); vi++)
		if (vi->second.driven)
			prs.insert(rule(vi->second.uid, &net, &vars, flags, false));

	if (flags->log_base_prs() && kind() == "process")
	{
		(*flags->log_file) << "Production Rules: " << name << endl;
		print_prs(flags->log_file);
	}
}

void process::bubble_reshuffle()
{
	//net.print_dot(flags->log_file, name);
	net.gen_tails();

	cout << "Production Rules: " << name << endl;
	prs.print(&cout);

	// Generate bubble reshuffle graph
	smap<pair<int, int>, pair<bool, bool> > bubble_graph = prs.gen_bubble_reshuffle_graph();
	svector<bool> inverted(vars.global.size(), false);
	svector<pair<svector<int>, bool> > cycles;

	// Execute bubble reshuffling algorithm
	for (smap<pair<int, int>, pair<bool, bool> >::iterator i = bubble_graph.begin(); i != bubble_graph.end(); i++)
		cycles.merge(reshuffle_algorithm(i, true, &bubble_graph, svector<int>(), &inverted));
	cycles.unique();

	for (int i = 1; i < cycles.size(); i++)
		if (cycles[i].first == cycles[i-1].first)
		{
			cycles.erase(cycles.begin() + i);
			cycles.erase(cycles.begin() + i-1);
			i--;
		}

	for (int i = 0; i < cycles.size(); i++)
		if (cycles[i].second)
		{
			cycles.erase(cycles.begin() + i);
			i--;
		}

	// Remove Negative Cycles (currently negative cycles just throw an error message)
	for (int i = 0; i < cycles.size(); i++)
	{
		cerr << "Error: Negative cycle found in process " << name << " ";
		for (int j = 0; j < cycles[i].first.size(); j++)
		{
			if (j != 0)
				cerr << ", ";
			cerr << vars.get_name(cycles[i].first[j]);
		}
		cerr << endl;
	}

	// Apply global inversions to production rules
	/*cout << "Inversions: " << name << endl;
	for (int i = 0; i < vars.global.size(); i++)
		cout << vars.get_name(i) << " " << inverted[i] << endl;*/

	svector<variable> toadd;
	logic temp;

	int uid = 0;
	for (smap<sstring, variable>::iterator i = vars.global.begin(); i != vars.global.end(); i++)
		if (inverted[i->second.uid])
		{
			for (int j = 0; j < prs.rules.size(); j++)
				for (int k = 0; k < 2; k++)
					for (int l = 0; l < prs.rules[j].guards[k].terms.size(); l++)
						prs.rules[j].guards[k].terms[l].sv_not(i->second.uid);

			prs.rules[i->second.uid].invert();

			toadd.push_back(i->second);
			toadd.back().name = vars.invert_name(toadd.back().name);
			i->second.uid = vars.global.size() + toadd.size() - 1;
		}

	for (int i = 0; i < toadd.size(); i++)
	{
		vars.global.insert(pair<sstring, variable>(toadd[i].name, toadd[i]));

		if (toadd[i].driven)
			prs.insert(rule("~" + toadd[i].name, toadd[i].name, vars.invert_name(toadd[i].name), &vars, &net, flags));
		else
			prs.insert(rule("~" + vars.invert_name(toadd[i].name), vars.invert_name(toadd[i].name), toadd[i].name, &vars, &net, flags));
	}

	// Apply local inversions to production rules
	for (smap<pair<int, int>, pair<bool, bool> >::iterator i = bubble_graph.begin(); i != bubble_graph.end(); i++)
	{
		if (i->second.second)
		{
			smap<sstring, variable>::iterator vi;
			for (vi = vars.global.begin(); vi != vars.global.end() && vi->second.uid != i->first.first; vi++);
			smap<sstring, variable>::iterator vj = vars.global.find(vars.invert_name(vi->second.name));
			if (vj == vars.global.end())
			{
				vi->second.name = vars.invert_name(vi->second.name);
				vi->second.uid = vars.global.size();
				uid = vars.global.size();
				vars.global.insert(pair<sstring, variable>(vi->second.name, vi->second));
				vi->second.name = vi->first;
				vi->second.uid = i->first.first;
				prs.insert(rule("~" + vi->first, vi->first, vars.invert_name(vi->first), &vars, &net, flags));
			}
			else
				uid = vj->second.uid;

			for (int j = 0; j < 2; j++)
				for (int k = 0; k < prs.rules[i->first.second].guards[j].terms.size(); k++)
				{
					prs.rules[i->first.second].guards[j].terms[k].set(uid, prs.rules[i->first.second].guards[j].terms[k].get(i->first.first));
					prs.rules[i->first.second].guards[j].terms[k].sv_not(uid);
					prs.rules[i->first.second].guards[j].terms[k].set(i->first.first, vX);
				}
		}
	}

	cout << "digraph g1" << endl;
	cout << "{" << endl;

	for (smap<sstring, variable>::iterator i = vars.global.begin(); i != vars.global.end(); i++)
		cout << "\tV" << i->second.uid << " [label=\"" << i->second.name << "\"];" << endl;

	for (smap<pair<int, int>, pair<bool, bool> >::iterator i = bubble_graph.begin(); i != bubble_graph.end(); i++)
		cout << "\tV" << i->first.first << " -> V" << i->first.second << (i->second.first ? " [style=dashed]" : "") << (i->second.second ? " [arrowhead=odotnormal]" : "") << endl;

	cout << "}" << endl;

	cout << endl;

	prs.print(&cout);
	cout << endl << endl;
	//net.print_dot(&cout, name);
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

void process::generate_paths(pair<int, int> *up_sense_count, svector<int> up_start, path_space *up_paths, pair<int, int> *down_sense_count, svector<int> down_start,  path_space *down_paths)
{
	down_paths->clear();
	up_paths->clear();
	svector<int> iin;
	for (int j = 0; j < up_start.size(); j++)
		iin.merge(net.input_arcs(up_start[j]));

	svector<int> jin;
	for (int j = 0; j < down_start.size(); j++)
		jin.merge(net.input_arcs(down_start[j]));

	svector<int> istart = net.start_path(iin, jin);
	svector<int> jstart = net.start_path(jin, iin);
	pair<int, int> swap_sense_count;
	path_space swap_path_space(net.arcs.size());

	*up_sense_count = net.get_input_sense_count(up_start);
	*down_sense_count = net.get_input_sense_count(down_start);

	if (up_sense_count->second > up_sense_count->first && down_sense_count->first > down_sense_count->second)
	{
		net.get_paths(istart, jin, up_paths);
		net.get_paths(jstart, iin, down_paths);
	}
	else if (up_sense_count->first > up_sense_count->second && down_sense_count->second > down_sense_count->first)
	{
		net.get_paths(istart, jin, down_paths);
		net.get_paths(jstart, iin, up_paths);
		swap_sense_count = *up_sense_count;
		*up_sense_count = *down_sense_count;
		*down_sense_count = swap_sense_count;
	}
	else
	{
		net.get_paths(istart, jin, up_paths);
		net.get_paths(jstart, iin, down_paths);

		if (up_sense_count->second > up_sense_count->first && down_sense_count->second > down_sense_count->first && up_paths->length() > down_paths->length())
		{
			swap_path_space = *up_paths;
			*up_paths = *down_paths;
			*down_paths = swap_path_space;
			swap_sense_count = *up_sense_count;
			*up_sense_count = *down_sense_count;
			*down_sense_count = swap_sense_count;
		}
		else if (up_sense_count->first > up_sense_count->second && down_sense_count->first > down_sense_count->second && down_paths->length() > up_paths->length())
		{
			swap_path_space = *up_paths;
			*up_paths = *down_paths;
			*down_paths = swap_path_space;
			swap_sense_count = *up_sense_count;
			*up_sense_count = *down_sense_count;
			*down_sense_count = swap_sense_count;
		}
	}
}

void process::generate_positive_paths(pair<int, int> *up_sense_count, svector<int> up_start, path_space *up_paths, pair<int, int> *down_sense_count, svector<int> down_start,  path_space *down_paths)
{
	down_paths->clear();
	up_paths->clear();
	svector<int> iin;
	for (int j = 0; j < up_start.size(); j++)
		iin.merge(net.input_arcs(up_start[j]));

	svector<int> jin;
	for (int j = 0; j < down_start.size(); j++)
		jin.merge(net.input_arcs(down_start[j]));

	svector<int> istart = net.start_path(iin, jin);
	svector<int> jstart = net.start_path(jin, iin);

	*up_sense_count = net.get_input_sense_count(up_start);
	*down_sense_count = net.get_input_sense_count(down_start);
	down_sense_count->second = 0;

	net.get_paths(istart, jin, up_paths);
	net.get_paths(jstart, iin, down_paths);
}

void process::generate_negative_paths(pair<int, int> *up_sense_count, svector<int> up_start, path_space *up_paths, pair<int, int> *down_sense_count, svector<int> down_start,  path_space *down_paths)
{
	down_paths->clear();
	up_paths->clear();
	svector<int> iin;
	for (int j = 0; j < up_start.size(); j++)
		iin.merge(net.input_arcs(up_start[j]));

	svector<int> jin;
	for (int j = 0; j < down_start.size(); j++)
		jin.merge(net.input_arcs(down_start[j]));

	svector<int> istart = net.start_path(iin, jin);
	svector<int> jstart = net.start_path(jin, iin);
	pair<int, int> swap_sense_count;
	path_space swap_path_space(net.arcs.size());

	*up_sense_count = net.get_input_sense_count(up_start);
	up_sense_count->first = 0;
	*down_sense_count = net.get_input_sense_count(down_start);

	net.get_paths(istart, jin, up_paths);
	net.get_paths(jstart, iin, down_paths);
}

void process::remove_invalid_split_points(pair<int, int> up_sense_count, svector<int> up_start, path_space *up_paths, pair<int, int> down_sense_count, svector<int> down_start, path_space *down_paths)
{
	path_space up_inv = up_paths->inverse();
	path_space down_inv = down_paths->inverse();

	path up_mask = up_inv.get_mask();
	path down_mask = down_inv.get_mask();

	up_paths->apply_mask(down_mask);
	down_paths->apply_mask(up_mask);

	if (up_sense_count.first != 0)
		net.zero_paths(up_paths, up_start);

	if (down_sense_count.second != 0)
		net.zero_paths(down_paths, down_start);

	for (int j = 0; j < up_paths->total.to.size(); j++)
		if (net.is_place(net.arcs[up_paths->total.to[j]].second))
			up_paths->zero(up_paths->total.to[j]);

	for (int j = 0; j < down_paths->total.to.size(); j++)
		if (net.is_place(net.arcs[down_paths->total.to[j]].second))
			down_paths->zero(down_paths->total.to[j]);

	for (int j = 0; j < (int)net.arcs.size(); j++)
		if (net.is_place(net.arcs[j].first) && net.output_nodes(net.arcs[j].first).size() > 1)
		{
			up_paths->zero(j);
			down_paths->zero(j);
		}
}

svector<int> process::choose_split_points(path_space *paths, bool up, bool down)
{
	svector<int> result;
	int max;

	for (smap<int, list<svector<int> > >::iterator l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
	{
		for (int j = 0; j < (int)net.arcs.size(); j++)
			if (paths->total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
				paths->total.nodes[j] += net.max_indistinguishables - (int)l->second.size();
	}
	if (up)
	{
		for (smap<int, list<svector<int> > >::iterator l = net.negative_indistinguishable.begin(); l != net.negative_indistinguishable.end(); l++)
		{
			for (int j = 0; j < (int)net.arcs.size(); j++)
				if (paths->total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
					paths->total.nodes[j] += net.max_negative_indistinguishables - (int)l->second.size();
		}
	}
	else if (down)
	{
		for (smap<int, list<svector<int> > >::iterator l = net.positive_indistinguishable.begin(); l != net.positive_indistinguishable.end(); l++)
		{
			for (int j = 0; j < (int)net.arcs.size(); j++)
				if (paths->total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
					paths->total.nodes[j] += net.max_positive_indistinguishables - (int)l->second.size();
		}
	}
	(*flags->log_file) << paths->total << endl;

	while (paths->size() > 0 && (max = net.closest_input(paths->total.maxes(), paths->total.to, path(net.arcs.size())).second) != -1)
	{
		result.push_back(max);
		*paths = paths->avoidance(max);

		for (smap<int, list<svector<int> > >::iterator l = net.indistinguishable.begin(); l != net.indistinguishable.end(); l++)
		{
			for (int j = 0; j < (int)net.arcs.size(); j++)
				if (paths->total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
					paths->total.nodes[j] += net.max_indistinguishables - (int)l->second.size();
		}
		if (up)
		{
			for (smap<int, list<svector<int> > >::iterator l = net.negative_indistinguishable.begin(); l != net.negative_indistinguishable.end(); l++)
			{
				for (int j = 0; j < (int)net.arcs.size(); j++)
					if (paths->total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
						paths->total.nodes[j] += net.max_negative_indistinguishables - (int)l->second.size();
			}
		}
		else if (down)
		{
			for (smap<int, list<svector<int> > >::iterator l = net.positive_indistinguishable.begin(); l != net.positive_indistinguishable.end(); l++)
			{
				for (int j = 0; j < (int)net.arcs.size(); j++)
					if (paths->total.nodes[j] > 0 && (net.arcs[j].first == l->first || net.arcs[j].second == l->first))
						paths->total.nodes[j] += net.max_positive_indistinguishables - (int)l->second.size();
			}
		}
		(*flags->log_file) << paths->total << endl;
	}
	return result;
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
	smap<sstring, variable>::iterator vi;
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
