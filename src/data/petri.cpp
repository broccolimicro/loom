/*
 * petri.cpp
 *
 *  Created on: May 12, 2013
 *      Author: nbingham
 */

#include "petri.h"
#include "variable_space.h"
#include "../syntax/instruction.h"
#include "../syntax/assignment.h"
#include "../syntax/sequential.h"
#include "../syntax/parallel.h"
#include "../syntax/rule_space.h"

node::node()
{
	owner = NULL;
	assumptions = 1;
}

node::node(logic index, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	this->index = index;
	this->active = active;
	this->definitely_invacuous = false;
	this->possibly_vacuous = true;
	this->definitely_vacuous = false;
	this->pbranch = pbranch;
	this->cbranch = cbranch;
	this->owner = owner;
	assumptions = 1;
}

node::~node()
{
	owner = NULL;
}

bool node::is_in_tail(int idx)
{
	return (tail.find(idx) != tail.end());
}

void node::add_to_tail(int idx)
{
	tail.push_back(idx);
	tail.unique();
}

void node::add_to_tail(svector<int> idx)
{
	tail.insert(tail.end(), idx.begin(), idx.end());
	tail.unique();
}

pair<int, int> node::sense_count()
{
	pair<int, int> result(0, 0);
	for (int i = 0; i < index.terms.size(); i++)
	{
		for (int j = 0; j < index.terms[i].size; j++)
		{
			if (index.terms[i].val(j) == 0)
				result.first++;
			else if (index.terms[i].val(j) == 1)
				result.second++;
		}
	}

	return result;
}

petri::petri()
{
	pbranch_count = 0;
	cbranch_count = 0;
	prs = NULL;
}

petri::~petri()
{
	prs = NULL;
}

int petri::new_transition(logic root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int ret = T.size() | 0x80000000;
	T.push_back(node(root, active, pbranch, cbranch, owner));
	return ret;
}

svector<int> petri::new_transitions(svector<logic> root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	svector<int> result;
	for (int i = 0; i < (int)root.size(); i++)
		result.push_back(new_transition(root[i], active, pbranch, cbranch, owner));
	return result;
}

int petri::new_place(logic root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int ret = S.size();
	S.push_back(node(root, false, pbranch, cbranch, owner));
	return ret;
}

int petri::insert_transition(int from, logic root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int t = new_transition(root, (owner != NULL && owner->kind() == "assignment"), pbranch, cbranch, owner);
	if (is_trans(from))
		cerr << "Error: Illegal arc {T[" << index(from) << "], T[" << index(t) << "]}." << endl;
	else
	{
		arcs.push_back(pair<int, int>(from, t));
		S[from].active = S[from].active || T[index(t)].active;
	}

	return t;
}

int petri::insert_transition(svector<int> from, logic root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int t = new_transition(root, (owner != NULL && owner->kind() == "assignment"), pbranch, cbranch, owner);
	for (int i = 0; i < from.size(); i++)
	{
		if (is_trans(from[i]))
			cerr << "Error: Illegal arc {T[" << index(from[i]) << "], T[" << index(t) << "]}." << endl;
		else
		{
			arcs.push_back(pair<int, int>(from[i], t));;
			S[from[i]].active = S[from[i]].active || T[index(t)].active;
		}
	}

	return t;
}

void petri::insert_sv_at(int a, logic root)
{
	smap<int, int> pbranch, cbranch;
	instruction *owner;
	int t, p;

	if ((*this)[arcs[a].first].pbranch.size() > (*this)[arcs[a].second].pbranch.size() ||
		(*this)[arcs[a].first].cbranch.size() > (*this)[arcs[a].second].cbranch.size())
	{
		pbranch = (*this)[arcs[a].first].pbranch;
		cbranch = (*this)[arcs[a].first].cbranch;
		owner = (*this)[arcs[a].first].owner;
	}
	else
	{
		pbranch = (*this)[arcs[a].second].pbranch;
		cbranch = (*this)[arcs[a].second].cbranch;
		owner = (*this)[arcs[a].second].owner;
	}

	t = new_transition(root, true, pbranch, cbranch, owner);
	p = new_place(root, pbranch, cbranch, owner);

	if (is_trans(arcs[a].first) && is_place(arcs[a].second))
	{
		connect(arcs[a].first, p);
		connect(p, t);
		arcs[a].first = t;
	}
	else if (is_place(arcs[a].first) && is_trans(arcs[a].second))
	{
		connect(arcs[a].first, t);
		connect(t, p);
		arcs[a].first = p;
	}
}

void petri::insert_sv_parallel(int from, logic root)
{
	smap<int, int>::iterator bi, bj;
	svector<int> ip = input_nodes(from);
	svector<int> op = output_nodes(from);
	svector<int> input_places;
	svector<int> output_places;
	svector<int> fvl;
	svector<int> tvl;
	int trans;
	int i;

	T[index(from)].index.vars(&fvl);
	fvl.merge(vars->x_channel(fvl));
	root.vars(&tvl);

	// Implement the Physical Structure
	input_places = duplicate_nodes(ip);
	trans = insert_transition(input_places, root, T[index(from)].pbranch, T[index(from)].cbranch, T[index(from)].owner);
	for (i = 0; i < (int)op.size(); i++)
	{
		output_places.push_back(insert_place(trans, S[op[i]].pbranch, S[op[i]].cbranch, S[op[i]].owner));
		connect(output_places.back(), output_nodes(op[i]));
	}

	// Update Branch IDs
	for (i = 0; i < (int)input_places.size(); i++)
	{
		S[ip[i]].pbranch.insert(pair<int, int>(pbranch_count, 0));
		S[input_places[i]].pbranch.insert(pair<int, int>(pbranch_count, 1));
	}
	T[index(from)].pbranch.insert(pair<int, int>(pbranch_count, 0));
	T[index(trans)].pbranch.insert(pair<int, int>(pbranch_count, 1));
	for (i = 0; i < (int)output_places.size(); i++)
	{
		S[op[i]].pbranch.insert(pair<int, int>(pbranch_count, 0));
		S[output_places[i]].pbranch.insert(pair<int, int>(pbranch_count, 1));
	}

	pbranch_count++;
}

svector<int> petri::insert_transitions(int from, svector<logic> root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int t, i;
	svector<int> result;
	for (i = 0; i < (int)root.size(); i++)
	{
		t = new_transition(root[i], (owner->kind() == "assignment"), pbranch, cbranch, owner);
		if (is_trans(from))
			cerr << "Error: Illegal arc {T[" << index(from) << "], T[" << index(t) << "]}." << endl;
		else
		{
			arcs.push_back(pair<int, int>(from, t));
			S[from].active = S[from].active || T[index(t)].active;
		}

		result.push_back(t);
	}
	return result;
}

svector<int> petri::insert_transitions(svector<int> from, svector<logic> root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int t, i, j;
	svector<int> result;
	for (i = 0; i < (int)root.size(); i++)
	{
		t = new_transition(root[i], (owner->kind() == "assignment"), pbranch, cbranch, owner);
		for (j = 0; j < (int)from.size(); j++)
		{
			if (is_trans(from[j]))
				cerr << "Error: Illegal arc {T[" << index(from[j]) << "], T[" << index(t) << "]}." << endl;
			else
			{
				arcs.push_back(pair<int, int>(from[j], t));
				S[from[j]].active = S[from[j]].active || T[index(t)].active;
			}
		}

		result.push_back(t);
	}
	return result;
}

int petri::insert_dummy(int from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int t = new_transition(logic(1), false, pbranch, cbranch, owner);
	if (is_trans(from))
		cerr << "Error: Illegal arc {T[" << index(from) << "], T[" << index(t) << "]}." << endl;
	else
		arcs.push_back(pair<int, int>(from, t));
	return t;
}

int petri::insert_dummy(svector<int> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int t = new_transition(logic(1), false, pbranch, cbranch, owner);
	for (int i = 0; i < from.size(); i++)
	{
		if (is_trans(from[i]))
			cerr << "Error: Illegal arc {T[" << index(from[i]) << "], T[" << index(t) << "]}." << endl;
		else
			arcs.push_back(pair<int, int>(from[i], t));
	}
	return t;
}

int petri::insert_place(int from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int p = new_place(logic(0), pbranch, cbranch, owner);

	if (is_place(from))
		cerr << "Error: Illegal arc {S[" << from << "], S[" << p << "]}." << endl;
	else
		arcs.push_back(pair<int, int>(from, p));
	return p;
}

int petri::insert_place(svector<int> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	int p = new_place(logic(0), pbranch, cbranch, owner);
	for (int i = 0; i < from.size(); i++)
	{
		if (is_place(from[i]))
			cerr << "Error: Illegal arc {S[" << from[i] << "], S[" << p << "]}." << endl;
		else
			arcs.push_back(pair<int, int>(from[i], p));
	}

	return p;
}

svector<int> petri::insert_places(svector<int> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	svector<int> res;
	for (int i = 0; i < from.size(); i++)
		res.push_back(insert_place(from[i], pbranch, cbranch, owner));
	return res;
}

void petri::remove_place(int from)
{
	int i;
	svector<int> ot = output_nodes(from);
	svector<int> op;
	for (i = 0; i < (int)ot.size(); i++)
		op.merge(output_nodes(ot[i]));
	op.unique();
	for (i = 0; i < (int)op.size(); i++)
		if (op[i] > from)
			op[i]--;

	bool init = false;

	if (is_place(from))
	{
		for (i = 0; i < (int)M0.size(); i++)
		{
			if (M0[i] > from)
				M0[i]--;
			else if (M0[i] == from)
			{
				M0.erase(M0.begin() + i);
				init = true;
				i--;
			}
		}

		for (i = 0; i < (int)arcs.size();)
		{
			if (arcs[i].first == from || arcs[i].second == from)
				arcs.erase(arcs.begin() + i);
			else
			{
				if (arcs[i].first > from)
					arcs[i].first--;
				if (arcs[i].second > from)
					arcs[i].second--;

				i++;
			}
		}

		if (init)
			M0.insert(M0.end(), op.begin(), op.end());
		S.erase(S.begin() + from);
	}
	else
		cerr << "Error: " << index(from) << " must be a place." << endl;
}

void petri::remove_place(svector<int> from)
{
	for (int i = 0; i < from.size(); i++)
		remove_place(from[i]);
}

void petri::remove_trans(int from)
{
	if (is_trans(from))
	{
		for (int i = 0; i < arcs.size();)
		{
			if (arcs[i].first == from || arcs[i].second == from)
				arcs.erase(arcs.begin() + i);
			else
			{
				if (is_trans(arcs[i].first) && arcs[i].first > from)
					arcs[i].first--;
				if (is_trans(arcs[i].second) && arcs[i].second > from)
					arcs[i].second--;

				i++;
			}
		}

		T.erase(T.begin() + index(from));
	}
	else
		cerr << "Error: " << index(from) << " must be a transition." << endl;
}

