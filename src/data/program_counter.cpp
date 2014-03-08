/*
 * program_counter.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: nbingham
 */

#include "program_counter.h"
#include "petri.h"
#include "variable_space.h"
#include <random>

program_counter::program_counter()
{
	net = NULL;
	done = false;
	elaborate = false;
}

program_counter::program_counter(string name, bool elaborate, petri_index index, petri_net *net)
{
	this->name = name;
	this->net = net;
	this->index = index;
	if (net->vars->reset.terms.size() > 0)
		this->state = net->vars->reset.terms.front();
	this->done = false;
	this->elaborate = elaborate;
}

program_counter::~program_counter()
{

}

bool program_counter::is_active()
{
	return net->at(index).active;
}

bool program_counter::is_active(petri_index i)
{
	return net->at(i).active;
}

bool program_counter::is_satisfied()
{
	for (svector<minterm>::iterator term = net->at(index).index.terms.begin(); term != net->at(index).index.terms.end(); term++)
		if ((*term & state) != 0)
			return true;

	return false;
}

bool program_counter::is_satisfied(petri_index i)
{
	for (svector<minterm>::iterator term = net->at(i).index.terms.begin(); term != net->at(i).index.terms.end(); term++)
		if ((*term & state) != 0)
			return true;

	return false;
}

bool program_counter::next_has_active_or_satisfied()
{
	for (int i = 0; i < net->arcs.size(); i++)
		if (net->arcs[i].first == index && (net->at(net->arcs[i].second).active || is_satisfied(net->arcs[i].second)))
			return true;

	return false;
}

svector<petri_index> program_counter::output_nodes()
{
	return net->next(index);
}

svector<petri_index> program_counter::input_nodes()
{
	return net->prev(index);
}

canonical &program_counter::predicate()
{
	return net->at(index).index;
}

void program_counter::apply(minterm term)
{
	state |= (state & term).xoutnulls();
}

void program_counter::set(minterm term)
{
	state = (state & term).xoutnulls();
}

bool operator==(program_counter p1, program_counter p2)
{
	return p1.name == p2.name && p1.net == p2.net && p1.index == p2.index;
}

remote_petri_index::remote_petri_index()
{

}

remote_petri_index::remote_petri_index(int idx, int iter, bool place) : petri_index(idx, place)
{
	iteration = iter;
}

remote_petri_index::remote_petri_index(petri_index i, int iter) : petri_index(i)
{
	iteration = iter;
}

remote_petri_index::~remote_petri_index()
{

}

bool operator==(remote_petri_index i1, remote_petri_index i2)
{
	return (i1.data == i2.data && i1.iteration == i2.iteration);
}

remote_program_counter::remote_program_counter()
{
	net = NULL;
}

remote_program_counter::remote_program_counter(string name, petri_net *net)
{
	this->place_iteration.resize(net->S.size(), 0);
	this->trans_iteration.resize(net->T.size(), pair<int, int>(0, 0));
	this->input_size.resize(net->T.size(), 0);
	for (petri_index i(0, false); i < net->T.size(); i++)
		this->input_size[i.idx()] = net->incoming(i).size();

	this->name = name;
	this->net = net;
	for (int i = 0; i < net->M0.size(); i++)
	{
		int iteration = nid(net->M0[i]);
		this->begin.push_back(remote_petri_index(net->M0[i], iteration));
		this->end.push_back(remote_petri_index(net->M0[i], iteration));
	}
}

remote_program_counter::~remote_program_counter()
{

}

svector<remote_petri_arc> remote_program_counter::input_arcs(remote_petri_index n)
{
	svector<remote_petri_arc> result;
	for (svector<remote_petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->second == n)
			result.push_back(*arc);
	return result;
}

svector<remote_petri_arc> remote_program_counter::output_arcs(remote_petri_index n)
{
	svector<remote_petri_arc> result;
	for (svector<remote_petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->first == n)
			result.push_back(*arc);
	return result;
}

