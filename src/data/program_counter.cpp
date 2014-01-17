/*
 * program_counter.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: nbingham
 */

#include "program_counter.h"
#include "petri.h"
#include "variable_space.h"

umap::umap()
{

}

umap::umap(const umap &um)
{
	arcs = um.arcs;
	begin = um.begin;
	end = um.end;
	tids = um.tids;
	sids = um.sids;
	iac = um.iac;
}

umap::~umap()
{

}

list<uarc> umap::input_arcs(unode n)
{
	list<uarc> result;
	for (list<uarc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->second == n)
			result.push_back(*arc);
	return result;
}

list<uarc> umap::output_arcs(unode n)
{
	list<uarc> result;
	for (list<uarc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->first == n)
			result.push_back(*arc);
	return result;
}

list<unode> umap::input_nodes(unode n)
{
	list<unode> result;
	for (list<uarc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->second == n)
			result.push_back(arc->first);
	return result;
}

list<unode> umap::output_nodes(unode n)
{
	list<unode> result;
	for (list<uarc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->first == n)
			result.push_back(arc->second);
	return result;
}

bool umap::path_contains(unode n)
{
	for (list<uarc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->first == n || arc->second == n)
			return true;
	return false;
}

int umap::nid(int i)
{
	if (i > 0)
		return sids[i]++;
	else
	{
		i &= 0x7FFFFFFF;
		if (tids[i].second < iac[i])
		{
			tids[i].second++;
			return tids[i].first;
		}
		else
		{
			tids[i].second = 0;
			return tids[i].first++;
		}
	}
}

umap &umap::operator=(umap um)
{
	arcs = um.arcs;
	begin = um.begin;
	end = um.end;
	tids = um.tids;
	sids = um.sids;
	iac = um.iac;
	return *this;
}

remote_program_counter::remote_program_counter()
{
	this->name = "";
	this->net = NULL;
}

remote_program_counter::remote_program_counter(const remote_program_counter &pc)
{
	name = pc.name;
	net = pc.net;
	forward_factors = pc.forward_factors;
	reverse_factors = pc.reverse_factors;
	hidden_factors = pc.hidden_factors;
	index = pc.index;
}

remote_program_counter::remote_program_counter(string name, petri *from, petri *to)
{
	this->name = name;
	net = to;

	index.sids.resize(net->S.size(), 0);
	index.tids.resize(net->T.size(), pair<int, int>(0, 0));
	index.iac.resize(net->T.size(), 0);
	for (int i = 0; i < net->T.size(); i++)
		index.iac[i] = net->input_arcs(net->trans_id(i)).size();

	for (svector<int>::iterator i = net->M0.begin(); i != net->M0.end(); i++)
	{
		int id = index.nid(*i);
		index.begin.push_back(unode(*i, id));
		index.end.push_back(pair<unode, list<unode> >(unode(*i, id), list<unode>(1, unode(*i, id))));
	}
	// translation
	for (smap<sstring, variable>::iterator vi = from->vars->global.begin(); vi != from->vars->global.end(); vi++)
	{
		if (name.length() == 0)
		{
			if (!vi->second.driven)
				hidden_factors.push_back(vi->second.uid);

			forward_factors.push_back(pair<int, int>(vi->second.uid, net->vars->get_uid(vi->second.name)));
			reverse_factors.push_back(pair<int, int>(forward_factors.back().second, forward_factors.back().first));
		}
		else if (vi->second.name.find(name) != vi->second.name.npos)
		{
			if (!vi->second.driven)
				hidden_factors.push_back(vi->second.uid);

			forward_factors.push_back(pair<int, int>(vi->second.uid, net->vars->get_uid(vi->second.name.substr(name.length()+1))));
			reverse_factors.push_back(pair<int, int>(forward_factors.back().second, forward_factors.back().first));
		}
		else
			hidden_factors.push_back(vi->second.uid);
	}
}

