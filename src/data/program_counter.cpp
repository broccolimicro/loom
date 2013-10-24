/*
 * program_counter.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: nbingham
 */

#include "program_counter.h"
#include "petri.h"
#include "variable_space.h"

program_counter::program_counter()
{
	name = "";
	net = NULL;
	pc = 0;
}

program_counter::program_counter(sstring name, petri *from, petri *to, int pc)
{
	this->name = name;
	this->net = to;
	this->pc = pc;
	this->firings = svector<minterm>(1, minterm());

	for (smap<sstring, variable>::iterator vi = from->vars->global.begin(); vi != from->vars->global.end(); vi++)
	{
		if (vi->second.name.find(name) != vi->second.name.npos)
		{
			if (!vi->second.driven)
				hidden.push_back(vi->second.uid);

			forward_factors.push_back(pair<int, int>(vi->second.uid, net->vars->get_uid(vi->second.name.substr(name.length()+1))));
			reverse_factors.push_back(pair<int, int>(forward_factors.back().second, forward_factors.back().first));
		}
		else
			hidden.push_back(vi->second.uid);
	}
}

program_counter::program_counter(program_counter l, int p)
{
	name = l.name;
	net = l.net;
	pc = p;
	firings = l.firings;
	indices = l.indices;
	arcs = l.arcs;
	begin = l.begin;
	forward_factors = l.forward_factors;
	reverse_factors = l.reverse_factors;
	hidden = l.hidden;
}

program_counter::~program_counter()
{

}

minterm program_counter::mute()
{
	svector<minterm> fire;
	svector<int> idx;
	for (int i = 0; i < begin.size(); i++)
	{
		fire.push_back(minterm());
		idx.push_back(arcs[begin[i]].first);
	}
	int count;
	for (int j = 0; j < idx.size(); j++)
	{
		bool found = true;
		while (found)
		{
			fire[j] &= firings[idx[j]];

			count = 0;
			for (int k = 0; k < idx.size(); k++)
				if (idx[k] == idx[j])
					count++;
			if (count == net->input_nodes(indices[idx[j]]).size())
			{
				for (int k = 0; k < idx.size();)
				{
					if (idx[k] == idx[j] && k != j)
					{
						fire[j] &= fire[k];
						fire.erase(fire.begin() + k);
						idx.erase(idx.begin() + k);
					}
					else
						k++;
				}
			}
			else
				break;

			found = false;
			for (int k = 0; k < arcs.size(); k++)
				if (arcs[k].first == idx[j])
				{
					idx[j] = arcs[k].second;
					found = true;
				}
		}
	}

	if (fire.size() > 0)
		return fire[0] & firings.back();
	else
		return firings.back();
}

logic program_counter::translate(logic state)
{
	return state.hide(hidden).refactor(forward_factors);
}

void program_counter::update(logic state)
{
	int j;
	bool found_next;
	for (int k = 0; k < begin.size(); k++)
	{
		j = begin[k];
		logic temp = ~net->T[net->index(indices[arcs[j].first])].index;
		if (is_mutex(&temp, &state))
		{
			found_next = false;
			for (int l = 0; l < arcs.size(); l++)
			{
				if (arcs[l].first == arcs[j].second)
				{
					begin[k] = l;
					found_next = true;
				}

				if (arcs[l].second == arcs[j].first)
					arcs[l].second = arcs[j].second;
				else if (arcs[l].second > arcs[j].first)
					arcs[l].second--;

				if (arcs[l].first > arcs[j].first)
					arcs[l].first--;
			}

			if (!found_next)
			{
				begin.erase(begin.begin() + k);
				k--;
			}

			indices.erase(indices.begin() + arcs[j].first);
			firings.erase(firings.begin() + arcs[j].first);
			arcs.erase(arcs.begin() + j);
		}
	}
}

void program_counter::reset()
{
	firings.clear();
	firings.push_back(minterm());
	indices.clear();
	arcs.clear();
	begin.clear();
	pc = net->M0[0];
}

logic &program_counter::index()
{
	return (*net)[pc].index;
}

bool program_counter::is_active()
{
	return (*net)[pc].active;
}

bool program_counter::is_place()
{
	return net->is_place(pc);
}

bool program_counter::is_trans()
{
	return net->is_trans(pc);
}

program_counter_space::program_counter_space()
{

}

program_counter_space::~program_counter_space()
{

}

