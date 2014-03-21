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
	this->n = net->next(index);
	this->p = net->prev(index);
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

program_execution &program_execution::operator=(program_execution e)
{
	pcs = e.pcs;
	rpcs = e.rpcs;
	deadlock = e.deadlock;
	return *this;
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

bool program_execution_space::remote_end_ready(program_execution *exec, int rpc, int &idx, svector<petri_index> &outgoing, minterm state)
{
	outgoing.clear();
	if (idx >= exec->rpcs[rpc].end.size())
		idx = 0;

	int end = idx;
	bool first = true;
	while (first || idx != end)
	{
		first = false;
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
		if (idx >= exec->rpcs[rpc].end.size())
			idx = 0;
	}

	return false;
}

bool program_execution_space::remote_begin_ready(program_execution *exec, int rpc, int &idx, minterm state)
{
	if (idx >= exec->rpcs[rpc].begin.size())
		idx = 0;

	int end = idx;
	bool first = true;
	while (first || idx != end)
	{
		first = false;
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
		if (idx >= exec->rpcs[rpc].begin.size())
			idx = 0;
	}

	return false;
}

void program_execution_space::full_elaborate()
{
	cout << "Elaborating" << endl;
	translations.clear();

	for (list<program_execution>::iterator exec = execs.begin(); exec != execs.end(); exec++)
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
	int remote = 0;
	int ordering = 0;
	int csplit = 0;
	int guards = 0;
	int total_iterations = 0;
	petri_state seen;
	while (execs.size() > 0)
	{
		if (execs.size() > max_width)
			max_width = execs.size();

		program_execution current_execution;

		if (execs.size() > 100000)
		{
			current_execution = execs.back();
			execs.pop_back();
		}
		else
		{
			current_execution = execs.front();
			execs.pop_front();
		}
		program_execution *exec = &current_execution;

		if ((number_processed/1000) != ((number_processed-1)/1000))
		{
			cout << "Execution " << number_processed << " " << execs.size() << " " << remote << " " << ordering << " " << csplit << " " << guards << " " << total_iterations << " " << seen << endl;

			remote = 0;
			ordering = 0;
			csplit = 0;
			guards = 0;
			total_iterations = 0;
			seen.state.clear();
		}

		if ((number_processed/10000) != ((number_processed-1)/10000))
			exec->pcs[0].net->print_dot(&cout, "graph");

		while (!exec->done() && !exec->deadlock)
		{
			total_iterations++;
			/**********************************
			 * Handle remote program counters *
			 **********************************/
			for (int pc = 0; pc < exec->rpcs.size(); pc++)
			{
				minterm state;
				for (int i = 0; i < exec->pcs.size(); i++)
					state &= translate(exec->pcs[i].name, exec->pcs[i].net, exec->pcs[i].state, exec->rpcs[pc].name, exec->rpcs[pc].net);

				int idx = 0;
				svector<petri_index> outgoing;
				while (remote_begin_ready(exec, pc, idx, state))
				{
					svector<remote_petri_index> ia = exec->rpcs[pc].input_nodes(exec->rpcs[pc].begin[idx]);
					svector<remote_petri_index> oa = exec->rpcs[pc].output_nodes(exec->rpcs[pc].begin[idx]);

					/*cout << "-" << exec->rpcs[pc].name << "." << exec->rpcs[pc].begin[idx] << " {" << exec->rpcs[pc].net->at(exec->rpcs[pc].begin[idx]).index.print(exec->rpcs[pc].net->vars) << "} [" << exec->rpcs[pc].firings().print(exec->rpcs[pc].net->vars) << "] (";
					for (int i = 0; i < exec->rpcs[pc].arcs.size(); i++)
						cout << exec->rpcs[pc].arcs[i].first << "->" << exec->rpcs[pc].arcs[i].second << " ";
					cout << ")" << endl;*/

					if (oa.size() == 0)
						exec->rpcs[pc].begin.erase(exec->rpcs[pc].begin.begin() + idx);
					else
					{
						for (svector<remote_petri_index>::iterator i = oa.begin(); i != oa.end(); i++)
							exec->rpcs[pc].begin.push_back(*i);

						for (svector<remote_petri_arc>::iterator arc = exec->rpcs[pc].arcs.begin(); ia.size() == 0 && arc != exec->rpcs[pc].arcs.end(); )
						{
							if (arc->first == exec->rpcs[pc].begin[idx])
								arc = exec->rpcs[pc].arcs.erase(arc);
							else
								arc++;
						}

						exec->rpcs[pc].begin.erase(exec->rpcs[pc].begin.begin() + idx);
					}
				}

				idx = 0;
				while (remote_end_ready(exec, pc, idx, outgoing, state))
				{
					if (outgoing.size() == 0)
					{
						cerr << "Error: Remote net has an acyclic component at " << exec->rpcs[pc].end[idx] << "." << endl;
						exec->rpcs[pc].end.erase(exec->rpcs[pc].end.begin() + idx);
					}
					else
					{
						if (exec->rpcs[pc].end[idx].is_trans() && exec->rpcs[pc].net->at(exec->rpcs[pc].end[idx]).active && exec->rpcs[pc].net->at(exec->rpcs[pc].end[idx]).index.terms.size() > 0)
							state = state >> exec->rpcs[pc].net->at(exec->rpcs[pc].end[idx]).index.terms.back();

						if (outgoing.size() > 1)
							remote += outgoing.size()-1;

						for (int i = outgoing.size()-1; i >= 0; i--)
						{
							/*cout << "+" << exec->rpcs[pc].name << "." << outgoing[i] << " {" << exec->rpcs[pc].net->at(outgoing[i]).index.print(exec->rpcs[pc].net->vars) << "} [" << exec->rpcs[pc].firings().print(exec->rpcs[pc].net->vars) << "] (";
							for (int i = 0; i < exec->rpcs[pc].arcs.size(); i++)
								cout << exec->rpcs[pc].arcs[i].first << "->" << exec->rpcs[pc].arcs[i].second << " ";
							cout << ")" << endl;*/

							program_execution *cexec = exec;
							int cidx = idx;

							if (i > 0)
							{
								if (cexec->rpcs[pc].end[cidx].is_place())
									duplicate_execution(cexec, &cexec);
								else
								{
									cexec->rpcs[pc].end.push_back(cexec->rpcs[pc].end[cidx]);
									cidx = cexec->rpcs[pc].end.size()-1;
								}
							}

							remote_petri_index new_node(outgoing[i], cexec->rpcs[pc].nid(outgoing[i]));
							cexec->rpcs[pc].arcs.push_back(remote_petri_arc(cexec->rpcs[pc].end[cidx], new_node));
							cexec->rpcs[pc].end[cidx] = new_node;
						}
					}
				}
			}

			/*cout << "{";
			for (int j = 0; j < exec->pcs.size(); j++)
				cout << exec->pcs[j].index;
			cout << "}" << endl;*/

			for (int j = 0; j < exec->rpcs.size(); j++)
				for (int i = 0; i < exec->pcs.size(); i++)
					exec->pcs[i].apply(translate(exec->rpcs[j].name, exec->rpcs[j].net, exec->rpcs[j].firings(), exec->pcs[i].name, exec->pcs[i].net));

			/*********************************
			 * Handle local program counters *
			 *********************************/
			int transition_ready = -1;
			svector<int> places_ready;

			for (int pc = exec->pcs.size()-1; transition_ready == -1 && pc >= 0; pc--)
			{
				if (exec->pcs[pc].index.is_trans())
				{
					if (exec->pcs[pc].p.size() == 1)
						transition_ready = pc;
					else
					{
						int total = 1;
						for (int pcj = pc+1; pcj < exec->pcs.size(); pcj++)
							if (exec->pcs[pcj] == exec->pcs[pc])
								total++;

						if (total == exec->pcs[pc].p.size())
						{
							for (int pcj = pc+1; pcj < exec->pcs.size(); )
							{
								if (exec->pcs[pcj] == exec->pcs[pc])
								{
									exec->pcs[pc].state &= exec->pcs[pcj].state;
									exec->pcs.erase(exec->pcs.begin() + pcj);
								}
								else
									pcj++;
							}

							transition_ready = pc;
						}
					}
				}
			}

			if (transition_ready == -1)
			{
				for (int pc = 0; pc < exec->pcs.size(); pc++)
				{
					if (exec->pcs[pc].index.is_place())
					{
						svector<petri_index> rn;
						for (int i = 0; i < exec->pcs[pc].n.size(); i++)
							if (exec->pcs[pc].net->at(exec->pcs[pc].n[i]).active || exec->pcs[pc].is_satisfied(exec->pcs[pc].n[i]))
								rn.push_back(exec->pcs[pc].n[i]);

						if (rn.size() > 0)
						{
							exec->pcs[pc].n = rn;
							places_ready.push_back(pc);
						}
					}
				}
			}

			/*for (int i = 0; i < transitions_ready.size(); i++)
				cout << exec->pcs[transitions_ready[i]].name << ":" << exec->pcs[transitions_ready[i]].index << "{" << exec->pcs[transitions_ready[i]].state.print(exec->pcs[transitions_ready[i]].net->vars) << "} ";
			for (int i = 0; i < places_ready.size(); i++)
				cout << exec->pcs[places_ready[i]].name << ":" << exec->pcs[places_ready[i]].index << "{" << exec->pcs[places_ready[i]].state.print(exec->pcs[places_ready[i]].net->vars) << "} ";
			cout << endl;*/

			if (transition_ready != -1)
			{
				svector<minterm> passing;
				if (exec->pcs[transition_ready].is_active())
					passing = exec->pcs[transition_ready].predicate().terms;
				else
					for (int t = 0; t < exec->pcs[transition_ready].predicate().terms.size(); t++)
						if ((exec->pcs[transition_ready].predicate().terms[t] & exec->pcs[transition_ready].state) != 0)
							passing.push_back(exec->pcs[transition_ready].predicate().terms[t]);

				if (passing.size() > 1)
					guards += passing.size()-1;

				for (int t = passing.size()-1; t >= 0; t--)
				{
					program_execution *texec = exec;
					if (t > 0)
						duplicate_execution(texec, &texec);

					if (texec->pcs[transition_ready].is_active())
					{
						texec->pcs[transition_ready].state = texec->pcs[transition_ready].state >> passing[t];
						for (int apc = 0; apc < texec->pcs.size(); apc++)
							if (apc != transition_ready)
								texec->pcs[apc].apply(translate(texec->pcs[transition_ready].name, texec->pcs[transition_ready].net, passing[t], texec->pcs[apc].name, texec->pcs[apc].net));
					}
					else
						texec->pcs[transition_ready].state &= passing[t];

					for (int j = texec->pcs[transition_ready].n.size()-1; j >= 0; j--)
					{
						int cpc = transition_ready;
						// Create a new program counter for parallel splits
						if (j > 0)
							duplicate_counter(texec, cpc, cpc);

						texec->pcs[cpc].index = texec->pcs[cpc].n[j];
						texec->pcs[cpc].n.clear();
						texec->pcs[cpc].p.clear();
						for (int i = 0; i < texec->pcs[cpc].net->arcs.size(); i++)
						{
							if (texec->pcs[cpc].net->arcs[i].first == texec->pcs[cpc].index)
								texec->pcs[cpc].n.push_back(texec->pcs[cpc].net->arcs[i].second);
							if (texec->pcs[cpc].net->arcs[i].second == texec->pcs[cpc].index)
								texec->pcs[cpc].p.push_back(texec->pcs[cpc].net->arcs[i].first);
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

				if (places_ready.size() > 1)
					ordering += places_ready.size()-1;

				svector<bool> new_done;
				for (int i = 0; i < places_ready.size(); )
				{
					int pc = places_ready[i];
					bool old_done = exec->pcs[pc].done;
					if (exec->pcs[pc].elaborate)
					{
						canonical old = exec->pcs[pc].net->at(exec->pcs[pc].index).index;
						exec->pcs[pc].net->at(exec->pcs[pc].index).index.push_back(exec->pcs[pc].state);
						exec->pcs[pc].net->at(exec->pcs[pc].index).index.mccluskey_or(exec->pcs[pc].net->at(exec->pcs[pc].index).index.terms.size()-1);

						exec->pcs[pc].done = false;
						if (exec->pcs[pc].net->at(exec->pcs[pc].index).index == old)
							exec->pcs[pc].done = true;
					}

					if (exec->done())
						places_ready.erase(places_ready.begin() + i);
					else
					{
						i++;
						new_done.push_back(exec->pcs[pc].done);
					}

					exec->pcs[pc].done = old_done;
				}

				if (places_ready.size() == 0)
					for (int i = 0; i < exec->pcs.size(); i++)
						exec->pcs[i].done = true;

				for (int i = 0; i < places_ready.size(); i++)
				{
					seen.state.push_back(exec->pcs[places_ready[i]].index);
					seen.state.unique();
					program_execution *oexec = exec;
					int pc = places_ready[i];

					if (i < places_ready.size()-1)
						duplicate_execution(oexec, &oexec);

					oexec->pcs[pc].done = new_done[i];

					if (oexec->pcs[pc].n.size() > 1)
						csplit += oexec->pcs[pc].n.size()-1;

					for (int j = oexec->pcs[pc].n.size()-1; j >= 0; j--)
					{
						program_execution *cexec = oexec;
						if (j > 0)
							duplicate_execution(cexec, &cexec);

						cexec->pcs[pc].index = cexec->pcs[pc].n[j];
						cexec->pcs[pc].n.clear();
						cexec->pcs[pc].p.clear();
						for (int i = 0; i < cexec->pcs[pc].net->arcs.size(); i++)
						{
							if (cexec->pcs[pc].net->arcs[i].first == cexec->pcs[pc].index)
								cexec->pcs[pc].n.push_back(cexec->pcs[pc].net->arcs[i].second);
							if (cexec->pcs[pc].net->arcs[i].second == cexec->pcs[pc].index)
								cexec->pcs[pc].p.push_back(cexec->pcs[pc].net->arcs[i].first);
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