remote_program_counter::remote_program_counter(int index, petri *net)
{
	this->name = "this";
	this->net = net;
	this->index.sids.resize(net->S.size(), 0);
	this->index.tids.resize(net->T.size(), pair<int, int>(0, 0));
	this->index.iac.resize(net->T.size(), 0);
	for (int i = 0; i < net->T.size(); i++)
		this->index.iac[i] = net->input_arcs(net->trans_id(i)).size();

	int id = this->index.nid(index);
	this->index.begin.push_back(unode(index, id));
	this->index.end.push_back(pair<unode, list<unode> >(unode(index, id), list<unode>(1, unode(index, id))));

	for (smap<sstring, variable>::iterator vi = net->vars->global.begin(); vi != net->vars->global.end(); vi++)
	{
		forward_factors.push_back(pair<int, int>(vi->second.uid, vi->second.uid));
		reverse_factors.push_back(pair<int, int>(vi->second.uid, vi->second.uid));
	}
}

remote_program_counter::~remote_program_counter()
{

}

logic &remote_program_counter::net_index(int i)
{
	return (*net)[i].index;
}

bool remote_program_counter::is_place(int i)
{
	return net->is_place(i);
}

bool remote_program_counter::is_trans(int i)
{
	return net->is_trans(i);
}

bool remote_program_counter::is_active(int i)
{
	return (*net)[i].active;
}

bool remote_program_counter::is_satisfied(int i, logic s)
{
	return !is_mutex(&(*net)[i].index, &s);
}

bool remote_program_counter::is_vacuous(int i, logic s)
{
	logic temp = ~(*net)[i].index;
	return is_mutex(&temp, &s);
}

bool remote_program_counter::is_one(int i)
{
	return ((*net)[i].index == 1);
}

void remote_program_counter::roll_to(unode idx)
{
	list<unode> iter(1, idx);

	cout << endl;
	for (list<uarc>::iterator i = index.arcs.begin(); i != index.arcs.end(); i++)
		cout << "(" << net->node_name(i->first.first) << "." << i->first.second << "->" << net->node_name(i->second.first) << "." << i->second.second << ") ";
	cout << endl;

	for (list<unode>::iterator n = iter.begin(); n != iter.end(); n = iter.erase(n))
	{
		cout << iter.size() << " ";
		for (list<uarc>::iterator a = index.arcs.begin(); a != index.arcs.end();)
		{
			if (a->second == *n)
			{
				iter.push_back(a->first);
				a = index.arcs.erase(a);
			}
			else
				a++;
		}

		index.begin.remove(*n);
	}

	index.begin.push_back(idx);
}

int remote_program_counter::count(unode n)
{
	int total = 0;
	for (list<pair<unode, list<unode> > >::iterator j = index.end.begin(); j != index.end.end(); j++)
		if (j->first == n)
			total++;
	return total;
}

minterm remote_program_counter::firings()
{
	minterm result;
	list<int> firing_nodes;
	for (list<uarc>::iterator i = index.arcs.begin(); i != index.arcs.end(); i++)
	{
		if (is_trans(i->second.first) && is_active(i->second.first))
			firing_nodes.push_back(i->second.first);
		else if (is_trans(i->first.first) && is_active(i->first.first))
			firing_nodes.push_back(i->first.first);
	}

	firing_nodes.sort();
	firing_nodes.unique();

	for (list<int>::iterator i = firing_nodes.begin(); i != firing_nodes.end(); i++)
		for (svector<minterm>::iterator j = net_index(*i).terms.begin(); j != net_index(*i).terms.end(); j++)
			result &= *j;

	return result;
}

logic remote_program_counter::waits(unode n)
{
	list<unode> iter(1, n);
	list<int> waiting_nodes;
	for (list<unode>::iterator end = iter.begin(); end != iter.end(); end = iter.erase(end))
	{
		if (is_trans(end->first) && !is_active(end->first))
			waiting_nodes.push_back(end->first);

		list<unode> inputs = index.input_nodes(*end);
		iter.insert(iter.end(), inputs.begin(), inputs.end());
	}

	waiting_nodes.sort();
	waiting_nodes.unique();

	logic result;
	bool set = false;
	for (list<int>::iterator i = waiting_nodes.begin(); i != waiting_nodes.end(); i++)
	{
		if (!set)
		{
			result = net_index(*i);
			set = true;
		}
		else
			result &= net_index(*i);
	}

	return result;
}