svector<remote_petri_index> remote_program_counter::input_nodes(remote_petri_index n)
{
	svector<remote_petri_index> result;
	for (svector<remote_petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->second == n)
			result.push_back(arc->first);
	return result;
}

svector<remote_petri_index> remote_program_counter::output_nodes(remote_petri_index n)
{
	svector<remote_petri_index> result;
	for (svector<remote_petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->first == n)
			result.push_back(arc->second);
	return result;
}

bool remote_program_counter::is_active(petri_index i)
{
	return net->at(i).active;
}

bool remote_program_counter::is_satisfied(petri_index i, minterm state)
{
	for (svector<minterm>::iterator term = net->at(i).index.terms.begin(); term != net->at(i).index.terms.end(); term++)
		if ((*term & state) != 0)
			return true;

	return false;
}

bool remote_program_counter::is_vacuous(petri_index i, minterm state)
{
	canonical temp = ~net->at(i).index;
	return is_mutex(&temp, &state);
}

bool remote_program_counter::next_has_active_or_satisfied(remote_petri_index i, minterm state, svector<petri_index> &outgoing)
{
	canonical wait = waits(i);
	for (int j = 0; j < net->arcs.size(); j++)
		if (net->arcs[j].first == i && (net->at(net->arcs[j].second).active || (is_satisfied(net->arcs[j].second, state) && wait != 0)))
			outgoing.push_back(net->arcs[j].second);
	return outgoing.size() > 0;
}

bool remote_program_counter::is_one(petri_index i)
{
	return (net->at(i).index == 1);
}

int remote_program_counter::nid(petri_index idx)
{
	if (idx.is_place())
		return place_iteration[idx.idx()]++;
	else
	{
		int i = idx.idx();
		if (trans_iteration[i].second >= input_size[i])
		{
			trans_iteration[i].second = 0;
			trans_iteration[i].first++;
		}

		trans_iteration[i].second++;
		return trans_iteration[i].first;
	}
}

int remote_program_counter::count(int n)
{
	int total = 1;
	for (int j = n+1; j < end.size(); j++)
	{
		if (end[j] == end[n])
			total++;
	}
	return total;
}

void remote_program_counter::merge(int n)
{
	for (int j = n+1; j < end.size();)
	{
		if (end[j] == end[n])
			end.erase(end.begin() + j);
		else
			j++;
	}
}

minterm remote_program_counter::firings()
{
	minterm result;
	list<petri_index> firing_nodes;
	for (svector<remote_petri_arc>::iterator i = arcs.begin(); i != arcs.end(); i++)
	{
		if (i->second.is_trans() && net->at(i->second).active)
			firing_nodes.push_back(i->second);
		else if (i->first.is_trans() && net->at(i->first).active)
			firing_nodes.push_back(i->first);
	}

	firing_nodes.sort();
	firing_nodes.unique();

	for (list<petri_index>::iterator i = firing_nodes.begin(); i != firing_nodes.end(); i++)
		for (svector<minterm>::iterator j = net->at(*i).index.terms.begin(); j != net->at(*i).index.terms.end(); j++)
			result &= *j;

	return result;
}

canonical remote_program_counter::waits(remote_petri_index n)
{
	list<remote_petri_index> iter(1, n);
	list<petri_index> waiting_nodes;
	for (list<remote_petri_index>::iterator end = iter.begin(); end != iter.end(); end = iter.erase(end))
	{
		if (end->is_trans() && !net->at(*end).active)
			waiting_nodes.push_back(*end);

		svector<remote_petri_index> inputs = input_nodes(*end);
		iter.insert(iter.end(), inputs.begin(), inputs.end());
	}

	waiting_nodes.sort();
	waiting_nodes.unique();

	canonical result(1);
	for (list<petri_index>::iterator i = waiting_nodes.begin(); i != waiting_nodes.end(); i++)
		result &= net->at(*i).index;

	return result;
}