void petri::remove_trans(svector<int> from)
{
	for (int i = 0; i < from.size(); i++)
		remove_trans(from[i]);
}

void petri::update()
{
	svector<bool> covered;
	svector<int> r1, r2;
	svector<int> ia;

	(*flags->log_file) << "Reset: {";
	for (int i = 0; i < (int)M0.size(); i++)
		cout << M0[i] << " ";
	cout << "} " << vars->reset.print(vars) << endl;

	env.simulate();

	for (int i = 0; i < env.final.size(); i++)
		S[i].index = env.final[i];

	for (int i = 0; i < T.size(); i++)
	{
		ia = input_nodes(trans_id(i));

		logic t = 1;
		for (int l = 0; l < ia.size(); l++)
			t &= S[ia[l]].index;

		logic r;
		if (T[i].active)
			r = (t >> T[i].index);
		else
			r = (t & T[i].index);

		T[i].definitely_invacuous = is_mutex(&t, &r);
		T[i].possibly_vacuous = !T[i].definitely_invacuous;
		T[i].definitely_vacuous = (t == r);
	}
}

void petri::check_assertions()
{
	for (int i = 0; i < (int)S.size(); i++)
	{
		for (int j = 0; j < (int)S[i].assertions.size(); j++)
			if ((S[i].index & S[i].assertions[j]) != 0)
				cerr << "Error: Assertion " << (~S[i].assertions[j]).print(vars) << " fails at state " << i << " with a state encoding of " << S[i].index.print(vars) << "." << endl;

		for (int j = 0; j < (int)vars->requirements.size(); j++)
			if ((S[i].index & vars->requirements[j]) != 0)
				cerr << "Error: Requirement " << (~vars->requirements[j]).print(vars) << " fails at state " << i << " with a state encoding of " << S[i].index.print(vars) << "." << endl;
	}
}

void petri::connect(svector<int> from, svector<int> to)
{
	for (int i = 0; i < from.size(); i++)
		for (int j = 0; j < to.size(); j++)
			connect(from[i], to[j]);
}

void petri::connect(svector<int> from, int to)
{
	for (int i = 0; i < from.size(); i++)
		connect(from[i], to);
}

void petri::connect(int from, svector<int> to)
{
	for (int j = 0; j < to.size(); j++)
		connect(from, to[j]);
}

void petri::connect(int from, int to)
{
	if (is_place(from) && is_trans(to))
	{
		arcs.push_back(pair<int, int>(from, to));
		S[from].active = S[from].active || T[index(to)].active;
	}
	else if (is_trans(from) && is_place(to))
		arcs.push_back(pair<int, int>(from, to));
	else if (is_place(from) && is_place(to))
		cerr << "Error: Illegal arc {S[" << from << "], S[" << to << "]}." << endl;
	else if (is_trans(from) && is_trans(to))
		cerr << "Error: Illegal arc {T[" << index(from) << "], T[" << index(to) << "]}." << endl;
}

pair<int, int> petri::closest_input(svector<int> from, svector<int> to, path p, int i)
{
	int count = 0;
	svector<int> next;
	svector<int> it, ip;
	svector<pair<int, int> > results;
	int j;
	int a;

	if (from.size() == 0)
		return pair<int, int>(arcs.size(), -1);

	next = to;

	while (next.size() == 1)
	{
		count++;
		a = next[0];

		if (p[a] > 0)
			return pair<int, int>(arcs.size(), -1);

		p[a]++;

		if (from.find(a) != from.end())
			return pair<int, int>(count, a);

		next.clear();
		for (j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].second == arcs[a].first)
				next.push_back(j);
	}

	for (j = 0; j < (int)next.size(); j++)
		if (p[next[j]] == 0)
		{
			count++;
			results.push_back(closest_input(from, svector<int>(1, next[j]), p, i+1));
		}
	results.unique();

	if (results.size() > 0)
		return pair<int, int>(results.front().first + count, results.front().second);
	else
		return pair<int, int>(arcs.size(), -1);
}

pair<int, int> petri::closest_output(svector<int> from, svector<int> to, path p, int i)
{
	int count = 0;
	svector<int> next;
	svector<int> it, ip;
	svector<pair<int, int> > results;
	int j;
	int a;

	if (to.size() == 0)
		return pair<int, int>(arcs.size(), -1);

	next = from;

	while (next.size() == 1)
	{
		count++;
		a = next[0];

		if (p[a] > 0)
			return pair<int, int>(arcs.size(), -1);

		p[a]++;

		if (to.find(a) != to.end())
			return pair<int, int>(count, a);

		next.clear();
		for (j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].first == arcs[a].second)
				next.push_back(j);
	}

	for (j = 0; j < (int)next.size(); j++)
		if (p[next[j]] == 0)
		{
			count++;
			results.push_back(closest_output(svector<int>(1, next[j]), to, p, i+1));
		}
	results.unique();

	if (results.size() > 0)
		return pair<int, int>(results.front().first + count, results.front().second);
	else
		return pair<int, int>(arcs.size(), -1);
}

bool petri::dead(int from)
{
	for (int i = 0; i < arcs.size(); i++)
		if (arcs[i].first == from || arcs[i].second == from)
			return false;

	return true;
}

bool petri::is_place(int from)
{
	return (from >= 0);
}

bool petri::is_trans(int from)
{
	return (from < 0);
}

int petri::index(int from)
{
	return (from&0x7FFFFFFF);
}

int petri::place_id(int idx)
{
	return (idx&0x7FFFFFFF);
}

int petri::trans_id(int idx)
{
	return (idx|0x80000000);
}

sstring petri::node_name(int idx)
{
	return (is_trans(idx) ? "T" : "S") + sstring(index(idx));
}

logic petri::base(svector<int> idx)
{
	logic res(1);
	int i;
	for (i = 0; i < (int)idx.size(); i++)
		res = res & S[idx[i]].index;

	return res;
}

bool petri::connected(int from, int to)
{
	svector<int> cf1, cf2;
	svector<int> ct1, ct2;

	for (int i = 0; i < arcs.size(); i++)
	{
		if (arcs[i].first == from)
			cf2.push_back(arcs[i].second);
		else if (arcs[i].second == from)
			cf1.push_back(arcs[i].first);
		if (arcs[i].first == to)
			ct2.push_back(arcs[i].second);
		else if (arcs[i].second == to)
			ct1.push_back(arcs[i].first);
	}

	cf1.unique();
	cf2.unique();
	ct1.unique();
	ct2.unique();

	for (int i = 0, j = 0; i < cf2.size() && j < ct1.size();)
	{
		if (cf2[i] == ct1[j])
			return true;
		else if (cf2[i] < ct1[j])
			i++;
		else if (cf2[i] > ct1[j])
			j++;
	}

	for (int i = 0, j = 0; i < cf1.size() && j < ct2.size();)
	{
		if (cf1[i] == ct2[j])
			return true;
		else if (cf1[i] < ct2[j])
			i++;
		else if (cf1[i] > ct2[j])
			j++;
	}

	return false;
}

int petri::psiblings(int p0, int p1)
{
	node *n0 = &((*this)[p0]);
	node *n1 = &((*this)[p1]);
	smap<int, int>::iterator bi, bj;
	for (bi = n0->pbranch.begin(); bi != n0->pbranch.end(); bi++)
		for (bj = n1->pbranch.begin(); bj != n1->pbranch.end(); bj++)
			if (bi->first == bj->first && bi->second != bj->second)
				return bi->first;
	return -1;
}

int petri::csiblings(int p0, int p1)
{
	node *n0 = &((*this)[p0]);
	node *n1 = &((*this)[p1]);
	smap<int, int>::iterator bi, bj;
	for (bi = n0->cbranch.begin(); bi != n0->cbranch.end(); bi++)
		for (bj = n1->cbranch.begin(); bj != n1->cbranch.end(); bj++)
			if (bi->first == bj->first && bi->second != bj->second)
				return bi->first;
	return -1;
}

bool petri::same_inputs(int p0, int p1)
{
	svector<int> it0, it1;
	bool diff_in = true;
	int k, l;

	it0 = input_nodes(p0);
	it1 = input_nodes(p1);

	if (it0.size() == it1.size())
	{
		diff_in = false;
		for (k = 0; k < (int)it0.size() && !diff_in; k++)
		{
			diff_in = true;
			for (l = 0; l < (int)it1.size() && diff_in; l++)
				if (T[it0[k]].index == T[it1[l]].index)
					diff_in = false;
		}
	}

	return !diff_in;
}

bool petri::same_outputs(int p0, int p1)
{
	svector<int> ot0, ot1;
	bool diff_out = true;
	int k, l;

	ot0 = output_nodes(p0);
	ot1 = output_nodes(p1);

	if (ot0.size() == ot1.size())
	{
		diff_out = false;
		for (k = 0; k < (int)ot0.size() && !diff_out; k++)
		{
			diff_out = true;
			for (l = 0; l < (int)ot1.size() && diff_out; l++)
				if (T[ot0[k]].index == T[ot1[l]].index)
					diff_out = false;
		}
	}

	return !diff_out;
}

svector<int> petri::duplicate_nodes(svector<int> from)
{
	svector<int> ret;
	for (int i = 0; i < from.size(); i++)
	{
		if (is_place(from[i]))
			ret.push_back(insert_place(input_nodes(from[i]), S[from[i]].pbranch, S[from[i]].cbranch, S[from[i]].owner));
		else
			ret.push_back(insert_transition(input_nodes(from[i]), T[index(from[i])].index, T[index(from[i])].pbranch, T[index(from[i])].cbranch, T[index(from[i])].owner));

		if (M0.find(from[i]) != M0.end())
			M0.push_back(ret.back());
	}
	return ret.rsort();
}

int petri::duplicate_node(int from)
{
	int ret;
	if (is_place(from))
		ret = insert_place(input_nodes(from), S[from].pbranch, S[from].cbranch, S[from].owner);
	else
		ret = insert_transition(input_nodes(from), T[index(from)].index, T[index(from)].pbranch, T[index(from)].cbranch, T[index(from)].owner);

	if (M0.find(from) != M0.end())
		M0.push_back(ret);

	return ret;
}

int petri::merge_places(svector<int> from)
{
	smap<int, int> pbranch;
	smap<int, int> cbranch;
	svector<pair<int, int> >::iterator ai;
	int j;
	logic idx(0);
	pbranch = S[from[0]].pbranch;
	cbranch = S[from[0]].cbranch;
	for (j = 0, idx = 0; j < (int)from.size(); j++)
	{
		pbranch = pbranch.set_intersection(S[from[j]].pbranch);
		cbranch = cbranch.set_intersection(S[from[j]].cbranch);
		idx = idx | S[from[j]].index;
	}

	int p = new_place(idx, pbranch, cbranch, NULL);
	for (j = 0; j < arcs.size(); j++)
	{
		if (from.find(arcs[j].first) != from.end())
			arcs.push_back(pair<int, int>(p, arcs[j].second));
		if (from.find(arcs[j].second) != from.end())
			arcs.push_back(pair<int, int>(arcs[j].first, p));
	}

	return p;
}

