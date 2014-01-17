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

#include "record.h"
#include "channel.h"
#include "process.h"
#include "operator.h"

operate::operate()
{
	name = "";
	_kind = "operate";
	is_inline = true;
	def.parent = NULL;
}

operate::operate(sstring raw, type_space *types, flag_space *flags)
{
	_kind = "operate";
	vars.types = types;
	is_inline = true;
	def.parent = NULL;
	this->flags = flags;

	parse(raw);

	types->insert(pair<sstring, operate*>(name, this));
}

operate::~operate()
{
	name = "";
	_kind = "operate";
	is_inline = true;
	def.parent = NULL;

	vars.clear();
}

operate &operate::operator=(operate p)
{
	def = p.def;
	prs = p.prs;
	vars = p.vars;
	flags = p.flags;
	return *this;
}

void operate::parse(sstring raw)
{
	chp = raw;

	int name_start = 0;
	int name_end = chp.find_first_of("(");
	int input_start = chp.find_first_of("(")+1;
	int input_end = chp.find_first_of(")");
	int sequential_start = chp.find_first_of("{")+1;
	int sequential_end = chp.length()-1;
	sstring io_sequential;
	sstring::iterator i, j;

	smap<sstring, variable> temp;
	smap<sstring, variable>::iterator vi, vj;
	type_space::iterator ti;
	list<sstring>::iterator ii, ij;

	name = chp.substr(name_start, name_end - name_start);
	io_sequential = chp.substr(input_start, input_end - input_start);

	if (flags->log_base_hse())
	{
		(*flags->log_file) << "Operator:\t" << chp << endl;
		(*flags->log_file) << "\tName:\t" << name << endl;
		(*flags->log_file) << "\tInputs:\t" << io_sequential << endl;
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

	if (args.size() > 3)
		cerr << "Error: Operators can have at most two inputs and one output." << endl;

	def.init(chp.substr(sequential_start, sequential_end - sequential_start), &vars, flags);

	variable *tv;

	name += "(";
	ij = args.begin();
	ij++;
	for (ii = ij; ii != args.end(); ii++)
	{
		if (ii != ij)
			name += ",";
		tv = vars.find(*ii);

		if (tv != NULL)
		{
			if (tv->driven)
				cerr << "Error: Input " << *ii << " driven in " << chp << endl;

			name += tv->type;
			if (tv->type == "node" && tv->fixed)
				name += "<" + sstring(tv->width) + ">";
		}
	}
	name += ")";

	if (flags->log_base_hse())
	{
		(*flags->log_file) << "\tVariables:" << endl;
		vars.print("\t\t", flags->log_file);
		(*flags->log_file) << "\tHSE:" << endl;
		def.print_hse("\t\t", flags->log_file);
		(*flags->log_file) << endl << endl;
	}
}

void operate::generate_states()
{
	process::generate_states();

	type_space::iterator j, my_iter;
	for (my_iter = vars.types->begin(); my_iter != vars.types->end() && my_iter->second != this; my_iter++);
	j = vars.types->find(my_iter->first.substr(0, my_iter->first.find_last_of(".")));

	svector<int> to_hide;
	for (list<sstring>::iterator i = args.begin(); i != args.end(); i++)
		for (smap<sstring, variable>::iterator j = vars.global.begin(); j != vars.global.end(); j++)
			if (j->second.name.substr(0, i->size()) == *i && (j->second.name.size() == i->size() || (j->second.name.size() > i->size() && j->second.name[i->size()] == '.')))
			{
				j->second.driven = false;
				to_hide.push_back(j->second.uid);
			}

	for (int i = 0; i < net.T.size(); i++)
	{
		net.T[i].index = net.T[i].index.hide(to_hide);

		if (net.T[i].active && net.T[i].index == 1)
			net.T[i].active = false;
	}

	vars.reset = vars.reset.hide(to_hide);
	vars.enforcements = vars.enforcements.hide(to_hide);

	for (int i = 0; i < net.S.size(); i++)
		if ((to_hide = net.output_nodes(i)).size() > 1)
			for (int j = 0; j < to_hide.size(); j++)
				for (int k = j+1; k < to_hide.size(); k++)
					if (!is_mutex(&net.T[net.index(to_hide[j])].index, &net.T[net.index(to_hide[k])].index, &vars.enforcements))
					{
						cerr << "Warning: We are going to need a mk_exclhi command to separate " << net.node_name(net.trans_id(i)) << " and " << net.node_name(net.trans_id(j)) << " in process " << name << "." << endl;
					}
}

void operate::trim_states()
{
	do
	{
		print_dot(flags->log_file);

		net.env.execs.clear();
		net.env.execs.push_back(program_execution(&net));
		type_space::iterator j;
		list<program_execution>::iterator exe = net.env.execs.begin();
		for (int k = 0; k < net.M0.size(); k++)
		{
			exe->pcs.push_front(program_counter(net.M0[k], &net));
			list<program_counter>::iterator pc = exe->pcs.begin();
			for (smap<sstring, variable>::iterator i = vars.label.begin(); i != vars.label.end(); i++)
			{
				if (i->second.type.find("operator?") != i->second.type.npos && (j = vars.types->find(i->second.type.substr(0, i->second.type.find_last_of(".")))) != vars.types->end() && j->second->kind() == "channel")
					pc->envs.push_back(program_environment(remote_program_counter(i->second.name.substr(0, i->second.name.find_last_of(".")), &net, &((channel*)j->second)->send->net)));
				else if (i->second.type.find("operator!") != i->second.type.npos && (j = vars.types->find(i->second.type.substr(0, i->second.type.find_last_of(".")))) != vars.types->end() && j->second->kind() == "channel")
					pc->envs.push_back(program_environment(remote_program_counter(i->second.name.substr(0, i->second.name.find_last_of(".")), &net, &((channel*)j->second)->recv->net)));
			}
		}

		type_space::iterator my_iter;
		for (my_iter = vars.types->begin(); my_iter != vars.types->end() && my_iter->second != this; my_iter++);
		cout << "LOOK " << name << " " << my_iter->first << endl;
		j = vars.types->find(my_iter->first.substr(0, my_iter->first.find_last_of(".")));

		for (list<program_execution>::iterator exe = net.env.execs.begin(); exe != net.env.execs.end(); exe++)
			for (list<program_counter>::iterator pc = exe->pcs.begin(); pc != exe->pcs.end(); pc++)
			{
				if (name.find("operator?") != name.npos && j != vars.types->end() && j->second->kind() == "channel")
					pc->envs.push_back(program_environment(remote_program_counter("this", &net, &((channel*)j->second)->send->net)));
				else if (name.find("operator!") != name.npos && j != vars.types->end() && j->second->kind() == "channel")
					pc->envs.push_back(program_environment(remote_program_counter("this", &net, &((channel*)j->second)->recv->net)));
			}

		net.update();
		print_dot(flags->log_file);
	} while(net.trim());

	net.check_assertions();
}

void operate::print_prs(ostream *fout, sstring prefix, svector<sstring> driven)
{
	smap<sstring, variable>::iterator vi;
	type_space::iterator ki;
	for (int i = 0; i < prs.rules.size(); i++)
		if (find(driven.begin(), driven.end(), prefix + vars.get_name(i)) == driven.end() && prs[i].vars != NULL)
			prs[i].print(*fout, prefix);
}