remote_program_counter &remote_program_counter::operator=(remote_program_counter pc)
{
	name = pc.name;
	net = pc.net;
	begin = pc.begin;
	end = pc.end;
	arcs = pc.arcs;
	place_iteration = pc.place_iteration;
	trans_iteration = pc.trans_iteration;
	input_size = pc.input_size;
	return *this;
}

ostream &operator<<(ostream &os, remote_petri_index i)
{
	os << i.name() << "." << i.iteration;
	return os;
}

program_execution::program_execution()
{
	deadlock = false;
}

program_execution::program_execution(const program_execution &exec)
{
	deadlock = exec.deadlock;
	pcs.reserve(exec.pcs.size());
	rpcs.reserve(exec.rpcs.size());
	for (int i = 0; i < exec.pcs.size(); i++)
		pcs.push_back(exec.pcs[i]);
	for (int i = 0; i < exec.rpcs.size(); i++)
		rpcs.push_back(exec.rpcs[i]);
}

program_execution::~program_execution()
{

}

int program_execution::count(int pci)
{
	int total = 0;
	for (int pcj = 0; pcj < pcs.size(); pcj++)
		if (pcs[pci] == pcs[pcj])
			total++;

	return total;
}

int program_execution::merge(int pci)
{
	for (int pcj = 0; pcj < pcs.size(); )
	{
		if (pci != pcj && pcs[pci] == pcs[pcj])
		{
			pcs[pci].state &= pcs[pcj].state;
			pcs.erase(pcs.begin() + pcj);
			if (pcj < pci)
				pci--;
		}
		else
			pcj++;
	}

	return pci;
}

bool program_execution::done()
{
	for (svector<program_counter>::iterator pci = pcs.begin(); pci != pcs.end(); pci++)
		if (!pci->done && pci->elaborate)
			return false;

	return true;
}

void program_execution::init_pcs(string name, petri_net *net, bool elaborate)
{
	cout << "Initializing pc " << name << endl;
	for (int k = 0; k < net->M0.size(); k++)
		pcs.push_back(program_counter(name, elaborate, net->M0[k], net));
}

void program_execution::init_rpcs(string name, petri_net *net)
{
	cout << "Initializing rpc " << name << " ";
	rpcs.push_back(remote_program_counter(name, net));
	cout << rpcs.size() << endl;
}

void program_execution_space::duplicate_execution(program_execution *exec_in, program_execution **exec_out)
{
	execs.push_back(*exec_in);
	*exec_out = &(*prev(execs.end()));
}

void program_execution_space::duplicate_counter(program_execution *exec_in, int pc_in, int &pc_out)
{
	exec_in->pcs.push_back(exec_in->pcs[pc_in]);
	pc_out = exec_in->pcs.size()-1;
}

bool program_execution_space::remote_end_ready(program_execution *exec, int &rpc, int &idx, svector<petri_index> &outgoing)
{
	outgoing.clear();
	while (rpc < exec->rpcs.size())
	{
		minterm state;
		for (int i = 0; i < exec->pcs.size(); i++)
			state &= translate(exec->pcs[i].name, exec->pcs[i].net, exec->pcs[i].state, exec->rpcs[rpc].name, exec->rpcs[rpc].net);

		while (idx < exec->rpcs[rpc].end.size())
		{
			if (exec->rpcs[rpc].end[idx].is_trans() && exec->rpcs[rpc].input_size[exec->rpcs[rpc].end[idx].idx()] == 1)
			{
				outgoing.merge(exec->rpcs[rpc].net->next(exec->rpcs[rpc].end[idx]));
				return true;
			}
			else if (exec->rpcs[rpc].end[idx].is_trans() && exec->rpcs[rpc].count(idx) == exec->rpcs[rpc].input_size[exec->rpcs[rpc].end[idx].idx()])
			{
				exec->rpcs[rpc].merge(idx);
				outgoing.merge(exec->rpcs[rpc].net->next(exec->rpcs[rpc].end[idx]));
				return true;
			}
			else if (exec->rpcs[rpc].end[idx].is_place() && exec->rpcs[rpc].next_has_active_or_satisfied(exec->rpcs[rpc].end[idx], state, outgoing))
				return true;

			idx++;
		}

		rpc++;

		while (rpc < exec->rpcs.size() && exec->rpcs[rpc].end.empty())
			exec->rpcs.erase(exec->rpcs.begin() + rpc);

		if (rpc < exec->rpcs.size())
			idx = 0;
	}

	return false;
}