void program_counter_space::simulate(petri *net, logic state, int i)
{
	cout << endl << "{";

	svector<int> oa;
	bool issatisfied;
	logic output_state, temp;
	int count;

	for (int j = 0; j < pcs.size(); j++)
	{
		output_state = pcs[j].translate(state);

		cout << "[" << output_state.print(pcs[j].net->vars) << " -> ";

		temp = ~pcs[j].index();
		issatisfied = !is_mutex(&pcs[j].index(), &output_state);

		while (pcs[j].is_place() || pcs[j].is_active() || issatisfied)
		{
			cout << net->node_name(pcs[j].pc) << " ";
			if (pcs[j].is_trans() && pcs[j].is_active() && pcs[j].firings.size() > 0)
			{
				temp = pcs[j].index().refactor(pcs[j].reverse_factors);
				for (int k = 0; k < temp.terms.size(); k++)
					pcs[j].firings.back() &= temp.terms[k];
			}
			else if (pcs[j].is_trans() && !pcs[j].is_active() && issatisfied)
			{
				pcs[j].firings.push_back(minterm());
				pcs[j].indices.push_back(pcs[j].pc);
				pcs[j].arcs.push_back(pair<int, int>(pcs[j].indices.size()-1, pcs[j].firings.size()-1));

				count = 0;
				for (int k = 0; k < pcs[j].arcs.size(); k++)
					if (pcs[j].arcs[k].second == pcs[j].indices.size()-1)
						count++;
				if (count == 0)
					pcs[j].begin.push_back(pcs[j].arcs.size()-1);
			}

			count = 0;
			for (int k = 0; k < pcs.size(); k++)
				if (pcs[k].net == pcs[j].net && pcs[k].pc == pcs[j].pc)
					count++;

			if (count == pcs[j].net->input_nodes(pcs[j].pc).size())
				j = check_merges(j);
			else
				break;

			increment(j);

			temp = ~pcs[j].index();
			issatisfied = !is_mutex(&pcs[j].index(), &output_state);
		}

		pcs[j].update(output_state);

		for (int k = 0; k < pcs[j].indices.size(); k++)
			cout << "(" << net->node_name(pcs[j].indices[k]) << ", " << pcs[j].firings[k].print(net->vars) << ") ";
		cout << pcs[j].firings.back().print(net->vars) << "] ";
	}
	cout << "}" << endl;
}

logic program_counter_space::mute(logic state)
{
	minterm result;
	for (int i = 0; i < pcs.size(); i++)
		result &= pcs[i].mute();

	for (int i = 0; i < state.terms.size(); i++)
		state.terms[i] = (state.terms[i] & result).xoutnulls() | state.terms[i];

	return state;
}

void program_counter_space::reset()
{
	for (int i = 0; i < pcs.size(); i++)
	{
		for (int j = i+1; j < pcs.size();)
		{
			if (pcs[i].net == pcs[j].net)
				pcs.erase(pcs.begin() + j);
			else
				j++;
		}
		pcs[i].reset();
	}
}

void program_counter_space::increment(int i)
{
	svector<int> oa = pcs[i].net->output_nodes(pcs[i].pc);
	for (int k = oa.size() - 1; k > 0; k--)
		pcs.push_back(program_counter(pcs[i], oa[k]));
	if (oa.size() > 0)
		pcs[i].pc = oa[0];
	else
		cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << endl;
}

int program_counter_space::check_merges(int j)
{
	logic temp;
	for (int k = 0; k < pcs.size(); )
	{
		if (k != j && (pcs[k].net == pcs[j].net && pcs[k].pc == pcs[j].pc))
		{
			for (int l = 0; l < pcs[k].begin.size(); l++)
				pcs[j].begin.push_back(pcs[k].begin[l] + pcs[k].arcs.size());

			for (int l = 0; l < pcs[k].arcs.size(); l++)
				pcs[j].arcs.push_back(pair<int, int>(pcs[k].arcs[l].first + pcs[j].indices.size(), pcs[k].arcs[l].second + pcs[j].indices.size()));

			pcs[j].firings.pop_back();
			pcs[k].firings.pop_back();

			pcs[j].indices.merge(pcs[k].indices);
			pcs[j].firings.merge(pcs[k].firings);

			pcs[j].firings.push_back(minterm());

			pcs.erase(pcs.begin() + k);

			if (k < j)
				j--;
		}
		else
			k++;
	}

	return j;
}