remote_program_counter &remote_program_counter::operator=(remote_program_counter pc)
{
	name = pc.name;
	net = pc.net;
	forward_factors = pc.forward_factors;
	reverse_factors = pc.reverse_factors;
	hidden_factors = pc.hidden_factors;
	index = pc.index;
	return *this;
}

program_environment::program_environment()
{
}

program_environment::program_environment(const remote_program_counter &pc)
{
	pcs.push_back(pc);
}

program_environment::program_environment(const program_environment &env)
{
	pcs.insert(pcs.end(), env.pcs.begin(), env.pcs.end());
}

program_environment::~program_environment()
{
}

minterm program_environment::firings()
{
	minterm result;
	for (list<remote_program_counter>::iterator pc = pcs.begin(); pc != pcs.end(); pc++)
	{
		list<int> firing_nodes;
		for (list<uarc>::iterator i = pc->index.arcs.begin(); i != pc->index.arcs.end(); i++)
		{
			if (pc->is_trans(i->second.first) && pc->is_active(i->second.first))
				firing_nodes.push_back(i->second.first);
			else if (pc->is_trans(i->first.first) && pc->is_active(i->first.first))
				firing_nodes.push_back(i->first.first);
		}

		firing_nodes.sort();
		firing_nodes.unique();

		for (list<int>::iterator i = firing_nodes.begin(); i != firing_nodes.end(); i++)
			for (svector<minterm>::iterator j = pc->net_index(*i).terms.begin(); j != pc->net_index(*i).terms.end(); j++)
				result &= j->refactor(pc->reverse_factors);
		firing_nodes.clear();
	}
	return result;
}

program_environment &program_environment::operator=(program_environment env)
{
	pcs = env.pcs;
	return *this;
}

program_counter::program_counter()
{
	net = NULL;
	index = 0;
	waiting = false;
}

program_counter::program_counter(const program_counter &pc)
{
	index = pc.index;
	net = pc.net;
	waiting = pc.waiting;
	envs = pc.envs;
	splits.clear();
	for (list<pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > > >::const_iterator pc_split = pc.splits.begin(); pc_split != pc.splits.end(); pc_split++)
	{
		list<pair<list<program_environment>::iterator, unode> > split_environments;
		for (list<pair<list<program_environment>::iterator, unode> >::const_iterator pc_prgm = pc_split->second.begin(); pc_prgm != pc_split->second.end(); pc_prgm++)
		{
			list<program_environment>::iterator env;
			list<program_environment>::const_iterator pc_env;
			for (env = envs.begin(), pc_env = pc.envs.begin(); pc_env != pc.envs.end() && pc_env != pc_prgm->first; pc_env++, env++);
			split_environments.push_back(pair<list<program_environment>::iterator, unode>(env, pc_prgm->second));
		}

		splits.push_back(pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > >(pc_split->first, split_environments));
	}
}

program_counter::program_counter(int index, petri *net)
{
	this->index = index;
	this->net = net;
	this->waiting = false;
}

program_counter::~program_counter()
{

}

bool program_counter::is_trans()
{
	return net->is_trans(index);
}

bool program_counter::is_place()
{
	return net->is_place(index);
}

bool program_counter::is_active()
{
	return (*net)[index].active;
}

string program_counter::node_name()
{
	return net->node_name(index);
}

bool program_counter::end_is_ready(logic s, list<program_environment>::iterator &env, list<remote_program_counter>::iterator &pc, list<pair<unode, list<unode> > >::iterator &idx, logic &state)
{
	for (env = envs.begin(); env != envs.end(); env++)
	{
		for (pc = env->pcs.begin(); pc != env->pcs.end(); pc++)
		{
			state = mute(s).hide(pc->hidden_factors).refactor(pc->forward_factors);

			for (idx = pc->index.end.begin(); idx != pc->index.end.end(); idx++)
			{
				if (pc->is_place(idx->first.first))
					return true;
				else if ((pc->is_trans(idx->first.first) && pc->is_active(idx->first.first)) || (pc->is_satisfied(idx->first.first, state) && pc->waits(idx->first) != 0))
				{
					// handle parallel merge
					svector<int> ia = pc->net->input_nodes(idx->first.first);

					if (pc->count(idx->first) == ia.size())
					{
						for (list<pair<unode, list<unode> > >::iterator j = pc->index.end.begin(); j != pc->index.end.end();)
						{
							if (j != idx && j->first == idx->first)
							{
								idx->second.insert(idx->second.end(), j->second.begin(), j->second.end());
								for (list<unode>::iterator ri = idx->second.begin(); ri != idx->second.end(); ri++)
									for (list<unode>::iterator rj = ri; rj != idx->second.end();)
									{
										if (ri != rj && *ri == *rj)
											rj = idx->second.erase(rj);
										else
											rj++;
									}

								j = pc->index.end.erase(j);
							}
							else
								j++;
						}

						if (!pc->is_active(idx->first.first) && !pc->is_one(idx->first.first))
						{
							cout << "<!>, ";
							idx->second.clear();
						}

						return true;
					}
				}
			}
		}
	}

	return false;
}