bool program_execution_space::remote_begin_ready(program_execution *exec, int &rpc, int &idx)
{
	while (rpc < exec->rpcs.size())
	{
		minterm state;
		for (int i = 0; i < exec->pcs.size(); i++)
			state &= translate(exec->pcs[i].name, exec->pcs[i].net, exec->pcs[i].state, exec->rpcs[rpc].name, exec->rpcs[rpc].net);

		while (idx < exec->rpcs[rpc].begin.size())
		{
			if (exec->rpcs[rpc].begin[idx].is_place() || (!exec->rpcs[rpc].net->at(exec->rpcs[rpc].begin[idx]).active && exec->rpcs[rpc].is_vacuous(exec->rpcs[rpc].begin[idx], state)))
			{
				bool wall = false;
				for (int j = 0; j != exec->rpcs[rpc].end.size() && !wall; j++)
					if (exec->rpcs[rpc].begin[idx] == exec->rpcs[rpc].end[j])
						wall = true;

				if (!wall)
					return true;
			}

			list<remote_petri_index> iter(1, exec->rpcs[rpc].begin[idx]);

			for (list<remote_petri_index>::iterator n = iter.begin(); n != iter.end(); n = iter.erase(n))
			{
				if (n->is_trans() && exec->rpcs[rpc].net->at(*n).active && !exec->rpcs[rpc].is_one(*n) && exec->rpcs[rpc].is_vacuous(*n, state))
				{
					bool wall = false;
					for (int j = 0; j != exec->rpcs[rpc].end.size() && !wall; j++)
						if (*n == exec->rpcs[rpc].end[j])
							wall = true;

					if (!wall)
						return true;
				}

				if (n->is_place() || exec->rpcs[rpc].net->at(*n).active || exec->rpcs[rpc].is_vacuous(*n, state))
					for (svector<remote_petri_arc>::iterator a = exec->rpcs[rpc].arcs.begin(); a != exec->rpcs[rpc].arcs.end(); a++)
						if (a->first == *n)
							iter.push_back(a->second);
			}

			idx++;
		}

		rpc++;

		while (rpc < exec->rpcs.size() && exec->rpcs[rpc].begin.empty())
			exec->rpcs.erase(exec->rpcs.begin() + rpc);

		if (rpc < exec->rpcs.size())
			idx = 0;
	}

	return false;
}