int petri::merge_places(int a, int b)
{
	smap<int, int> pbranch;
	smap<int, int> cbranch;
	svector<pair<int, int> >::iterator ai;
	int j, p;

	pbranch = S[a].pbranch.set_intersection(S[b].pbranch);
	cbranch = S[a].cbranch.set_intersection(S[b].cbranch);
	p = new_place((S[a].index | S[b].index), pbranch, cbranch, NULL);
	for (j = 0; j < arcs.size(); j++)
	{
		if (arcs[j].first == a || arcs[j].first == b)
			arcs.push_back(pair<int, int>(p, arcs[j].second));
		if (arcs[j].second == a || arcs[j].second == b)
			arcs.push_back(pair<int, int>(arcs[j].first, p));
	}

	return p;
}

svector<int> petri::input_nodes(int from)
{
	svector<int> ret;
	for (int i = 0; i < arcs.size(); i++)
		if (arcs[i].second == from)
			ret.push_back(arcs[i].first);
	return ret.rsort();
}

svector<int> petri::input_nodes(svector<int> from)
{
	svector<int> ret;
	for (int i = 0; i < arcs.size(); i++)
		if (from.find(arcs[i].second) != from.end())
			ret.push_back(arcs[i].first);
	return ret.rsort();
}

svector<int> petri::output_nodes(int from)
{
	svector<int> ret;
	for (int i = 0; i < arcs.size(); i++)
		if (arcs[i].first == from)
			ret.push_back(arcs[i].second);
	return ret.rsort();
}

svector<int> petri::output_nodes(svector<int> from)
{
	svector<int> ret;
	for (int i = 0; i < arcs.size(); i++)
		if (from.find(arcs[i].first) != from.end())
			ret.push_back(arcs[i].second);
	return ret.rsort();
}

svector<int> petri::input_arcs(int from)
{
	svector<int> result;
	for (int i = 0; i < (int)arcs.size(); i++)
		if (arcs[i].second == from)
			result.push_back(i);
	return result;
}

svector<int> petri::input_arcs(svector<int> from)
{
	svector<int> result;
	for (int i = 0; i < (int)arcs.size(); i++)
		if (from.find(arcs[i].second) != from.end())
			result.push_back(i);
	return result;
}


svector<int> petri::output_arcs(int from)
{
	svector<int> result;
	for (int i = 0; i < (int)arcs.size(); i++)
		if (arcs[i].first == from)
			result.push_back(i);
	return result;
}

svector<int> petri::output_arcs(svector<int> from)
{
	svector<int> result;
	for (int i = 0; i < (int)arcs.size(); i++)
		if (from.find(arcs[i].first) != from.end())
			result.push_back(i);
	return result;
}

svector<int> petri::increment_arcs(svector<int> a)
{
	svector<int> result;
	for (int i = 0; i < arcs.size(); i++)
		for (int j = 0; j < a.size(); j++)
			if (arcs[i].first == arcs[a[j]].second)
			{
				result.push_back(i);
				j = a.size();
			}
	return result;
}

svector<int> petri::decrement_arcs(svector<int> a)
{
	svector<int> result;
	for (int i = 0; i < arcs.size(); i++)
		for (int j = 0; j < a.size(); j++)
			if (arcs[i].second == arcs[a[j]].first)
			{
				result.push_back(i);
				j = a.size();
			}
	return result;
}

pair<int, int> petri::get_input_sense_count(int idx)
{
	pair<int, int> result(0, 0), temp;
	svector<int> input = input_nodes(idx);
	for (int j = 0; j < input.size(); j++)
	{
		temp = T[index(input[j])].sense_count();
		result.first += temp.first;
		result.second += temp.second;
	}
	return result;
}

pair<int, int> petri::get_input_sense_count(svector<int> idx)
{
	pair<int, int> result(0, 0), temp;
	svector<int> input = input_nodes(idx);
	for (int j = 0; j < input.size(); j++)
	{
		temp = T[index(input[j])].sense_count();
		result.first += temp.first;
		result.second += temp.second;
	}
	return result;
}

int petri::get_split_place(int merge_place, svector<bool> *covered)
{
	int i = merge_place, j, k, l;
	svector<int> ot, op, it, ip;
	svector<bool> c;
	bool loop;

	if ((*covered)[i])
		return -1;

	loop = true;
	it = input_nodes(i);
	if ((int)it.size() <= 0)
		return i;
	else if ((int)it.size() == 1)
	{
		ot = output_nodes(i);
		for (j = 0; j < (int)ot.size() && loop; j++)
		{
			op = output_nodes(ot[j]);
			for (k = 0; k < (int)op.size() && loop; k++)
				if (!(*covered)[op[k]])
					loop = false;
		}
	}

	(*covered)[i] = true;

	while (loop)
	{
		it = input_nodes(i);
		if ((int)it.size() <= 0)
			return i;
		else if ((int)it.size() == 1)
		{
			ip = input_nodes(it[0]);
			if (ip.size() == 0)
				return i;
			i = ip[0];

			if ((*covered)[i])
				return -1;
		}
		else
		{
			for (l = 0, k = -1; l < (int)it.size() && k == -1; l++)
			{
				ip = input_nodes(it[l]);
				for (j = 0; j < (int)ip.size() && k == -1; j++)
				{
					c = *covered;
					k = get_split_place(ip[j], &c);
				}
			}

			if (k == -1)
				return i;
			else
				i = ip[--j];
		}

		(*covered)[i] = true;

		loop = true;
		ot = output_nodes(i);
		for (j = 0; j < (int)ot.size() && loop; j++)
		{
			op = output_nodes(ot[j]);
			for (k = 0; k < (int)op.size() && loop; k++)
				if (!(*covered)[op[k]])
					loop = false;
		}
	}

	return i;
}

void petri::add_conflict_pair(smap<int, list<svector<int> > > *c, int i, int j)
{
	smap<int, list<svector<int> > >::iterator ri;
	list<svector<int> >::iterator li;
	svector<list<svector<int> >::iterator > gi;
	svector<int> group;
	int k;

	ri = c->find(i);
	if (ri != c->end())
	{
		gi.clear();
		for (li = ri->second.begin(); li != ri->second.end(); li++)
			for (k = 0; k < (int)li->size(); k++)
				if (connected(j, (*li)[k]))// || psiblings(j, (*li)[k]) != -1)
				{
					gi.push_back(li);
					k = (int)li->size();
				}

		group = svector<int>(1, j);
		for (k = 0; k < (int)gi.size(); k++)
		{
			group.insert(group.end(), gi[k]->begin(), gi[k]->end());
			ri->second.erase(gi[k]);
		}
		group.unique();
		ri->second.push_back(group);
	}
	else
		c->insert(pair<int, list<svector<int> > >(i, list<svector<int> >(1, svector<int>(1, j))));
}

void petri::gen_conditional_places()
{
	smap<int, pair<int, int> >::iterator ci;
	svector<int> oa, ia, sibs;
	int i, j, k;

	conditional_places.clear();
	for (i = 0; i < (int)S.size(); i++)
	{
		oa = output_nodes(i);
		ia = input_nodes(i);

		sibs.clear();
		for (j = 0; j < (int)oa.size(); j++)
			for (k = j+1; k < (int)oa.size(); k++)
				sibs.push_back(csiblings(oa[j], oa[k]));
		sibs.unique();

		for (k = 0; k < (int)sibs.size(); k++)
			if (sibs[k] != -1)
			{
				ci = conditional_places.find(sibs[k]);
				if (ci == conditional_places.end())
					conditional_places.insert(pair<int, pair<int, int> >(sibs[k], pair<int, int>(-1, i)));
				else
					ci->second.second = i;
			}

		sibs.clear();
		for (j = 0; j < (int)ia.size(); j++)
			for (k = j+1; k < (int)ia.size(); k++)
				sibs.push_back(csiblings(ia[j], ia[k]));
		sibs.unique();

		for (k = 0; k < (int)sibs.size(); k++)
			if (sibs[k] != -1)
			{
				ci = conditional_places.find(sibs[k]);
				if (ci == conditional_places.end())
					conditional_places.insert(pair<int, pair<int, int> >(sibs[k], pair<int, int>(i, -1)));
				else
					ci->second.first = i;
			}
	}
}

bool petri::are_sibling_guards(int i, int j)
{
	svector<int> i_in = input_nodes(input_nodes(i));
	svector<int> j_in = input_nodes(input_nodes(j));
	for (int k = 0; k < i_in.size(); k++)
		for (int l = 0; l < j_in.size(); l++)
			if (i_in[k] == j_in[l])
				return true;
	return false;
}

void petri::gen_conflicts()
{
	smap<int, list<svector<int> > >::iterator ri;
	list<svector<int> >::iterator li;
	svector<list<svector<int> >::iterator > gi;
	smap<int, int>::iterator bi, bj;
	svector<int> group;
	svector<int> oa;
	svector<int> temp;
	int i, j;
	logic t(0);
	logic nt(0);

	logic s1(0), s2(0), st;

	conflicts.clear();
	indistinguishable.clear();

	for (i = 0; i < (int)S.size(); i++)
	{
		cout << i << "/" << S.size() << endl;
		oa = output_nodes(i);

		// INDEX
		s1 = S[i].index;
		st = S[i].index;
		t = 1;
		for (j = 0; j < (int)oa.size(); j++)
			if (T[index(oa[j])].active)
			{
				for (int k = 0; k < s1.terms.size(); k++)
					for (int l = 0; l < T[index(oa[j])].index.terms.size(); l++)
					{
						s1.terms[k] &= T[index(oa[j])].index.terms[l];
						st.terms[k] &= T[index(oa[j])].index.terms[l].inverse();
					}

				t = t & T[index(oa[j])].index;
			}

		for (int k = 0, l = 0; k < s1.terms.size() && l < S[i].index.terms.size(); l++)
		{
			if (s1.terms[k] != 0 && st.terms[l] == 0)
				s1.terms.erase(s1.terms.begin() + k);
			else
			{
				s1.terms[k] = s1.terms[k].xoutnulls() | S[i].index.terms[l];
				k++;
			}
		}


		for (j = 0; j < (int)S.size(); j++)
		{
			/* States are conflicting if:
			 *  - they are not the same state
			 *  - one is not in the tail of another (the might as well be here case)
			 *  - the transition which causes the conflict is not a vacuous firing in the other state
			 *  - they are indistinguishable
			 *  - the two states do not exist in parallel
			 */

			s2 = get_effective_state_encoding(j, i);

			// INDEX
			if (i != j && psiblings(i, j) < 0 && !is_mutex(&s1, &s2))
			{
				cout << "CONFLICT " << node_name(i) << "=" << s1.print(vars) << " " << node_name(j) << "=" << s2.print(vars) << endl;

				oa = output_nodes(j);
				nt = ~t;
				// is it a conflicting state? (e.g. not vacuous)
				if (S[i].active && (!is_mutex(&nt, &S[i].index, &S[j].index) || (oa.size() > 0 && T[index(oa[0])].active && is_mutex(&t, &T[index(oa[0])].index))))
				{
					if (!are_sibling_guards(i, j))
						add_conflict_pair(&conflicts, i, j);
					else
						cerr << "Warning: Conditional near S" << i << " and S" << j << " has non mutually exclusive guards." << endl;
				}

				// it is at least an indistinguishable state at this point
				add_conflict_pair(&indistinguishable, i, j);
			}
		}
	}

	max_indistinguishables = 0;
	for (smap<int, list<svector<int> > >::iterator l = indistinguishable.begin(); l != indistinguishable.end(); l++)
		if ((int)l->second.size() > max_indistinguishables)
			max_indistinguishables = l->second.size();
}