bool program_counter::begin_is_ready(logic s, list<program_environment>::iterator &env, list<remote_program_counter>::iterator &pc, list<unode>::iterator &idx, list<pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > > >::iterator &split, list<pair<list<program_environment>::iterator, unode> >::iterator &prgm, logic &state)
{
	split = splits.end();
	for (env = envs.begin(); env != envs.end(); env++)
	{
		for (pc = env->pcs.begin(); pc != env->pcs.end(); pc++)
		{
			state = mute(s).hide(pc->hidden_factors).refactor(pc->forward_factors);

			for (idx = pc->index.begin.begin(); idx != pc->index.begin.end();)
			{
				if (pc->is_place(idx->first) || (pc->is_trans(idx->first) && pc->is_active(idx->first)) || pc->is_vacuous(idx->first, state))
				{
					bool wall = false;
					for (list<pair<unode, list<unode> > >::iterator j = pc->index.end.begin(); j != pc->index.end.end() && !wall; j++)
						if (find(j->second.begin(), j->second.end(), *idx) != j->second.end() || *idx == j->first)
							wall = true;

					if (!wall)
					{
						for (split = splits.begin(); split != splits.end(); split++)
							if (split->first.first == pc->name && split->first.second == pc->net && (prgm = find(split->second.begin(), split->second.end(), pair<list<program_environment>::iterator, unode>(env, *idx))) != split->second.end())
								return true;

						return true;
					}

					idx++;
				}
				else
					idx++;
			}
		}
	}

	return false;
}