void program_execution_space::full_elaborate()
{
	cout << "Elaborating" << endl;
	execs.reserve(100);
	translations.clear();

	for (svector<program_execution>::iterator exec = execs.begin(); exec != execs.end(); exec++)
	{
		for (svector<program_counter>::iterator pc = exec->pcs.begin(); pc != exec->pcs.end(); pc++)
		{
			cout << "Reset ";
			for (int i = 0; i < pc->net->M0.size(); i++)
				cout << pc->net->M0[i] << " ";
			cout << endl;
			pc->net->print_dot(&cout, pc->name);
			if (pc->elaborate && nets.find(pc->net) == nets.end())
				nets.push_back(pc->net);
			for (svector<program_counter>::iterator pcj = exec->pcs.begin(); pcj != exec->pcs.end(); pcj++)
				if (pcj != pc && (pcj->net != pc->net || pcj->name != pc->name))
				{
					gen_translation(pcj->name, pcj->net, pc->name, pc->net);
					pc->set(translate(pcj->name, pcj->net, pcj->state, pc->name, pc->net));
				}

			for (svector<remote_program_counter>::iterator pcj = exec->rpcs.begin(); pcj != exec->rpcs.end(); pcj++)
			{
				gen_translation(pcj->name, pcj->net, pc->name, pc->net);
				gen_translation(pc->name, pc->net, pcj->name, pcj->net);
				pc->set(translate(pcj->name, pcj->net, pcj->net->vars->reset.terms.front(), pc->name, pc->net));
			}
		}

		for (svector<remote_program_counter>::iterator pc = exec->rpcs.begin(); pc != exec->rpcs.end(); pc++)
		{
			cout << "Reset ";
			for (int i = 0; i < pc->net->M0.size(); i++)
				cout << pc->net->M0[i] << " ";
			cout << endl;
			pc->net->print_dot(&cout, pc->name);
		}
	}

	for (svector<petri_net*>::iterator net = nets.begin(); net != nets.end(); net++)
		for (int i = 0; i < (*net)->S.size(); i++)
			(*net)->S[i].index = canonical(0);

	int number_processed = 0;
	int max_width = 0;
	while (execs.size() > 0)
	{
		if (execs.size() > max_width)
			max_width = execs.size();

		program_execution current_execution = execs.back();
		program_execution *exec = &current_execution;
		execs.pop_back();

		if ((number_processed/1000) != ((number_processed-1)/1000))
			cout << "Execution " << number_processed << " " << execs.size() << endl;

		while (!exec->done() && !exec->deadlock)
		{
			/**********************************
			 * Handle remote program counters *
			 **********************************/
			bool level1 = true;
			int rpc = 0;
			int idx = 0;
			while (exec->rpcs.size() > 0 && level1)
			{
				level1 = false;

				if (remote_begin_ready(exec, rpc, idx))
				{
					level1 = true;

					svector<remote_petri_index> ia = exec->rpcs[rpc].input_nodes(exec->rpcs[rpc].begin[idx]);
					svector<remote_petri_index> oa = exec->rpcs[rpc].output_nodes(exec->rpcs[rpc].begin[idx]);

					if (oa.size() == 0)
						exec->rpcs[rpc].begin.erase(exec->rpcs[rpc].begin.begin() + idx);
					else
					{
						for (svector<remote_petri_index>::iterator i = oa.begin(); i != oa.end(); i++)
						{
							/*cout << "-" << exec->rpcs[rpc].name << "." << *i << " [" << exec->rpcs[rpc].firings().print(exec->rpcs[rpc].net->vars) << "] (";
							for (int i = 0; i < exec->rpcs[rpc].arcs.size(); i++)
								cout << exec->rpcs[rpc].arcs[i].first << "->" << exec->rpcs[rpc].arcs[i].second << " ";
							cout << ")" << endl;*/
							exec->rpcs[rpc].begin.push_back(*i);
						}

						if (oa.size() > 0 || exec->rpcs[rpc].end.size() == 0)
						{
							for (svector<remote_petri_arc>::iterator arc = exec->rpcs[rpc].arcs.begin(); ia.size() == 0 && arc != exec->rpcs[rpc].arcs.end(); )
							{
								if (arc->first == exec->rpcs[rpc].begin[idx])
									arc = exec->rpcs[rpc].arcs.erase(arc);
								else
									arc++;
							}

							exec->rpcs[rpc].begin.erase(exec->rpcs[rpc].begin.begin() + idx);
						}
						else
						{
							cerr << "Error! " << exec->rpcs[rpc].end.size() << " " << exec->rpcs[rpc].end.front() << endl;
							for (svector<remote_petri_arc>::iterator arc = exec->rpcs[rpc].arcs.begin(); arc != exec->rpcs[rpc].arcs.end(); arc++)
								cout << "(" << arc->first << "->" << arc->second << ") ";
							cout << endl;
						}
					}
				}
			}

			level1 = true;
			rpc = 0;
			idx = 0;
			while (exec->rpcs.size() > 0 && level1)
			{
				level1 = false;

				svector<petri_index> oa;
				if (remote_end_ready(exec, rpc, idx, oa))
				{
					level1 = true;

					if (oa.size() == 0)
					{
						cerr << "Error: Remote net has an acyclic component at " << exec->rpcs[rpc].end[idx] << "." << endl;
						exec->rpcs[rpc].end.erase(exec->rpcs[rpc].end.begin() + idx);
					}
					else
					{
						for (int i = oa.size()-1; i >= 0; i--)
						{
							/*cout << "+" << exec->rpcs[rpc].name << "." << oa[i] << " {" << exec->rpcs[rpc].net->at(oa[i]).index.print(exec->rpcs[rpc].net->vars) << "} [" << exec->rpcs[rpc].firings().print(exec->rpcs[rpc].net->vars) << "] (";
							for (int i = 0; i < exec->rpcs[rpc].arcs.size(); i++)
								cout << exec->rpcs[rpc].arcs[i].first << "->" << exec->rpcs[rpc].arcs[i].second << " ";
							cout << ")" << endl;*/

							program_execution *cexec = exec;
							int cidx = idx;

							if (i != 0)
							{
								if (cexec->rpcs[rpc].end[cidx].is_place())
									duplicate_execution(cexec, &cexec);
								else
								{
									cexec->rpcs[rpc].end.push_back(cexec->rpcs[rpc].end[cidx]);
									cidx = cexec->rpcs[rpc].end.size()-1;
								}
							}

							remote_petri_index new_node(oa[i], cexec->rpcs[rpc].nid(oa[i]));
							cexec->rpcs[rpc].arcs.push_back(remote_petri_arc(cexec->rpcs[rpc].end[cidx], new_node));
							cexec->rpcs[rpc].end[cidx] = new_node;
						}
					}
				}
			}

			/*cout << "State:" << endl;
			for (int j = 0; j < exec->rpcs.size(); j++)
			{
				cout << exec->rpcs[j].name << "{";
				for (int k = 0; k < exec->rpcs[j].arcs.size(); k++)
					cout << exec->rpcs[j].arcs[k].first << "->" << exec->rpcs[j].arcs[k].second << " ";
				cout << "} " << exec->rpcs[j].firings().print(exec->rpcs[j].net->vars) << endl;
			}*/
			for (int j = 0; j < exec->rpcs.size(); j++)
				for (int i = 0; i < exec->pcs.size(); i++)
					exec->pcs[i].apply(translate(exec->rpcs[j].name, exec->rpcs[j].net, exec->rpcs[j].firings(), exec->pcs[i].name, exec->pcs[i].net));

			/*********************************
			 * Handle local program counters *
			 *********************************/
			svector<int> transitions_ready;
			svector<int> places_ready;

			for (int pc = 0; pc < exec->pcs.size(); pc++)
			{
				if (transitions_ready.size() == 0 && exec->pcs[pc].index.is_place() && exec->pcs[pc].next_has_active_or_satisfied())
					places_ready.push_back(pc);
				else if (exec->pcs[pc].index.is_trans())
				{
					int incoming_size = exec->pcs[pc].input_nodes().size();
					if (incoming_size == 1)
						transitions_ready.push_back(pc);
					else if (exec->count(pc) == incoming_size)	// Parallel Merge
						transitions_ready.push_back(exec->merge(pc));
				}
			}

			/*for (int i = 0; i < transitions_ready.size(); i++)
				cout << exec->pcs[transitions_ready[i]].name << ":" << exec->pcs[transitions_ready[i]].index << "{" << exec->pcs[transitions_ready[i]].state.print(exec->pcs[transitions_ready[i]].net->vars) << "} ";
			for (int i = 0; i < places_ready.size(); i++)
				cout << exec->pcs[places_ready[i]].name << ":" << exec->pcs[places_ready[i]].index << "{" << exec->pcs[places_ready[i]].state.print(exec->pcs[places_ready[i]].net->vars) << "} ";
			cout << endl;*/

			if (transitions_ready.size() > 0)
			{
				for (int i = transitions_ready.size()-1; i >= 0; i--)
				{
					int pc = transitions_ready[i];

					svector<petri_index> outgoing = exec->pcs[pc].output_nodes();
					for (int j = outgoing.size()-1; j >= 0; j--)
					{
						int cpc = pc;
						// Create a new program counter for parallel splits
						if (j > 0)
							duplicate_counter(exec, cpc, cpc);

						//cout << ">" << exec->pcs[cpc].name << "." << outgoing[j] << " {" << exec->pcs[cpc].predicate().print(exec->pcs[cpc].net->vars) << "} [" << exec->pcs[cpc].state.print(exec->pcs[cpc].net->vars) << "]" << endl;

						if (exec->pcs[cpc].is_active())
						{
							svector<minterm> passing = exec->pcs[cpc].predicate().terms;
							for (int t = passing.size()-1; t >= 0; t--)
							{
								program_execution *texec = exec;
								if (t > 0)
									duplicate_execution(texec, &texec);

								texec->pcs[cpc].state = texec->pcs[cpc].state >> passing[t];
								for (int apc = 0; apc < texec->pcs.size(); apc++)
									if (apc != cpc)
										texec->pcs[apc].apply(translate(texec->pcs[cpc].name, texec->pcs[cpc].net, passing[t], texec->pcs[apc].name, texec->pcs[apc].net));

								texec->pcs[cpc].index = outgoing[j];
							}
						}
						else
						{
							svector<minterm> passing;
							for (int t = 0; t < exec->pcs[cpc].predicate().terms.size(); t++)
								if ((exec->pcs[cpc].predicate().terms[t] & exec->pcs[cpc].state) != 0)
									passing.push_back(exec->pcs[cpc].predicate().terms[t]);

							for (int t = passing.size()-1; t >= 0 ; t--)
							{
								program_execution *texec = exec;
								if (t > 0)
									duplicate_execution(texec, &texec);

								texec->pcs[cpc].state &= passing[t];
								texec->pcs[cpc].index = outgoing[j];
							}
						}
					}
				}
			}
			else if (places_ready.size() > 0)
			{
				// We want to create a new program execution for every possible ordering of events.
				//if (places_ready.size() > 1)
				//	cout << "Order split " << places_ready.size() << endl;
				for (int i = places_ready.size()-1; i >= 0; i--)
					for (int j = i-1; exec->pcs[places_ready[i]].elaborate && j >= 0; j--)
						if (!exec->pcs[places_ready[j]].elaborate)
						{
							int temp = places_ready[i];
							places_ready[i] = places_ready[j];
							places_ready[j] = temp;
						}

				for (int i = 0; i < places_ready.size(); i++)
				{
					program_execution *oexec = exec;
					int pc = places_ready[i];

					if (i < places_ready.size()-1)
						duplicate_execution(oexec, &oexec);

					if (oexec->pcs[pc].elaborate)
					{
						canonical old = oexec->pcs[pc].net->at(oexec->pcs[pc].index).index;
						oexec->pcs[pc].net->at(oexec->pcs[pc].index).index.push_back(oexec->pcs[pc].state);
						oexec->pcs[pc].net->at(oexec->pcs[pc].index).index.mccluskey();

						oexec->pcs[pc].done = false;
						if (oexec->pcs[pc].net->at(oexec->pcs[pc].index).index == old)
							oexec->pcs[pc].done = true;
					}

					if (!oexec->done())
					{
						svector<petri_index> outgoing = oexec->pcs[pc].output_nodes();
						for (int j = 0; j < outgoing.size(); )
						{
							if (!oexec->pcs[pc].is_active(outgoing[j]) && !oexec->pcs[pc].is_satisfied(outgoing[j]))
								outgoing.erase(outgoing.begin() + j);
							else
								j++;
						}

						//if (outgoing.size() > 1)
						//	cout << "Conditional split at " << oexec->pcs[pc].name << ":" << oexec->pcs[pc].net->node_name(oexec->pcs[pc].index) << " " << outgoing.size() << endl;

						for (int j = outgoing.size()-1; j >= 0; j--)
						{
							//cout << ">" << oexec->pcs[pc].name << "." << outgoing[j] << " [" << oexec->pcs[pc].state.print(oexec->pcs[pc].net->vars) << "]" << endl;
							program_execution *cexec = oexec;
							if (j > 0)
								duplicate_execution(cexec, &cexec);

							cexec->pcs[pc].index = outgoing[j];
						}
					}
				}
			}
			else
			{
				cerr << "Error: Deadlock detected ";
				for (svector<program_counter>::iterator pc = exec->pcs.begin(); pc != exec->pcs.end(); pc++)
					cerr << pc->name << ":" << pc->index.name() << "{" << pc->state.print(pc->net->vars) << "}" << endl;
				for (svector<remote_program_counter>::iterator pc = exec->rpcs.begin(); pc != exec->rpcs.end(); pc++)
				{
					cerr << pc->name << ":{";
					for (int i = 0; i < pc->begin.size(); i++)
						cerr << pc->begin[i] << " ";
					cerr << "}=>{";
					for (int i = 0; i < pc->end.size(); i++)
						cerr << pc->end[i] << " ";
					cerr << "} " << pc->firings().print(pc->net->vars) << " ";
				}
				cerr << endl;
				exec->deadlock = true;
			}
		}

		number_processed++;
	}

	for (svector<petri_net*>::iterator net = nets.begin(); net != nets.end(); net++)
	{
		for (int i = 0; i < (*net)->T.size(); i++)
		{
			svector<petri_index> ia = (*net)->prev(petri_index(i, false));

			canonical t = 1;
			for (int l = 0; l < ia.size(); l++)
				t &= (*net)->at(ia[l]).index;

			canonical r;
			if ((*net)->T[i].active)
				r = (t >> (*net)->T[i].index);
			else
				r = (t & (*net)->T[i].index);

			(*net)->T[i].definitely_invacuous = is_mutex(&t, &r);
			(*net)->T[i].possibly_vacuous = !(*net)->T[i].definitely_invacuous;
			(*net)->T[i].definitely_vacuous = (t == r);
		}
	}

	cout << "Executions: " << number_processed << " " << max_width << endl;
}