void petri::gen_bubbleless_conflicts()
{
	smap<int, list<svector<int> > >::iterator ri;
	list<svector<int> >::iterator li;
	svector<list<svector<int> >::iterator > gi;
	smap<int, int>::iterator bi, bj;
	svector<int> group;
	svector<int> oa;
	svector<int> vl;
	svector<int> temp;
	int i, j;
	logic tp(0), ntp(0), sp1(0);
	logic tn(0), ntn(0), sn1(0);
	logic s2;
	bool parallel;
	bool strict;

	positive_conflicts.clear();
	positive_indistinguishable.clear();
	negative_conflicts.clear();
	negative_indistinguishable.clear();

	gen_conflicts();

	for (i = 0; i < (int)S.size(); i++)
	{
		ri = conflicts.find(i);
		oa = output_nodes(i);

		// POSITIVE
		vl.clear();
		for (j = 0, tp = 1; j < (int)oa.size(); j++)
			if (T[index(oa[j])].active)
			{
				T[index(oa[j])].negative.vars(&vl);
				tp &= T[index(oa[j])].negative;
			}
		vl.unique();
		sp1 = S[i].positive.hide(vl);

		// NEGATIVE
		vl.clear();
		for (j = 0, tn = 1; j < (int)oa.size(); j++)
			if (T[index(oa[j])].active)
			{
				T[index(oa[j])].positive.vars(&vl);
				tn &= T[index(oa[j])].positive;
			}
		vl.unique();
		sn1 = S[i].negative.hide(vl);

		for (j = 0; j < (int)S.size(); j++)
		{
			strict = false;
			if (ri != conflicts.end())
				for (li = ri->second.begin(); li != ri->second.end() && !strict; li++)
					if (li->find(j) != li->end())
						strict = true;

			if (!strict)
			{
				/* A node has to have at least one output transition due to the trim function.
				 * A node can only have one output transition if that transition is active,
				 * otherwise it can have multiple.
				 */
				oa = output_nodes(j);

				/* States are conflicting if:
				 *  - they are not the same state
				 *  - one is not in the tail of another (the might as well be here case)
				 *  - the transition which causes the conflict is not a vacuous firing in the other state
				 *  - they are indistinguishable
				 *  - the two states do not exist in parallel
				 */

				s2 = get_effective_state_encoding(j, i);

				parallel = (psiblings(i, j) >= 0);
				// POSITIVE
				if (i != j && !parallel && !is_mutex(&sp1, &s2))
				{
					// is it a conflicting state? (e.g. not vacuous)
					ntp = ~tp;
					if (S[i].active && (!is_mutex(&ntp, &sp1, &S[j].index) || (oa.size() > 0 && T[index(oa[0])].active && is_mutex(&tp, &T[index(oa[0])].index))))
					{
						if (!are_sibling_guards(i, j))
							add_conflict_pair(&positive_conflicts, i, j);
						else
							cerr << "Warning: Conditional near S" << i << " and S" << j << " has non mutually exclusive guards." << endl;
					}

					// it is at least an indistinguishable state at this point
					add_conflict_pair(&positive_indistinguishable, i, j);
				}

				// NEGATIVE
				if (i != j && !parallel && !is_mutex(&sn1, &s2))
				{
					// is it a conflicting state? (e.g. not vacuous)
					ntn = ~tn;
					if (S[i].active && (!is_mutex(&ntn, &sn1, &S[j].index) || (oa.size() > 0 && T[index(oa[0])].active && is_mutex(&tn, &T[index(oa[0])].index))))
					{
						if (!are_sibling_guards(i, j))
							add_conflict_pair(&negative_conflicts, i, j);
						else
							cerr << "Warning: Conditional near S" << i << " and S" << j << " has non mutually exclusive guards." << endl;
					}
					// it is at least an indistinguishable state at this point
					add_conflict_pair(&negative_indistinguishable, i, j);
				}
			}
		}
	}

	max_positive_indistinguishables = 0;
	for (smap<int, list<svector<int> > >::iterator l = positive_indistinguishable.begin(); l != positive_indistinguishable.end(); l++)
		if ((int)l->second.size() > max_positive_indistinguishables)
			max_positive_indistinguishables = l->second.size();

	max_negative_indistinguishables = 0;
	for (smap<int, list<svector<int> > >::iterator l = negative_indistinguishable.begin(); l != negative_indistinguishable.end(); l++)
		if ((int)l->second.size() > max_negative_indistinguishables)
			max_negative_indistinguishables = l->second.size();
}

void petri::gen_senses()
{
	int i;
	for (i = 0; i < (int)S.size(); i++)
	{
		S[i].positive = S[i].index.pabs();
		S[i].negative = S[i].index.nabs();
	}

	for (i = 0; i < (int)T.size(); i++)
	{
		T[i].positive = T[i].index.pabs();
		T[i].negative = T[i].index.nabs();
	}
}

logic petri::apply_debug(int pc)
{
	return (vars->enforcements & S[pc].assumptions);
}

void petri::trim_branch_ids()
{
	smap<int, svector<int> > pbranch_counts;
	smap<int, svector<int> > cbranch_counts;
	smap<int, svector<int> >::iterator bci;
	smap<int, int>::iterator bi;
	svector<smap<int, int>::iterator > remove;
	int i, j;

	for (i = 0; i < pbranch_count; i++)
		pbranch_counts.insert(pair<int, svector<int> >(i, svector<int>()));

	for (i = 0; i < cbranch_count; i++)
		cbranch_counts.insert(pair<int, svector<int> >(i, svector<int>()));

	for (i = 0; i < (int)T.size(); i++)
	{
		for (bi = T[i].pbranch.begin(); bi != T[i].pbranch.end(); bi++)
			pbranch_counts[bi->first].push_back(bi->second);
		for (bi = T[i].cbranch.begin(); bi != T[i].cbranch.end(); bi++)
			cbranch_counts[bi->first].push_back(bi->second);
	}

	for (bci = pbranch_counts.begin(); bci != pbranch_counts.end(); bci++)
		bci->second.unique();

	for (bci = cbranch_counts.begin(); bci != cbranch_counts.end(); bci++)
		bci->second.unique();

	for (i = 0; i < (int)T.size(); i++)
	{
		remove.clear();
		for (bi = T[i].pbranch.begin(); bi != T[i].pbranch.end(); bi++)
			if (pbranch_counts[bi->first].size() <= 1)
				remove.push_back(bi);

		for (j = 0; j < (int)remove.size(); j++)
			T[i].pbranch.erase(remove[j]);

		remove.clear();
		for (bi = T[i].cbranch.begin(); bi != T[i].cbranch.end(); bi++)
			if (cbranch_counts[bi->first].size() <= 1)
				remove.push_back(bi);

		for (j = 0; j < (int)remove.size(); j++)
			T[i].cbranch.erase(remove[j]);
	}

	for (i = 0; i < (int)S.size(); i++)
	{
		remove.clear();
		for (bi = S[i].pbranch.begin(); bi != S[i].pbranch.end(); bi++)
			if (pbranch_counts[bi->first].size() <= 1)
				remove.push_back(bi);

		for (j = 0; j < (int)remove.size(); j++)
			S[i].pbranch.erase(remove[j]);

		remove.clear();
		for (bi = S[i].cbranch.begin(); bi != S[i].cbranch.end(); bi++)
			if (cbranch_counts[bi->first].size() <= 1)
				remove.push_back(bi);

		for (j = 0; j < (int)remove.size(); j++)
			S[i].cbranch.erase(remove[j]);
	}
}

void petri::gen_tails()
{
	for (int i = 0; i < (int)S.size(); i++)
		S[i].tail.clear();
	for (int i = 0; i < (int)T.size(); i++)
		T[i].tail.clear();

	svector<int> ia, oa;
	svector<int> old;
	int i;
	bool done = false;
	while (!done)
	{
		done = true;
		for (i = 0; i < (int)arcs.size(); i++)
		{
			// State A->T is in the tail of T1 if A -> T -> Tail of T1 and T is either not active or not invacuous
			if (is_trans(arcs[i].first) && (!T[index(arcs[i].first)].active || T[index(arcs[i].first)].possibly_vacuous))
			{
				old = S[arcs[i].second].tail;
				S[arcs[i].second].add_to_tail(arcs[i].first);
				S[arcs[i].second].add_to_tail(T[index(arcs[i].first)].tail);
				done = done && (old == S[arcs[i].second].tail);
			}
			else if (is_place(arcs[i].first))
			{
				old = T[index(arcs[i].second)].tail;
				T[index(arcs[i].second)].add_to_tail(arcs[i].first);
				T[index(arcs[i].second)].add_to_tail(S[arcs[i].first].tail);
				done = done && (old == T[index(arcs[i].second)].tail);
			}
		}
	}

	cout << "TAILS" << endl;
	for (int i = 0; i < S.size(); i++)
	{
		cout << "S" << i << ": ";
		for (int j = 0; j < S[i].tail.size(); j++)
			cout << node_name(S[i].tail[j]);
		cout << endl;
	}

	for (int i = 0; i < T.size(); i++)
	{
		cout << "T" << i << ": ";
		for (int j = 0; j < T[i].tail.size(); j++)
			cout << node_name(T[i].tail[j]);
		cout << endl;
	}
}