void program_counter::simulate(logic s)
{
	list<program_environment>::iterator env;
	list<remote_program_counter>::iterator pc;
	list<unode>::iterator begin_idx;
	list<pair<unode, list<unode> > >::iterator end_idx;
	list<pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > > >::iterator split;
	list<pair<list<program_environment>::iterator, unode> >::iterator this_prgm;

	bool done = false;
	while (!done)
	{
		done = true;
		bool changes = true;
		while (changes)
		{
			changes = false;
			logic state;
			if (end_is_ready(s, env, pc, end_idx, state))
			{
				changes = true;
				done = false;

				svector<int> oa = pc->net->output_nodes(end_idx->first.first);
				if (oa.size() == 0)
				{
					cerr << "Error: Remote net has an acyclic component at " << pc->net->node_name(end_idx->first.first) << "." << endl;
					pc->index.end.erase(end_idx);
				}
				else
				{
					unode split_location = end_idx->first;
					list<pair<list<program_environment>::iterator, unode> > split_environments;
					for (int i = oa.size()-1; i >= 0; i--)
					{
						cout << "+" << pc->name << "." << pc->net->node_name(oa[i]) << "{" << pc->net_index(oa[i]).print(pc->net->vars) << "} ";
						fflush(stdout);

						list<program_environment>::iterator update_env;
						list<remote_program_counter>::iterator update_pc, temp_pc;
						list<pair<unode, list<unode> > >::iterator update_idx, temp_idx;

						if (i != 0)
						{
							if (pc->is_place(end_idx->first.first))
							{
								envs.push_back(*env);
								update_env = prev(envs.end());
								for (update_pc = update_env->pcs.begin(), temp_pc = env->pcs.begin();
									 temp_pc != pc; update_pc++, temp_pc++);
								for (update_idx = update_pc->index.end.begin(), temp_idx = pc->index.end.begin();
									 temp_idx != end_idx; update_idx++, temp_idx++);
							}
							else
							{
								update_env = env;
								update_pc = pc;
								update_pc->index.end.push_front(*end_idx);
								update_idx = update_pc->index.end.begin();
							}
						}
						else
						{
							update_env = env;
							update_pc = pc;
							update_idx = end_idx;
						}

						unode new_node(oa[i], update_pc->index.nid(oa[i]));
						split_environments.push_back(pair<list<program_environment>::iterator, unode>(update_env, new_node));

						update_pc->index.arcs.push_back(uarc(update_idx->first, new_node));
						update_idx->first = update_pc->index.arcs.back().second;

						if (update_idx->second.size() == 0)
							update_idx->second.push_back(update_idx->first);
					}

					if (pc->is_place(split_location.first) && split_environments.size() > 1)
						splits.push_back(pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > >(pair<string, petri*>(pc->name, pc->net), split_environments));
				}
			}
		}

		changes = true;
		while (changes)
		{
			changes = false;
			logic state;
			if (begin_is_ready(s, env, pc, begin_idx, split, this_prgm, state))
			{
				changes = true;
				done = false;

				list<unode> ia = pc->index.input_nodes(*begin_idx);
				list<unode> oa = pc->index.output_nodes(*begin_idx);

				if (oa.size() == 0)
					pc->index.begin.erase(begin_idx);
				else if (pc->is_trans(begin_idx->first) && !pc->is_active(begin_idx->first) && split != splits.end())
				{
					cout << "LOOK " << split->second.size() << "{";
					fflush(stdout);
					int x = 0;
					for (list<pair<list<program_environment>::iterator, unode> >::iterator prgm = split->second.begin(); prgm != split->second.end(); )
					{
						cout << x++ << " ";
						bool delete_prgm = false;
						for (list<remote_program_counter>::iterator prgm_pc = prgm->first->pcs.begin(); !delete_prgm && prgm_pc != prgm->first->pcs.end(); prgm_pc++)
						{
							cout << prgm_pc->name;
							if (prgm_pc->name == split->first.first && prgm_pc->net == split->first.second)
							{
								for (list<pair<unode, list<unode> > >::iterator prgm_idx = prgm_pc->index.end.begin(); !delete_prgm && prgm_idx != prgm_pc->index.end.end(); prgm_idx++)
									if (prgm_idx->first == prgm->second && !prgm_pc->is_satisfied(prgm->second.first, state))
										delete_prgm = true;

								if (!delete_prgm && prgm_pc->index.path_contains(prgm->second))
								{
									cout << "rolling" << prgm_pc->net->node_name(prgm->second.first) << endl;
									// Roll the beginning forward until the conditional
									prgm_pc->roll_to(prgm->second);
								}
							}
							cout << " ";
						}

						if (delete_prgm)
						{
							list<program_environment>::iterator delete_env = prgm->first;

							// Loop through all splits and remove any reference to this environment
							for (list<pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > > >::iterator temp_split = splits.begin(); temp_split != splits.end(); temp_split++)
							{
								for (list<pair<list<program_environment>::iterator, unode> >::iterator temp_prgm = temp_split->second.begin(); temp_prgm != temp_split->second.end(); )
								{
									if (temp_prgm->first == prgm->first)
									{
										if (temp_split == split && temp_prgm == prgm)
											prgm++;

										temp_prgm = temp_split->second.erase(temp_prgm);
									}
									else
										temp_prgm++;
								}
							}

							envs.erase(delete_env);
						}
						else
							prgm++;
					}
					cout << "}" << endl;

					split = splits.erase(split);
				}
				else
				{
					for (list<unode>::iterator i = oa.begin(); i != oa.end(); i++)
					{
						cout << "-" << pc->name << "." << net->node_name(i->first) << " ";
						fflush(stdout);
						pc->index.begin.push_back(*i);
					}

					if (oa.size() > 0 || pc->index.end.size() == 0)
					{
						for (list<uarc>::iterator arc = pc->index.arcs.begin(); ia.size() == 0 && arc != pc->index.arcs.end(); )
						{
							if (arc->first == *begin_idx)
								arc = pc->index.arcs.erase(arc);
							else
								arc++;
						}

						pc->index.begin.erase(begin_idx);
					}
					else
					{
						cerr << "Error! " << pc->index.end.size() << " " << pc->net->node_name(pc->index.end.begin()->first.first) << endl;
						for (list<uarc>::iterator arc = pc->index.arcs.begin(); arc != pc->index.arcs.end(); arc++)
							cout << "(" << pc->net->node_name(arc->first.first) << "." << arc->first.second << "->" << pc->net->node_name(arc->second.first) << "." << arc->second.second << ") ";
						cout << endl;
					}
				}
			}
		}
	}

	cout << " {";
	for (list<program_environment>::iterator env = envs.begin(); env != envs.end(); env++)
	{
		cout << "[";
		for (list<remote_program_counter>::iterator rpc = env->pcs.begin(); rpc != env->pcs.end(); rpc++)
		{
			cout << "(";
			for (list<uarc>::iterator arc = rpc->index.arcs.begin(); arc != rpc->index.arcs.end(); arc++)
				cout << rpc->net->node_name(arc->first.first) << "." << arc->first.second << "->" << rpc->net->node_name(arc->second.first) << "." << arc->second.second << " ";
			cout << ")";
		}
		cout << "]";
	}
	cout << "}";
}