void program_execution_space::reset()
{
	execs.clear();
	nets.clear();
}

void program_execution_space::gen_translation(string name0, petri_net *net0, string name1, petri_net *net1)
{
	cout << "Translation from " << name0 << " to " << name1 << endl;
	svector<pair<int, int> > factors;
	for (smap<sstring, variable>::iterator vi = net0->vars->global.begin(); vi != net0->vars->global.end(); vi++)
	{
		string test = vi->second.name;
		if (test.substr(0, 5) == "this.")
			test = test.substr(5);

		if (name0 != "")
			test = name0 + "." + test;

		int uid = -1;
		if (name1.length() == 0)
			uid = net1->vars->get_uid(test);
		else if (test.substr(0, name1.length()) == name1)
			uid = net1->vars->get_uid(test.substr(name1.length()+1));

		cout << "Looking for " << test << "{" << vi->second.uid << "} and found " << net1->vars->get_name(uid) << "{" << uid << "}" << endl;

		if (uid != -1)
			factors.push_back(pair<int, int>(vi->second.uid, uid));
	}

	translations.insert(
		pair<pair<pair<string, petri_net*>, pair<string, petri_net*> >, svector<pair<int, int> > >(
			pair<pair<string, petri_net*>, pair<string, petri_net*> >(
				pair<string, petri_net*>(name0, net0),
				pair<string, petri_net*>(name1, net1)
			),
			factors
		)
	);
}

minterm program_execution_space::translate(string name0, petri_net *net0, minterm t, string name1, petri_net *net1)
{
	if (name0 != name1 || net0 != net1)
	{
		return t.refactor(
			translations[
				pair<pair<string, petri_net*>, pair<string, petri_net*> >(
					pair<string, petri_net*>(name0, net0),
					pair<string, petri_net*>(name1, net1)
				)
			]
		);
	}
	else
		return t;
}