smap<pair<int, int>, pair<bool, bool> > petri::gen_isochronics()
{
	svector<pair<int, int> > net;

	svector<logic> minimal_guards(vars->global.size()*2);
	svector<int> inputs, inputs2, outputs, outputs2;
	svector<int> tvars, ivars;
	int idx;
	for (int i = 0; i < T.size(); i++)
	{
		if (T[i].active)
		{
			inputs = input_nodes(trans_id(i));

			for (int j = 0; j < inputs.size(); j++)
			{
				inputs2 = input_nodes(inputs[j]);
				for (int k = 0; k < inputs2.size(); k++)
					if (T[index(inputs2[k])].index == 1)
					{
						idx = inputs2[k];
						inputs2.erase(inputs2.begin() + k);
						k = 0;
						inputs2.merge(input_nodes(input_nodes(idx)));
						inputs2.unique();
					}

				for (int k = 0; k < inputs2.size(); k++)
					if (!T[index(inputs2[k])].active)
						for (int l = 0; l < T[i].index.terms.size(); l++)
						{
							tvars = T[i].index.terms[l].vars();
							for (int m = 0; m < T[index(inputs2[k])].index.terms.size(); m++)
							{
								ivars = T[index(inputs2[k])].index.terms[m].vars();
								for (int n = 0; n < tvars.size(); n++)
									for (int o = 0; o < ivars.size(); o++)
										net.push_back(pair<int, int>(ivars[o]*2 + T[index(inputs2[k])].index.terms[m].val(ivars[o]), tvars[n]*2 + T[i].index.terms[l].val(tvars[n])));
							}
						}
			}
		}
	}

	net.unique();

	sstring label;
	cout << "digraph g0" << endl;
	cout << "{" << endl;

	for (smap<sstring, variable>::iterator i = vars->global.begin(); i != vars->global.end(); i++)
	{
		cout << "\tV" << i->second.uid*2 << " [label=\"" << i->second.name << "-\"];" << endl;
		cout << "\tV" << i->second.uid*2+1 << " [label=\"" << i->second.name << "+\"];" << endl;
	}

	for (int i = 0; i < net.size(); i++)
		cout << "\tV" << net[i].first << " -> V" << net[i].second << endl;

	cout << "}" << endl;

	svector<pair<int, int> > removed;
	svector<pair<int, int> >::iterator remarc;
	for (smap<sstring, variable>::iterator i = vars->global.begin(); i != vars->global.end(); i++)
		for (smap<sstring, variable>::iterator j = vars->global.begin(); j != vars->global.end(); j++)
			if (i != j)
			{
				if ((remarc = net.find(pair<int, int>(i->second.uid*2, j->second.uid*2))) != net.end() && net.find(pair<int, int>(i->second.uid*2+1, j->second.uid*2)) != net.end())
				{
					cerr << "Crap: " << i->second.name << " -> " << j->second.name << "-" << endl;
					removed.push_back(*remarc);
					net.erase(remarc);
				}

				if (net.find(pair<int, int>(i->second.uid*2, j->second.uid*2+1)) != net.end() && (remarc = net.find(pair<int, int>(i->second.uid*2+1, j->second.uid*2+1))) != net.end())
				{
					cerr << "Crap: " << i->second.name << " -> " << j->second.name << "+" << endl;
					removed.push_back(*remarc);
					net.erase(remarc);
				}

				if ((remarc = net.find(pair<int, int>(i->second.uid*2, j->second.uid*2))) != net.end() && net.find(pair<int, int>(i->second.uid*2, j->second.uid*2+1)) != net.end())
				{
					cerr << "Crap: " << i->second.name << "- -> " << j->second.name << endl;
					removed.push_back(*remarc);
					net.erase(remarc);
				}

				if (net.find(pair<int, int>(i->second.uid*2+1, j->second.uid*2)) != net.end() && (remarc = net.find(pair<int, int>(i->second.uid*2+1, j->second.uid*2+1))) != net.end())
				{
					cerr << "Crap: " << i->second.name << "+ -> " << j->second.name << endl;
					removed.push_back(*remarc);
					net.erase(remarc);
				}
			}

	cout << "digraph g1" << endl;
	cout << "{" << endl;

	for (smap<sstring, variable>::iterator i = vars->global.begin(); i != vars->global.end(); i++)
	{
		cout << "\tV" << i->second.uid*2 << " [label=\"" << i->second.name << "-\"];" << endl;
		cout << "\tV" << i->second.uid*2+1 << " [label=\"" << i->second.name << "+\"];" << endl;
	}

	for (int i = 0; i < net.size(); i++)
		cout << "\tV" << net[i].first << " -> V" << net[i].second << endl;

	cout << "}" << endl;

	smap<pair<int, int>, pair<bool, bool> > net2;
	smap<pair<int, int>, pair<bool, bool> >::iterator net2iter;
	for (int i = 0; i < net.size(); i++)
	{
		net2iter = net2.find(pair<int, int>(net[i].first/2, net[i].second/2));
		if (net2iter == net2.end())
			net2.insert(pair<pair<int, int>, pair<bool, bool> >(pair<int, int>(net[i].first/2, net[i].second/2), pair<bool, bool>(true, net[i].first%2 == net[i].second%2)));
		else
			net2iter->second.first = false;
	}

	cout << "digraph g2" << endl;
	cout << "{" << endl;

	for (smap<sstring, variable>::iterator i = vars->global.begin(); i != vars->global.end(); i++)
		cout << "\tV" << i->second.uid << " [label=\"" << i->second.name << "\"];" << endl;

	for (net2iter = net2.begin(); net2iter != net2.end(); net2iter++)
		cout << "\tV" << net2iter->first.first << " -> V" << net2iter->first.second << (net2iter->second.first ? " [style=dashed]" : "") << (net2iter->second.second ? " [arrowhead=odotnormal]" : "") << endl;

	cout << "}" << endl;

	/*for (int i = 0; i < removed.size(); i++)
	{
		cout << vars->get_name(node_name(ed[i].first/2) << (removed[i].first%2 == 0 ? "-" : "+") << " -> " << vars->get_name(removed[i].second/2) << (removed[i].second%2 == 0 ? "-" : "+") << "\t{";
		for (int j = 0; j < T.size(); j++)
		{
			if (T[j].index.val(removed[i].second/2) == (uint32_t)removed[i].second%2)
			{
				cout << "T" << j << ", ";
				inputs = input_nodes(trans_id(j));
				for (int k = 0; k < inputs.size(); k++)
				{
					for (int l = 0; l < S[inputs[k]].index.terms.size(); l++)
						S[inputs[k]].index.terms[l].sv_union(removed[i].first/2, (removed[i].first%2 == 0 ? v1 : v0));
				}
			}
		}
		cout << "}" << endl;
	}*/

	return net2;
}

logic petri::get_effective_state_encoding(int place, int observer)
{
	if (!is_place(place))
	{
		cerr << "Error: Internal failure at line " << __LINE__ << " in " << __FILE__ << "." << endl;
		return logic(0);
	}

	logic encoding = 0;
	if ((*this)[observer].is_in_tail(place) && observer != place)
	{
		svector<int> idx = output_arcs(place);
		while (idx.size() != 0)
		{
			for (int k = 0; k < idx.size();)
			{
				cout << node_name(arcs[idx[k]].first) << "=>" << node_name(arcs[idx[k]].second) << "{";
				if (is_place(arcs[idx[k]].second) && (*this)[observer].is_in_tail(arcs[idx[k]].second))
				{
					encoding |= ~T[index(arcs[idx[k]].first)].index;
					k++;
				}
				else if (is_place(arcs[idx[k]].second) && observer == arcs[idx[k]].second)
				{
					encoding |= ~T[index(arcs[idx[k]].first)].index;
					idx.erase(idx.begin() + k);
				}
				else if (is_trans(arcs[idx[k]].second) && (observer == arcs[idx[k]].second || !(*this)[observer].is_in_tail(arcs[idx[k]].second)))
					idx.erase(idx.begin() + k);
				else if (is_place(arcs[idx[k]].second))
					idx.erase(idx.begin() + k);
				else
					k++;
				cout << encoding.print(vars) << "} ";
			}
			idx = increment_arcs(idx);
		}
		return encoding & S[place].index;
	}
	else
		return S[place].index;
}