logic program_counter::mute(logic s)
{
	logic firings;
	// Since Binary AND is commutative, we don't need to traverse the graph.
	for (list<program_environment>::iterator i = envs.begin(); i != envs.end(); i++)
		firings.terms.push_back(i->firings());

	if (firings.terms.size() == 0)
		firings.terms.push_back(minterm());

	logic result;
	for (int i = 0; i < s.terms.size(); i++)
		for (int j = 0; j < firings.terms.size(); j++)
			result.terms.push_back((s.terms[i] & firings.terms[j]).xoutnulls() | s.terms[i]);

	result.mccluskey();

	return result;
}

int program_counter::count()
{
	int total = 0x7FFFFFFF;
	for (list<program_environment>::iterator env = envs.begin(); env != envs.end(); env++)
	{
		int num_ready = 0;
		for (list<remote_program_counter>::iterator pc = env->pcs.begin(); pc != env->pcs.end(); pc++)
			if (pc->net == net && pc->name == "this" && pc->index.end.size() == 1 && pc->count(unode(index, 0)) == 1)
				num_ready++;
		if (total > num_ready)
			total = num_ready;
	}

	if (total == 0x7FFFFFFF)
		total = 0;

	return  total;
}

void program_counter::merge()
{
	for (list<program_environment>::iterator env = envs.begin(); env != envs.end(); env++)
		for (list<remote_program_counter>::iterator pc = env->pcs.begin(); pc != env->pcs.end(); pc++)
			if (pc->net == net && pc->name == "this")
				for (list<pair<unode, list<unode> > >::iterator idx = pc->index.end.begin(); idx != pc->index.end.end();)
				{
					if (idx->first.first == index)
						idx = pc->index.end.erase(idx);
					else
						idx++;
				}
}

program_counter &program_counter::operator=(program_counter pc)
{
	index = pc.index;
	net = pc.net;
	waiting = pc.waiting;
	envs = pc.envs;
	splits.clear();
	for (list<pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > > >::const_iterator pc_split = pc.splits.begin(); pc_split != pc.splits.end(); pc_split++)
	{
		list<pair<list<program_environment>::iterator, unode> > split_environments;
		for (list<pair<list<program_environment>::iterator, unode> >::const_iterator pc_prgm = pc_split->second.begin(); pc_prgm != pc_split->second.end(); pc_prgm++)
		{
			list<program_environment>::iterator env;
			list<program_environment>::const_iterator pc_env;
			for (env = envs.begin(), pc_env = pc.envs.begin(); pc_env != pc.envs.end() && pc_env != pc_prgm->first; pc_env++, env++);
			split_environments.push_back(pair<list<program_environment>::iterator, unode>(env, pc_prgm->second));
		}

		splits.push_back(pair<pair<string, petri*>, list<pair<list<program_environment>::iterator, unode> > >(pc_split->first, split_environments));
	}
	return *this;
}

program_execution::program_execution()
{
	net = NULL;
	done = false;
	deadlock = false;
}

program_execution::program_execution(const program_execution &exe)
{
	net = exe.net;
	states = exe.states;
	pcs = exe.pcs;
	deadlock = exe.deadlock;
	done = exe.done;
}

program_execution::program_execution(petri *net)
{
	this->net = net;
	this->done = false;
	this->deadlock = false;
	this->states.resize(net->S.size(), logic());
}

program_execution::~program_execution()
{

}

int program_execution::count(list<program_counter>::iterator i)
{
	int total = 0;
	int check = i->net->input_arcs(i->index).size();
	for (list<program_counter>::iterator j = pcs.begin(); j != pcs.end(); j++)
		if (j->index == i->index && j->count()+1 == check)
			total++;

	cout << "LOOK AGAIN " << total << "/" << check << endl;

	return total;
}

void program_execution::merge(list<program_counter>::iterator i)
{
	i->merge();

	for (list<program_counter>::iterator pc = pcs.begin(); pc != pcs.end();)
	{
		if (pc != i && pc->index == i->index)
		{
			pc->merge();
			i->envs.insert(i->envs.end(), pc->envs.begin(), pc->envs.end());
			pc = pcs.erase(pc);
		}
		else
			pc++;
	}
}

logic program_execution::calculate_state(list<program_counter>::iterator pc, svector<logic> *f)
{
	if (net->is_place(pc->index))
	{
		svector<int> it = pc->net->input_nodes(pc->index);
		logic r = 0;
		for (int k = 0; k < it.size(); k++)
		{
			svector<int> ip = pc->net->input_nodes(it[k]);

			logic t = 1;
			for (int l = 0; l < ip.size(); l++)
				t &= states[ip[l]];

			if (pc->net->T[pc->net->index(it[k])].active)
				r |= (t >> pc->net->T[pc->net->index(it[k])].index);
			else
				r |= (t & pc->net->T[pc->net->index(it[k])].index);
		}

		pc->simulate(r);
		r = pc->mute(r);

		r &= pc->net->apply_debug(pc->index);

		if ((r & ((*f)[pc->index] | states[pc->index])) == r)
			done = true;
		else
			states[pc->index] |= r;

		return states[pc->index];
	}
	else
	{
		svector<int> ip = pc->net->input_nodes(pc->index);

		logic t;
		for (int l = 0; l < ip.size(); l++)
		{
			if (l == 0)
				t = states[ip[l]];
			else
				t &= states[ip[l]];
		}

		return t;
	}
}

program_execution &program_execution::operator=(program_execution exe)
{
	net = exe.net;
	states = exe.states;
	pcs = exe.pcs;
	deadlock = exe.deadlock;
	done = exe.done;
	return *this;
}

program_execution_space::program_execution_space()
{
}

program_execution_space::~program_execution_space()
{
	execs.clear();
}