bool petri::trim()
{
	bool result = false;
	int i, j, l;
	svector<int> ia, oa, iia;
	smap<int, int>::iterator bi;
	svector<pair<int, int> >::iterator ai;
	bool vacuous;
	logic v;

	// Remove unreachable places
	/**
	 * A place is "unreachable" when there is no possible state encoding
	 * that would enable an input transition.
	 */
	for (i = 0; i < S.size();)
	{
		if (S[i].index == 0)
		{
			remove_place(i);
			result = true;
		}
		else
			i++;
	}

	for (i = 0; i < T.size();)
	{
		ia = input_nodes(trans_id(i));
		oa = output_nodes(trans_id(i));

		for (j = 0, v = 1; j < ia.size(); j++)
			v &= S[ia[j]].index;

		// Impossible and dangling transitions
		/**
		 * A transition is "impossible" when it can never fire, regardless of the state
		 * encodings at it's input places. Logically, you would think that this
		 * never should happen in the first place, but this can happen during the
		 * compilation of infinite loops. In an infinite loop, you will always
		 * continue back to the start of a loop, except what if the circuit designer
		 * placed an instruction after an infinite loop? It is easier for us to just
		 * place an impossible transition at the exit of the infinite loop, and then
		 * concatenate other instructions on there.
		 *
		 * A transition is "dangling" when it either has no input arcs or has
		 * no output arcs. This can happen after removing an unreachable state.
		 * Doing so makes it's neighbors dangle.
		 */
		if (T[i].index == 0 || ia.size() == 0 || oa.size() == 0)
		{
			remove_trans(trans_id(i));
			result = true;
		}

		// Vacuous Transitions
		/**
		 * A transition is "vacuous" when it does nothing to change the state of a
		 * system. We can remove all but two cases: First, if it has more than one
		 * input arc and more than one output arc, it actually makes the system more
		 * complex overall to remove that transition because you would have to merge
		 * every input place with every output place. If there were n input places
		 * and m output places, the number of places in the system would increase
		 * by (n*m - n - m). There is also a case we can't handle where there is
		 * only one output arc, but the place at the end of that arc is a conditional
		 * merge. The reason we can't handle this is because it would break the structure
		 * of the petri-net. You essentially have a parallel merge right before a
		 * conditional merge, and removing the transition that acts as the parallel
		 * merge would break the parallel block.
		 *
		 * The problem with this "vacuous" check is that while it may be locally vacuous,
		 * it can be globally invacuous. For example, If you have *[S1; x-; S2] where
		 * S1 and S2 don't modify the value of x, then x will always be 0 and the x-
		 * transition will be locally vacuous. However, if you then remove the x- transition
		 * and have *[S1;S2], you no longer know the value of x. Therefore, the x- transition
		 * is globally invacuous.
		 */
		else if (T[index(i)].definitely_vacuous && ia.size() == 1 && oa.size() == 1 && output_nodes(ia[0]).size() == 1)
		{
			for (l = 0; l < arcs.size(); l++)
				if (arcs[l].second == ia[0])
					arcs[l].second = oa[0];

			S[oa[0]].assumptions = S[ia[0]].assumptions >> S[oa[0]].assumptions;
			S[oa[0]].assertions.insert(S[oa[0]].assertions.end(), S[ia[0]].assertions.begin(), S[ia[0]].assertions.end());

			remove_place(ia[0]);
			remove_trans(trans_id(i));
			result = true;
		}
		else if (T[index(i)].definitely_vacuous && ia.size() == 1 && oa.size() == 1 && input_nodes(oa[0]).size() == 1)
		{
			for (l = 0; l < arcs.size(); l++)
				if (arcs[l].first == oa[0])
					arcs[l].first = ia[0];

			S[ia[0]].assumptions = S[ia[0]].assumptions >> S[oa[0]].assumptions;
			S[ia[0]].assertions.insert(S[ia[0]].assertions.end(), S[oa[0]].assertions.begin(), S[oa[0]].assertions.end());

			remove_place(oa[0]);
			remove_trans(trans_id(i));
			result = true;
		}
		/*else if (T[index(i)].definitely_vacuous && oa.size() == 1 && input_nodes(oa[0]).size() == 1 && (output_nodes(oa[0]).size() == 1 || input_nodes(trans_id(i)).size() == 1))
		{
			for (j = 0; j < ia.size(); j++)
			{
				for (l = 0; l < Wn[oa[0]].size(); l++)
				{
					Wn[ia[j]][l] = (Wn[ia[j]][l] || Wn[oa[0]][l]);
					Wp[ia[j]][l] = (Wp[ia[j]][l] || Wp[oa[0]][l]);
				}

				S[ia[j]].assumptions = S[ia[j]].assumptions >> S[oa[0]].assumptions;
				S[ia[j]].assertions.insert(S[ia[j]].assertions.end(), S[oa[0]].assertions.begin(), S[oa[0]].assertions.end());
			}

			remove_place(oa[0]);

			Wp.remr(i);
			Wn.remr(i);
			T.erase(T.begin() + i);
			result = true;
		}*/
		// All other transitions are fine
		else
			i++;
	}

	// Vacuous pbranches

	/**
	 * Whenever a branch in a parallel block has no transitions, it is a "vacuous branch".
	 * It is equivalent to saying that "nothing" is happening in parallel with "something".
	 * This can happen after the removal of vacuous transitions in a parallel block.
	 */
	for (i = 0; i < S.size(); )
	{
		vacuous = true;
		for (j = 0; j < T.size() && vacuous; j++)
		{
			vacuous = false;
			for (bi = S[i].pbranch.begin(); bi != S[i].pbranch.end() && !vacuous; bi++)
				if (find(T[j].pbranch.begin(), T[j].pbranch.end(), *bi) == T[j].pbranch.end())
					vacuous = true;
		}

		if (vacuous)
		{
			remove_place(i);
			result = true;
		}
		else
			i++;
	}

	// Merge Or
	for (i = 0; i < T.size(); i++)
		for (j = i+1; j < T.size(); )
		{
			if (!T[i].active && !T[j].active && input_nodes(trans_id(i)) == input_nodes(trans_id(j)) && output_nodes(trans_id(i)) == output_nodes(trans_id(j)))
			{
				T[i].index |= T[j].index;
				remove_trans(trans_id(j));
			}
			else
				j++;
		}

	// Check for nodes with no input arcs
	for (i = 0; i < M0.size(); i++)
	{
		ia = input_arcs(M0[i]);

		if (ia.size() == 0)
		{
			oa = output_nodes(M0[i]);

			bool all_active = true;
			for (j = 0; all_active && j < oa.size(); j++)
				if (!T[index(oa[j])].active || is_mutex(vars->reset, ~T[index(oa[j])].index))
					all_active = false;


			if (all_active)
			{
				remove_place(M0[i]);
				for (j = 0; j < oa.size(); j++)
				{
					vars->reset = vars->reset >> T[index(oa[j])].index;
					remove_trans(oa[j]);
				}
			}
		}
	}

	/* Despite our best efforts to fix up the pbranch id's on the fly, we still
	 * need to clean up.
	 */
	trim_branch_ids();

	return result;
}

void petri::merge_conflicts()
{
	svector<int>				remove;
	svector<int>				ia, oa;
	svector<int>				vli, vlji, vljo;
	svector<svector<int> >	groups(S.size());
	svector<pair<logic, logic> > merge_value(S.size());	// the aggregated state encoding of input places and aggregated state encoding of output places for each group
	svector<pair<logic, logic> >::iterator ai;
	smap<int, int>			pbranch, cbranch;
	int						i, j, k, p;
	bool					conflict;

	/**
	 * Identify the groups of indistinguishable states and keep track of their aggregate
	 * state encoding. Aggregated state encodings the of input and output places within a
	 * group must be tracked separately because the aggregate state encoding of input
	 * places is the binary AND of all of the individual state encodings and the aggregate
	 * state encoding of the output places is the binary OR of all of the individual state
	 * encodings. Finally, the two aggregate values are combined with a binary AND. The
	 * reason for this is that if you have two transitions that have the same input
	 * place, the state encoding for that input place must allow for either transition
	 * to fire at any time. That means that there cannot exist a state encoding within
	 * the input place that prevents one of the transitions from firing, hence binary AND
	 * (intersection). Also, if there are two transitions with the same output places,
	 * then either transition can fire and place a token in that place, hence binary OR
	 * (union).
	 */
	for (i = 0; i < S.size(); i++)
	{
		groups[i].push_back(i);
		if (input_nodes(i).size() == 0)
		{
			merge_value[i].first = S[i].index;
			merge_value[i].second = 0;
		}
		else if (output_nodes(i).size() == 0)
		{
			merge_value[i].first = 1;
			merge_value[i].second = S[i].index;
		}
	}

	for (i = 0; i < S.size(); i++)
	{
		p = groups.size();
		for (j = 0; j < p; j++)
			if (groups[j].find(i) == groups[j].end())
			{
				ia = input_nodes(i);
				oa = output_nodes(i);

				conflict = true;
				for (k = 0; k < groups[j].size() && conflict; k++)
					if ((S[i].index & S[groups[j][k]].index) == 0)
						conflict = false;

				/**
				 * To add a place into a conflict group, it's state encoding must be indistinguishable
				 * with the state encoding of every other place in that group.
				 */
				if (conflict)
				{
					vli.clear();
					vlji.clear();
					vljo.clear();

					S[i].index.vars(&vli);
					merge_value[j].first.vars(&vlji);
					merge_value[j].second.vars(&vljo);

					vli.unique();
					vlji.unique();
					vljo.unique();

					/**
					 * Furthermore, if it is an input place, the amount of information that it's state
					 * encoding contains must be less than or equal to the amount of information contained
					 * in the state encoding of every output place currently in the group and exactly equal
					 * to the amount of state encoding in every input place currently in the group. If it
					 * is an output place, the amount of information that it's state encode contains must
					 * be greater than or equal to the amount of information contained in every input place
					 * currently in the group.
					 *
					 * For example if we start out knowing that the variable X is equal to 0, and then do a
					 * transition and that changes the value of X to 1, we can't suddenly know that the
					 * value of Y is 1 in order to execute the next transition. Also, if we know that
					 * X must be 1 to fire transition A, and X and Y must both be 1 to fire transition B,
					 * we can't suddenly know that Y is 1 every time we want to fire transition A.
					 */

					/* This check for information can be streamlined by checking which variables are
					 * included in the encoding, and not checking their value. The check to make sure that
					 * the state encodings are indistinguishable does the value check for us. For example, if we
					 * knew that X must be 1 to fire transition A, we can't suddenly know that X must also
					 * be 0 to fire transition B. Those two state encodings are not conflicting.
					 */
					/*if ((ia.size() == 0 && (vli == vlji && includes(vljo.begin(), vljo.end(), vli.begin(), vli.end()))) ||
						(oa.size() == 0 && (includes(vli.begin(), vli.end(), vlji.begin(), vlji.end()))))
					{*/
						groups.push_back(groups[j]);
						merge_value.push_back(merge_value[j]);
						groups[j].push_back(i);

						if (ia.size() == 0)
							merge_value[j].first = merge_value[j].first & S[i].index;

						if (oa.size() == 0)
							merge_value[j].second = merge_value[j].second | S[i].index;
					//}
				}
			}
	}

	/* The code segment above results in many duplicate groups and groups that
	 * are subsets of other groups. We need to remove these to make sure that we
	 * only get one place for each unique set of indistinguishable places.
	 */
	for (i = 0; i < groups.size(); i++)
		groups[i].unique();

	for (i = 0; i < groups.size(); i++)
		for (j = 0; j < groups.size(); j++)
			if (i != j && (includes(groups[i].begin(), groups[i].end(), groups[j].begin(), groups[j].end()) || groups[j].size() <= 1))
			{
				groups.erase(groups.begin() + j);
				merge_value.erase(merge_value.begin() + j);
				j--;
				if (i > j)
					i--;
			}

	cout << "MERGES" << endl;
	for (i = 0; i < groups.size(); i++)
	{
		for (j = 0; j < groups[i].size(); j++)
			cout << groups[i][j] << " ";
		cout << endl;
	}
	cout << endl << endl;

	// Merge all of the places in each group identified above.
	for (i = 0; i < groups.size(); i++)
	{
		pbranch.clear();
		cbranch.clear();
		for (j = 0; j < groups[i].size(); j++)
		{
			pbranch.merge(S[groups[i][j]].pbranch);
			cbranch.merge(S[groups[i][j]].cbranch);
		}
		if (merge_value[i].second == 0)
			merge_value[i].second = 1;

		p = new_place((merge_value[i].first & merge_value[i].second), pbranch, cbranch, NULL);
		for (j = 0; j < groups[i].size(); j++)
		{
			for (k = 0; k < arcs.size(); k++)
			{
				if (arcs[k].first == groups[i][j])
					arcs.push_back(pair<int, int>(p, arcs[k].second));
				if (arcs[k].second == groups[i][j])
					arcs.push_back(pair<int, int>(arcs[k].first, p));
			}
			remove.push_back(groups[i][j]);
		}
	}

	remove.unique();
	for (i = remove.size()-1; i >= 0; i--)
		remove_place(remove[i]);
}

void petri::zip()
{
	svector<int> it0, ot0, it1, ot1, remove;
	svector<svector<int> > groups;
	smap<int, int> pbranch;
	smap<int, int>::iterator bi, bj;
	svector<pair<int, int> >::iterator ai;
	int i, j, k, p;

	/**
	 * First, check the places for possible merges, and then move on to the
	 * transitions because the act of merging places gives more information
	 * about the transitions to which they are connected.
	 *
	 * If two places have exactly the same neighborhood (the same sets of
	 * input and output arcs), then these two places are really just two
	 * possible state encodings for the same place. This generally happens
	 * directly after a conditional merge (where there is one state encoding
	 * for each possible path through the conditional). Instead of merging
	 * just two places at a time, we go ahead and merge all places that have
	 * the same neighborhoods.
	 */
	for (i = 0; i < S.size(); i++)
		groups.push_back(svector<int>(1, i));
	for (i = 0; i < S.size(); i++)
	{
		p = groups.size();
		for (j = 0; j < p; j++)
			if (groups[j].find(i) == groups[j].end() && same_inputs(i, groups[j][0]) && same_outputs(i, groups[j][0]))
			{
				groups.push_back(groups[j]);
				groups[j].push_back(i);
			}
	}

	/* The code segment above results in many duplicate groups and groups that
	 * are subsets of other groups. We need to remove these to make sure that we
	 * only get one place for each unique neighborhood once we have merge all of
	 * the places in each individual group.
	 */
	for (i = 0; i < groups.size(); i++)
		groups[i].unique();
	groups.unique();
	for (i = 0; i < groups.size(); i++)
		for (j = 0; j < groups.size(); j++)
			if (i != j && (includes(groups[i].begin(), groups[i].end(), groups[j].begin(), groups[j].end()) || groups[j].size() <= 1))
			{
				groups.erase(groups.begin() + j);
				j--;
				if (i > j)
					i--;
			}

	for (i = 0; i < groups.size(); i++)
		if (groups[i].size() > 1)
		{
			merge_places(groups[i]);
			remove.merge(groups[i]);
		}

	remove.unique();
	for (i = remove.size()-1; i >= 0; i--)
		remove_place(remove[i]);

	remove.clear();
	groups.clear();

	/**
	 * After checking the places for possible merge points, we also need to
	 * check the transitions. Logically, If there are multiple transitions that
	 * start at the same place and do the same thing, then they should also
	 * have the same result. So we can just merge their output arcs and delete
	 * all but one of the transitions.
	 */
	for (i = 0; i < T.size(); i++)
		for (j = i+1; j < T.size(); j++)
			if (T[i].index == T[j].index && input_nodes(trans_id(i)) == input_nodes(trans_id(j)))
			{
				for (k = 0; k < arcs.size(); k++)
					if (arcs[k].second == trans_id(j))
						arcs.push_back(pair<int, int>(arcs[k].first, trans_id(i)));

				T[i].pbranch = T[i].pbranch.set_intersection(T[j].pbranch);
				T[i].cbranch = T[i].cbranch.set_intersection(T[j].cbranch);

				remove.push_back(j);
			}

	remove.unique();
	for (i = remove.size()-1; i >= 0; i--)
	{
		T.erase(T.begin() + remove[i]);
		for (j = 0; j < arcs.size();)
		{
			if (arcs[j].first == remove[i] || arcs[j].second == remove[i])
				arcs.erase(arcs.begin() + j);
			else
				j++;
		}
	}

	/* Despite our best efforts to fix up the branch id's on the fly, we still
	 * need to clean up.
	 */
	trim_branch_ids();
}

void petri::get_paths(svector<int> from, svector<int> to, path_space *p)
{
	path_space t(arcs.size());
	svector<int> ex;
	int i;

	for (i = 0; i < from.size(); i++)
	{
		t.clear();
		t.push_back(path(arcs.size()));
		ex = from;
		ex.erase(ex.begin() + i);
		arc_paths(from[i], to, ex, &t, p);
	}

	p->total.from.unique();
	p->total.to.unique();
}

svector<int> petri::arc_paths(int from, svector<int> to, svector<int> ex, path_space *t, path_space *p, int i)
{
	path_space tmp1(arcs.size()), tmp2(arcs.size());
	svector<int> next, curr, temp;
	smap<int, pair<int, int> >::iterator cpi;
	int j, k;
	list<path>::iterator pi;
	bool immune = false;

	/*cout << sstring(i, '\t') << "Arc Paths " << from << " -> {";
	for (j = 0; j < to.size(); j++)
	{
		if (j != 0)
			cout << ", ";
		cout << to[j];
	}
	cout << "}" << endl;*/

	if (i == 0)
	{
		t->paths.front().from.push_back(from);
		t->total.from.push_back(from);
	}

	next.push_back(from);

	while (1)
	{
		//cout << sstring(i, '\t') << next[0] << " ";

		for (pi = t->paths.begin(); pi != t->paths.end();)
		{
			if ((*pi)[next[0]] > 0)
				pi = t->erase(pi);
			else
				pi++;
		}

		if (t->paths.size() == 0)
		{
			//cout << "ALREADY COVERED " << t->total << endl;
			return svector<int>();
		}

		t->inc(next[0]);

		for (j = 0; j < to.size(); j++)
			if (next[0] == to[j])
			{
				for (pi = t->paths.begin(); pi != t->paths.end(); pi++)
				{
					pi->to.push_back(to[j]);
					p->push_back(pi->mask());
				}
				//cout << "END " << t->total << endl;
				return svector<int>();
			}

		curr = next;
		next.clear();
		for (j = 0; j < arcs.size(); j++)
			for (k = 0; k < curr.size(); k++)
				if (arcs[curr[k]].second == arcs[j].first && ex.find(j) == ex.end())
					next.push_back(j);
		next.unique();

		if (!immune && i != 0 && is_trans(arcs[next[0]].first) && input_arcs(arcs[next[0]].first).size() > 1)
		{
			//cout << "PARALLEL MERGE " << t->total << endl;
			return next;
		}

		for (cpi = conditional_places.begin(); !immune && i != 0 && cpi != conditional_places.end(); cpi++)
			if (arcs[next[0]].first == cpi->second.first)
			{
				//cout << "CONDITIONAL MERGE " << t->total << endl;
				return next;
			}

		//cout << "\t" << t->total << endl;

		immune = false;
		while (next.size() > 1)
		{
			curr = next;
			tmp2 = *t;

			t->clear();
			next.clear();
			for (j = 0; j < curr.size(); j++)
			{
				tmp1 = tmp2;
				temp = arc_paths(curr[j], to, ex, &tmp1, p, i+1);
				if (temp.size() != 0)
				{
					next.merge(temp);
					immune = true;
					t->merge(tmp1);
				}
			}

			next.unique();
		}

		if (next.size() < 1)
		{
			//cout << "KILL" << endl;
			return svector<int>();
		}
	}
}

void petri::filter_path_space(path_space *p)
{
	smap<int, pair<int, int> >::iterator ci;
	svector<int> it, ip;
	list<path>::iterator pi;
	int i;
	int count, total;

	for (ci = conditional_places.begin(); ci != conditional_places.end(); ci++)
	{
		for (i = 0; i < (int)arcs.size(); i++)
			if (arcs[i].second == ci->second.first)
				it.push_back(i);

		for (i = 0, count = 0, total = 0; i < (int)it.size(); i++)
		{
			count += (p->total[it[i]] > 0);
			total++;
		}

		for (pi = p->paths.begin(); pi != p-> paths.end() && count > 0 && count < total; pi++)
			for (i = 0; i < (int)it.size(); i++)
				if ((*pi)[it[i]] > 0)
					filter_path(ci->second.second, it[i], &(*pi));

		for (int j = 0; j < (int)p->total.size(); j++)
			p->total[j] = 0;

		for (pi = p->paths.begin(); pi != p->paths.end(); pi++)
		{
			count = 0;
			for (int j = 0; j < (int)pi->size(); j++)
			{
				p->total[j] += (*pi)[j];
				count += (*pi)[j];
			}
			if (count == 0)
				pi = p->paths.erase(pi);
		}
	}
}

void petri::filter_path(int from, int to, path *p)
{
	svector<int> next;
	svector<int> it, ip;
	int i;

	next.push_back(to);

	while (next.size() == 1)
	{
		to = next[0];
		if (p->nodes[to] == 0)
			return;

		p->nodes[to] = 0;

		if (arcs[to].first == from || find(p->from.begin(), p->from.end(), arcs[to].first) != p->from.end() ||
							   	   	  find(p->to.begin(), p->to.end(), arcs[to].first) != p->to.end())
			return;

		next.clear();
		for (i = 0; i < (int)arcs.size(); i++)
			if (arcs[to].first == arcs[i].second)
				next.push_back(i);
	}

	for (i = 0; i < (int)next.size(); i++)
		filter_path(from, next[i], p);
}

void petri::zero_paths(path_space *paths, int from)
{
	for (int i = 0; i < (int)arcs.size(); i++)
		if (arcs[i].first == from || arcs[i].second == from)
			paths->zero(i);
}

void petri::zero_paths(path_space *paths, svector<int> from)
{
	for (int i = 0; i < (int)from.size(); i++)
		for (int j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].first == from[i] || arcs[j].second == from[i])
				paths->zero(j);
}

void petri::zero_ins(path_space *paths, int from)
{
	for (int i = 0; i < (int)arcs.size(); i++)
		if (arcs[i].second == from)
			paths->zero(i);
}

void petri::zero_ins(path_space *paths, svector<int> from)
{
	for (int i = 0; i < (int)from.size(); i++)
		for (int j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].second == from[i])
				paths->zero(j);
}

void petri::zero_outs(path_space *paths, int from)
{
	for (int i = 0; i < (int)arcs.size(); i++)
		if (arcs[i].first == from)
			paths->zero(i);
}

void petri::zero_outs(path_space *paths, svector<int> from)
{
	for (int i = 0; i < (int)from.size(); i++)
		for (int j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].first == from[i])
				paths->zero(j);
}

svector<int> petri::start_path(int from, svector<int> ex)
{
	svector<int> result, oa;
	smap<int, pair<int, int> >::iterator ci;
	smap<int, int>::iterator bi, bj, bk;
	int j, k;
	bool same;

	result.push_back(from);
	for (bi = S[arcs[from].second].cbranch.begin(); bi != S[arcs[from].second].cbranch.end(); bi++)
	{
		ci = conditional_places.find(bi->first);
		if (ci != conditional_places.end())
		{
			oa = output_arcs(ci->second.second);
			for (j = 0; j < (int)oa.size(); j++)
			{
				bk = T[index(arcs[oa[j]].second)].cbranch.find(bi->first);
				same = (bk->second == bi->second);

				for (k = 0; k < (int)ex.size() && !same; k++)
				{
					bj = (*this)[arcs[ex[k]].second].cbranch.find(bi->first);
					if (bj != (*this)[arcs[ex[k]].second].cbranch.end() && bj->second == bk->second)
						same = true;
				}

				if (!same)
					result.push_back(oa[j]);
			}
		}
		else
		{
			cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
			cout << "Failed to find the conditional branch " << bi->first << "." << endl;
			print_branch_ids(&cout);
		}
	}
	result.unique();

	/*cout << "Start {" << node_name(from) << "} -> {";
	for (j = 0; j < (int)result.size(); j++)
		cout << node_name(result[j]) << " ";
	cout << "}" << endl;*/

	return result;
}