void program_execution_space::simulate()
{
	final.clear();
	for (list<program_execution>::iterator exec = execs.begin(); exec != execs.end(); exec++)
	{
		for (int i = 0; i < exec->states.size(); i++)
			exec->states[i] = logic();

		for (int i = 0; i < exec->net->M0.size(); i++)
			exec->states[exec->net->M0[i]] = exec->net->vars->reset;

		if (final.size() < exec->states.size())
			final.resize(exec->states.size(), logic());

		for (list<program_counter>::iterator pc = exec->pcs.begin(); pc != exec->pcs.end(); pc++)
			for (list<program_environment>::iterator env = pc->envs.begin(); env != pc->envs.end(); env++)
				for (list<remote_program_counter>::iterator rpc = env->pcs.begin(); rpc != env->pcs.end(); rpc++)
					if (rpc->net != exec->net)
						for (int i = 0; i < exec->net->M0.size(); i++)
							exec->states[exec->net->M0[i]] &= rpc->net->vars->reset.refactor(rpc->reverse_factors);

		for (list<program_counter>::iterator pc = exec->pcs.begin(); pc != exec->pcs.end(); pc++)
			pc->simulate(exec->states[pc->index]);

		for (int i = 0; i < exec->net->M0.size(); i++)
			for (list<program_counter>::iterator pc = exec->pcs.begin(); pc != exec->pcs.end(); pc++)
				exec->states[exec->net->M0[i]] = pc->mute(exec->states[exec->net->M0[i]]);
	}

	for (list<program_execution>::iterator exec = execs.begin(); exec != execs.end();)
	{
		cout << "Start Execution" << endl;
		while (!exec->done && !exec->deadlock)
		{
			cout << "{" << endl;
			exec->deadlock = true;
			for (list<program_counter>::iterator pc = exec->pcs.begin(); pc != exec->pcs.end();)
			{
				cout << "\t[";
				svector<int> ia = pc->net->input_nodes(pc->index);
				svector<int> oa = pc->net->output_nodes(pc->index);

				bool parallel_split = (oa.size() > 1 && pc->is_trans());
				bool parallel_merge = (ia.size() > 1 && pc->is_trans());
				bool conditional_split = (oa.size() > 1 && pc->is_place());
				bool remove = false;

				pc->waiting = false;
				if (parallel_merge)
				{
					cout << "Parallel Merge ";
					if (exec->count(pc) == ia.size())
						exec->merge(pc);
					else
						pc->waiting = true;
				}

				/* Check to see if we an continue down this path yet. Yes if:
				 * 	- This is a place
				 * 	- This is an active transition
				 * 	- This is a satisfied passive transition
				 */
				if (!pc->waiting && (pc->is_place() || pc->is_active() || !is_mutex(exec->calculate_state(pc, &final), exec->net->T[exec->net->index(pc->index)].index)))
				{
					cout << pc->node_name();
					if (pc->is_place())
						cout << "{" << exec->states[pc->index].print(pc->net->vars) << "}";
					else
						cout << "{" << exec->net->T[exec->net->index(pc->index)].index.print(pc->net->vars) << "}";
					cout << " => ";
					fflush(stdout);
					exec->deadlock = false;
					for (int j = oa.size()-1; j >= 0; j--)
					{
						fflush(stdout);
						list<program_execution>::iterator update_exec = exec;
						list<program_counter>::iterator update_pc = pc, temp_pc;
						if (j != 0 && conditional_split)
						{
							// Duplicate the program_execution
							execs.push_back(*exec);
							update_exec = prev(execs.end());
							for (temp_pc = exec->pcs.begin(), update_pc = update_exec->pcs.begin(); temp_pc != pc && temp_pc != exec->pcs.end() && update_pc != update_exec->pcs.end(); temp_pc++, update_pc++);
							if (temp_pc == exec->pcs.end())
							{
								cout << "ERROR!" << endl;
								exit(0);
							}
						}
						else if (j != 0 && parallel_split)
						{
							// Duplicate the program_counter
							update_exec->pcs.push_front(*pc);
							update_pc = update_exec->pcs.begin();
						}

						if (parallel_split)
						{
							for (list<program_environment>::iterator update_env = update_pc->envs.begin(); update_env != update_pc->envs.end(); update_env++)
								for (int k = 0; k < oa.size(); k++)
									if (k != j)
										update_env->pcs.push_back(remote_program_counter(oa[k], pc->net));
						}

						update_pc->index = oa[j];
						cout << update_pc->node_name();
						fflush(stdout);

						// Check to see if we need to update the state encoding for this program execution
						if (update_pc->is_place())
							update_exec->states[update_pc->index] |= update_exec->calculate_state(update_pc, &final);

						if (update_pc->is_place())
							cout << "{" << update_exec->states[update_pc->index].print(update_pc->net->vars) << "}";
						else
							cout << "{" << update_exec->net->T[update_exec->net->index(update_pc->index)].index.print(update_pc->net->vars) << "}";

						cout << ", ";
					}

					if (oa.size() == 0)
					{
						cout << "Error: Net has acyclic component at " << pc->net->node_name(pc->index) << "." << endl;
						remove = true;
					}
				}
				cout << "]" << endl;

				if (remove)
					pc = exec->pcs.erase(pc);
				else
					pc++;
			}
			if (exec->deadlock)
				cerr << "Error: Deadlock detected: All program counters are waiting." << endl;
			cout << "}" << endl;
		}

		for (int i = 0; i < exec->states.size(); i++)
			final[i] |= exec->states[i];

		exec = execs.erase(exec);

		cout << "End Execution" << endl;
	}
}