svector<int> petri::start_path(svector<int> from, svector<int> ex)
{
	svector<int> result, oa;
	smap<int, pair<int, int> >::iterator ci;
	smap<int, int>::iterator bi, bj, bk;
	int i, j, k;
	bool same = false;

	for (i = 0; i < (int)from.size(); i++)
	{
		result.push_back(from[i]);
		for (bi = (*this)[arcs[from[i]].second].cbranch.begin(); bi != (*this)[arcs[from[i]].second].cbranch.end(); bi++)
		{
			ci = conditional_places.find(bi->first);
			if (ci != conditional_places.end())
			{
				oa = output_arcs(ci->second.second);
				for (j = 0; j < (int)oa.size(); j++)
				{
					bk = (*this)[arcs[oa[j]].second].cbranch.find(bi->first);

					same = false;
					for (k = 0; k < (int)from.size() && !same; k++)
					{
						bj = (*this)[arcs[from[k]].second].cbranch.find(bi->first);
						if (bj != (*this)[arcs[from[k]].second].cbranch.end() && bj->second == bk->second)
							same = true;
					}

					for (k = 0; k < (int)ex.size() && !same; k++)
					{
						bj = (*this)[arcs[ex[k]].second].cbranch.find(bi->first);
						if (bj != (*this)[arcs[ex[k]].second].cbranch.end() && bj->second == bk->second)
							same = true;
					}

					if (!same)
						result.push_back(oa[j]);
				}
			}
			else
			{
				cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
				cout << "Failed to find the conditional branch " << bi->first << "." << endl;
				print_branch_ids(&cout);
			}
		}
	}
	result.unique();

	/*cout << "Start {";
	for (i = 0; i < (int)from.size(); i++)
		cout << node_name(from[i]) << " ";
	cout << "} -> {";

	for (i = 0; i < (int)result.size(); i++)
		cout << node_name(result[i]) << " ";

	cout << "}" << endl;*/

	return result;
}

svector<int> petri::end_path(int to, svector<int> ex)
{
	svector<int> result, oa;
	smap<int, pair<int, int> >::iterator ci;
	smap<int, int>::iterator bi, bj, bk;
	int j, k;
	bool same;

	result.push_back(to);
	for (bi = S[to].cbranch.begin(); bi != S[to].cbranch.end(); bi++)
	{
		ci = conditional_places.find(bi->first);
		if (ci != conditional_places.end())
		{
			oa = input_nodes(ci->second.first);
			for (j = 0; j < (int)oa.size(); j++)
			{
				bk = T[index(oa[j])].cbranch.find(bi->first);
				same = (bk->second == bi->second);

				for (k = 0; k < (int)ex.size() && !same; k++)
				{
					bj = (*this)[ex[k]].cbranch.find(bi->first);
					if (bj != (*this)[ex[k]].cbranch.end() && bj->second == bk->second)
						same = true;
				}

				if (!same)
					result.push_back(oa[j]);
			}
		}
		else
		{
			cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
			cout << "Failed to find the conditional branch " << bi->first << "." << endl;
			print_branch_ids(&cout);
		}
	}
	result.unique();

	/*cout << "End {" << node_name(to) << "} -> {";
	for (j = 0; j < (int)result.size(); j++)
		cout << node_name(result[j]) << " ";
	cout << "}" << endl;*/

	return result;
}

svector<int> petri::end_path(svector<int> to, svector<int> ex)
{
	svector<int> result, oa;
	smap<int, pair<int, int> >::iterator ci;
	smap<int, int>::iterator bi, bj, bk;
	int i, j, k;
	bool same = false;

	for (i = 0; i < (int)to.size(); i++)
	{
		result.push_back(to[i]);
		for (bi = S[to[i]].cbranch.begin(); bi != S[to[i]].cbranch.end(); bi++)
		{
			ci = conditional_places.find(bi->first);
			if (ci != conditional_places.end())
			{
				oa = input_nodes(ci->second.first);
				for (j = 0; j < (int)oa.size(); j++)
				{
					bk = T[index(oa[j])].cbranch.find(bi->first);

					same = false;
					for (k = 0; k < (int)to.size() && !same; k++)
					{
						bj = (*this)[to[k]].cbranch.find(bi->first);
						if (bj != (*this)[to[k]].cbranch.end() && bj->second == bk->second)
							same = true;
					}

					for (k = 0; k < (int)ex.size() && !same; k++)
					{
						bj = (*this)[ex[k]].cbranch.find(bi->first);
						if (bj != (*this)[ex[k]].cbranch.end() && bj->second == bk->second)
							same = true;
					}

					if (!same)
						result.push_back(oa[j]);
				}
			}
			else
			{
				cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
				cout << "Failed to find the conditional branch " << bi->first << "." << endl;
				print_branch_ids(&cout);
			}
		}
	}
	result.unique();

	/*cout << "End {";
	for (i = 0; i < (int)to.size(); i++)
		cout << node_name(to[i]) << " ";
	cout << "} -> {";

	for (i = 0; i < (int)result.size(); i++)
		cout << node_name(result[i]) << " ";

	cout << "}" << endl;*/

	return result;
}

node &petri::operator[](int i)
{
	if (is_place(i))
		return S[i];
	else
		return T[index(i)];
}

void petri::print_dot(ostream *fout, sstring name)
{
	int i, j, k;
	sstring label;
	(*fout) << "digraph " << name << endl;
	(*fout) << "{" << endl;

	for (i = 0; i < (int)S.size(); i++)
		if (!dead(i))
		{
			(*fout) << "\tS" << i << " [label=\"" << sstring(i) << " ";
			label = S[i].index.print(vars);

			for (j = 0, k = 0; j < (int)label.size(); j++)
				if (label[j] == '|')
				{
					(*fout) << label.substr(k, j+1 - k) << "\\n";
					k = j+1;
				}

			(*fout) << label.substr(k) << "\"];" << endl;
		}

	for (i = 0; i < (int)T.size(); i++)
	{
		label = T[i].index.print(vars);
		if (!T[i].active)
			label = "[" + label + "]";
		label = sstring(i) + " " + label;
		if (label != "")
			(*fout) << "\tT" << i << " [shape=box] [label=\"" << label << "\"];" << endl;
		else
			(*fout) << "\tT" << i << " [shape=box];" << endl;
	}

	for (i = 0; i < (int)arcs.size(); i++)
		(*fout) << "\t" << node_name(arcs[i].first) << " -> " << node_name(arcs[i].second) << "[label=\" " << i << " \"];" <<  endl;

	(*fout) << "}" << endl;
}

void petri::print_branch_ids(ostream *fout)
{
	for (int i = 0; i < (int)S.size(); i++)
	{
		(*fout) << "S" << i << ": ";
		for (smap<int, int>::iterator j = S[i].pbranch.begin(); j != S[i].pbranch.end(); j++)
			(*fout) << "p{" << j->first << " " << j->second << "} ";
		for (smap<int, int>::iterator j = S[i].cbranch.begin(); j != S[i].cbranch.end(); j++)
			(*fout) << "c{" << j->first << " " << j->second << "} ";
		(*fout) << endl;
	}
	for (int i = 0; i < (int)T.size(); i++)
	{
		(*fout) << "T" << i << ": ";
		for (smap<int, int>::iterator j = T[i].pbranch.begin(); j != T[i].pbranch.end(); j++)
			(*fout) << "p{" << j->first << " " << j->second << "} ";
		for (smap<int, int>::iterator j = T[i].cbranch.begin(); j != T[i].cbranch.end(); j++)
			(*fout) << "c{" << j->first << " " << j->second << "} ";
		(*fout) << endl;
	}
	(*fout) << endl;
}

void petri::print_conflicts(ostream &fout, string name)
{
	smap<int, list<svector<int> > >::iterator i;
	list<svector<int> >::iterator lj;
	int j;

	fout << "Conflicts: " << name << endl;
	for (i = conflicts.begin(); i != conflicts.end(); i++)
	{
		fout << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (j = 0; j < (int)lj->size(); j++)
				fout << (*lj)[j] << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri::print_indistinguishables(ostream &fout, string name)
{
	smap<int, list<svector<int> > >::iterator i;
	list<svector<int> >::iterator lj;
	int j;

	fout << "Indistinguishables: " << name << endl;
	for (i = indistinguishable.begin(); i != indistinguishable.end(); i++)
	{
		fout << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (j = 0; j < (int)lj->size(); j++)
				fout << (*lj)[j] << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri::print_positive_conflicts(ostream &fout, string name)
{
	smap<int, list<svector<int> > >::iterator i;
	list<svector<int> >::iterator lj;
	int j;

	fout << "Negative Indistinguishables: " << name << endl;
	for (i = negative_indistinguishable.begin(); i != negative_indistinguishable.end(); i++)
	{
		fout << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (j = 0; j < (int)lj->size(); j++)
				fout << (*lj)[j] << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri::print_positive_indistinguishables(ostream &fout, string name)
{
	smap<int, list<svector<int> > >::iterator i;
	list<svector<int> >::iterator lj;
	int j;

	fout << "Positive Indistinguishables: " << name << endl;
	for (i = positive_indistinguishable.begin(); i != positive_indistinguishable.end(); i++)
	{
		fout << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (j = 0; j < (int)lj->size(); j++)
				fout << (*lj)[j] << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri::print_negative_conflicts(ostream &fout, string name)
{
	smap<int, list<svector<int> > >::iterator i;
	list<svector<int> >::iterator lj;
	int j;

	fout << "Positive Conflicts: " << name << endl;
	for (i = positive_conflicts.begin(); i != positive_conflicts.end(); i++)
	{
		fout << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (j = 0; j < (int)lj->size(); j++)
				fout << (*lj)[j] << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri::print_negative_indistinguishables(ostream &fout, string name)
{
	smap<int, list<svector<int> > >::iterator i;
	list<svector<int> >::iterator lj;
	int j;

	fout << "Negative Conflicts: " << name << endl;
	for (i = negative_conflicts.begin(); i != negative_conflicts.end(); i++)
	{
		fout << i->first << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (j = 0; j < (int)lj->size(); j++)
				fout << (*lj)[j] << " ";
			fout << "} ";
		}
		fout << endl;
	}
}
