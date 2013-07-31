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

node::node()
{
	owner = NULL;
}

node::node(int index, bool active, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	this->index = index;
	this->active = active;
	this->pbranch = pbranch;
	this->cbranch = cbranch;
	this->owner = owner;
}

node::~node()
{
	owner = NULL;
}

petri::petri()
{
	pbranch_count = 0;
	cbranch_count = 0;
}

petri::~petri()
{

}

int petri::new_transition(int root, bool active, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int ret = T.size() | 0x80000000;
	T.push_back(node(root, active, pbranch, cbranch, owner));
	Wp.addr();
	Wn.addr();
	return ret;
}

vector<int> petri::new_transitions(vector<int> root, bool active, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	vector<int> result;
	for (int i = 0; i < (int)root.size(); i++)
		result.push_back(new_transition(root[i], active, pbranch, cbranch, owner));
	return result;
}

int petri::new_place(int root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int ret = S.size();
	S.push_back(node(root, false, pbranch, cbranch, owner));
	Wp.addc();
	Wn.addc();
	return ret;
}

int petri::insert_transition(int from, int root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int t = new_transition(root, (owner != NULL && owner->kind() == "assignment"), pbranch, cbranch, owner);
	if (is_trans(from))
		cout << "Error: Illegal arc {T[" << index(from) << "], T[" << index(t) << "]}." << endl;
	else
	{
		Wn[from][index(t)]++;
		S[from].active = S[from].active || T[index(t)].active;
	}

	return t;
}

int petri::insert_transition(vector<int> from, int root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int t = new_transition(root, (owner != NULL && owner->kind() == "assignment"), pbranch, cbranch, owner);
	for (size_t i = 0; i < from.size(); i++)
	{
		if (is_trans(from[i]))
			cout << "Error: Illegal arc {T[" << index(from[i]) << "], T[" << index(t) << "]}." << endl;
		else
		{
			Wn[from[i]][index(t)]++;
			S[from[i]].active = S[from[i]].active || T[index(t)].active;
		}
	}

	return t;
}

void petri::insert_sv_at(int a, int root)
{
	map<int, int> pbranch, cbranch;
	instruction *owner;
	int t, p;

	if ((*this)[arcs[a].first].pbranch.size() > (*this)[arcs[a].first].pbranch.size() ||
		(*this)[arcs[a].first].cbranch.size() > (*this)[arcs[a].first].cbranch.size())
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
		Wp[arcs[a].second][arcs[a].first] = 0;
		connect(arcs[a].first, p);
		connect(p, t);
		connect(t, arcs[a].second);
		arcs[a].first = t;
	}
	else if (is_place(arcs[a].first) && is_trans(arcs[a].second))
	{
		Wn[arcs[a].first][arcs[a].second] = 0;
		connect(arcs[a].first, t);
		connect(t, p);
		connect(p, arcs[a].second);
		arcs[a].first = p;
	}
}

void petri::insert_sv_before(int from, int root)
{
	map<int, int>::iterator bi, bj;
	vector<pair<int, int> >::iterator ai;
	vector<int> input_places = input_arcs(from);
	int next;
	int i;
	instruction *ins = NULL, *prev;
	sequential *s = NULL;
	assignment *a = NULL;
	parallel *p = NULL;

	for (i = 0; i < (int)input_places.size(); i++)
		Wn[input_places[i]][index(from)] = 0;
	/*cout << index(from) << " " << root << endl;
	ins = T[index(from)].owner;
	prev = NULL;
	cout << (int)ins << " " << ins->kind() << endl;
	while (ins != NULL && ins->kind() != "sequential" && ins->kind() != "parallel")
	{
		cout << "round" << endl;
		prev = ins;
		ins = ins->parent;
		cout << (int)ins << endl;
		if (ins != NULL)
			cout << ins->kind() << endl;
	}
	cout << "loop" << endl;

	if (ins != NULL && ins->kind() == "sequential")
	{
		s = (sequential*)ins;
		a = new assignment(s, vars->get_name(values.var(root)) + (values.high(root) ? "+" : "-"), vars, s->tab, s->verbosity);
		s->instrs.insert(find(s->instrs.begin(), s->instrs.end(), prev), a);
	}
	else if (ins != NULL && ins->kind() == "parallel")
	{
		p = (parallel*)ins;
		s = new sequential();
		a = new assignment(s, vars->get_name(values.var(root)) + (values.high(root) ? "+" : "-"), vars, s->tab, s->verbosity);
		s->instrs.push_back(a);
		s->instrs.push_back(prev);
		s->verbosity = prev->verbosity;
		s->tab = prev->tab;
		s->from = prev->from;
		s->net = prev->net;
		s->vars = prev->vars;
		s->parent = p;

		p->instrs.erase(find(p->instrs.begin(), p->instrs.end(), prev));
		p->instrs.push_back(s);
		prev->parent = s;
	}
	else
		cout << "FUCK " << endl;*/

	for (i = 0; i < (int)input_places.size(); i++)
		S[input_places[i]].active = true;
	next = insert_transition(input_places, root, T[index(from)].pbranch, T[index(from)].cbranch, a);
	T[index(next)].active = true;
	next = insert_place(next, T[index(from)].pbranch, T[index(from)].cbranch, s);
	connect(next, from);
}

void petri::insert_sv_parallel(int from, int root)
{
	map<int, int>::iterator bi, bj;
	vector<int> ip = input_arcs(from);
	vector<int> op = output_arcs(from);
	vector<int> input_places;
	vector<int> output_places;
	vector<int> fvl;
	vector<int> tvl;
	int trans;
	int i;

	values.allvars(T[index(from)].index, &fvl);
	merge_vectors(&fvl, vars->x_channel(fvl));
	values.allvars(root, &tvl);

	// Implement the Physical Structure
	input_places = duplicate_nodes(ip);
	trans = insert_transition(input_places, root, T[index(from)].pbranch, T[index(from)].cbranch, T[index(from)].owner);
	for (i = 0; i < (int)op.size(); i++)
	{
		output_places.push_back(insert_place(trans, S[op[i]].pbranch, S[op[i]].cbranch, S[op[i]].owner));
		connect(output_places.back(), output_arcs(op[i]));
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

void petri::insert_sv_after(int from, int root)
{
	map<int, int>::iterator bi, bj;
	vector<int> output_places = output_arcs(from);
	vector<pair<int, int> >::iterator ai;
	vector<int> vl;
	int next;
	int i;

	for (i = 0; i < (int)output_places.size(); i++)
		Wp[output_places[i]][index(from)] = 0;

	next = insert_place(from, T[index(from)].pbranch, T[index(from)].cbranch, T[index(from)].owner);
	next = insert_transition(next, root, T[index(from)].pbranch, T[index(from)].cbranch, T[index(from)].owner);

	connect(next, output_places);
}

vector<int> petri::insert_transitions(int from, vector<int> root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int t, i;
	vector<int> result;
	for (i = 0; i < (int)root.size(); i++)
	{
		t = new_transition(root[i], (owner->kind() == "assignment"), pbranch, cbranch, owner);
		if (is_trans(from))
			cout << "Error: Illegal arc {T[" << index(from) << "], T[" << index(t) << "]}." << endl;
		else
		{
			Wn[from][index(t)]++;
			S[from].active = S[from].active || T[index(t)].active;
		}

		result.push_back(t);
	}
	return result;
}

vector<int> petri::insert_transitions(vector<int> from, vector<int> root, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int t, i, j;
	vector<int> result;
	for (i = 0; i < (int)root.size(); i++)
	{
		t = new_transition(root[i], (owner->kind() == "assignment"), pbranch, cbranch, owner);
		for (j = 0; j < (int)from.size(); j++)
		{
			if (is_trans(from[j]))
				cout << "Error: Illegal arc {T[" << index(from[j]) << "], T[" << index(t) << "]}." << endl;
			else
			{
				Wn[from[j]][index(t)]++;
				S[from[j]].active = S[from[j]].active || T[index(t)].active;
			}
		}

		result.push_back(t);
	}
	return result;
}

int petri::insert_dummy(int from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int t = new_transition(1, false, pbranch, cbranch, owner);
	if (is_trans(from))
		cout << "Error: Illegal arc {T[" << index(from) << "], T[" << index(t) << "]}." << endl;
	else
		Wn[from][index(t)]++;
	return t;
}

int petri::insert_dummy(vector<int> from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int t = new_transition(1, false, pbranch, cbranch, owner);
	for (size_t i = 0; i < from.size(); i++)
	{
		if (is_trans(from[i]))
			cout << "Error: Illegal arc {T[" << index(from[i]) << "], T[" << index(t) << "]}." << endl;
		else
			Wn[from[i]][index(t)]++;
	}
	return t;
}

int petri::insert_place(int from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int p = new_place(0, pbranch, cbranch, owner);

	if (is_place(from))
		cout << "Error: Illegal arc {S[" << from << "], S[" << p << "]}." << endl;
	else
		Wp[p][index(from)]++;
	return p;
}

int petri::insert_place(vector<int> from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	int p = new_place(0, pbranch, cbranch, owner);
	for (size_t i = 0; i < from.size(); i++)
	{
		if (is_place(from[i]))
			cout << "Error: Illegal arc {S[" << from[i] << "], S[" << p << "]}." << endl;
		else
			Wp[p][index(from[i])]++;
	}

	return p;
}

vector<int> petri::insert_places(vector<int> from, map<int, int> pbranch, map<int, int> cbranch, instruction *owner)
{
	vector<int> res;
	for (size_t i = 0; i < from.size(); i++)
		res.push_back(insert_place(from[i], pbranch, cbranch, owner));
	return res;
}

void petri::remove_place(int from)
{
	int i;
	if (is_place(from))
	{
		for (i = 0; i < (int)M0.size(); i++)
		{
			if (M0[i] > from)
				M0[i]--;
			else if (M0[i] == from)
			{
				M0.erase(M0.begin() + i);
				i--;
			}
		}
		S.erase(S.begin() + from);
		Wp.remc(from);
		Wn.remc(from);
	}
	else
		cout << "Error: " << index(from) << " must be a place." << endl;
}

void petri::remove_place(vector<int> from)
{
	sort(from.rbegin(), from.rend());
	for (int i = 0; i < (int)from.size(); i++)
		remove_place(from[i]);
}

void petri::propogate_marking_forward(int from)
{
	vector<int> ot = output_arcs(from);
	vector<int> op, ip;
	bool success;
	int i, j;
	for (i = 0; i < (int)ot.size(); i++)
	{
		if (T[ot[i]].index == 1)
		{
			ip = input_arcs(ot[i]);
			success = true;
			for (j = 0; j < (int)ip.size() && success; j++)
				if (find(M0.begin(), M0.end(), ip[j]) == M0.end())
					success = false;

			if (success)
			{
				op = output_arcs(ot[i]);
				M0.insert(M0.end(), op.begin(), op.end());
				for (j = 0; j < (int)op.size(); j++)
					propogate_marking_forward(op[i]);
			}
		}
	}
}

void petri::propogate_marking_backward(int from)
{
	vector<int> it = input_arcs(from);
	vector<int> ip, op;
	bool success;
	int i, j;
	for (i = 0; i < (int)it.size(); i++)
	{
		if (T[it[i]].index == 1)
		{
			op = output_arcs(it[i]);
			success = true;
			for (j = 0; j < (int)op.size() && success; j++)
				if (find(M0.begin(), M0.end(), op[j]) == M0.end())
					success = false;

			if (success)
			{
				M0.insert(M0.end(), ip.begin(), ip.end());
				for (j = 0; j < (int)ip.size(); j++)
					propogate_marking_backward(ip[i]);
			}
		}
	}
}

void petri::updateplace(int p)
{
	map<int, int>::iterator ji;
	vector<int> ia = input_arcs(p);
	vector<int> oa = output_arcs(p);
	vector<int> ip;
	vector<int> vl;
	int i, j, t;

	S[p].index = 0;
	for (i = 0; i < (int)ia.size(); i++)
	{
		ip = input_arcs(ia[i]);
		t = S[ip[0]].index;
		for (j = 1; j < (int)ip.size(); j++)
			t = values.apply_and(t, S[ip[j]].index);

		S[p].index = values.apply_or(values.transition(t, T[index(ia[i])].index), S[p].index);
	}
	for (ji = S[p].mutables.begin(); ji != S[p].mutables.end(); ji++)
		if (values.apply_and(S[p].index, ji->second) != S[p].index)
			S[p].index = values.smooth(S[p].index, ji->first);
}

bool petri::update(int p, vector<bool> *covered)
{
	map<int, int>::iterator ji;
	vector<int> ia = input_arcs(p);
	vector<int> oa = output_arcs(p);
	vector<int> op;
	vector<int> ip;
	vector<int> vl;
	int o;
	int i, j, t;
	bool ret = false;

	if (covered != NULL && covered->size() < S.size())
		covered->resize(S.size(), false);

	if (is_trans(p))
	{
		for (j = 0; j < (int)oa.size(); j++)
			ret = ret || update(oa[j], covered);
		return ret;
	}
	else if (ia.size() > 0)
	{
		o = S[p].index;
		S[p].index = 0;
		for (i = 0; i < (int)ia.size(); i++)
		{
			ip = input_arcs(ia[i]);

			for (j = 0, t = 1; j < (int)ip.size(); j++)
			{
				if (!(*covered)[ip[j]])
					t = values.apply_and(t, 1);
				else
					t = values.apply_and(t, S[ip[j]].index);
			}

			S[p].index = values.apply_or(values.transition(t, T[index(ia[i])].index), S[p].index);
		}
		for (ji = S[p].mutables.begin(); ji != S[p].mutables.end(); ji++)
			if (values.apply_and(S[p].index, ji->second) != S[p].index)
				S[p].index = values.smooth(S[p].index, ji->first);

		if (S[p].index != o || !(*covered)[p])
		{
			(*covered)[p] = true;
			for (i = 0; i < (int)oa.size(); i++)
			{
				op = output_arcs(oa[i]);
				for (j = 0; j < (int)op.size(); j++)
					update(op[j], covered);
			}
			return true;
		}
		else
			return false;
	}

	return false;
}

void petri::update()
{
	int i, j;
	int s = (int)M0.size();
	for (i = 0; i < s; i++)
	{
		propogate_marking_forward(i);
		propogate_marking_backward(i);
	}
	unique(&M0);

	for (i = 0; i < (int)S.size(); i++)
	{
		if (find(M0.begin(), M0.end(), i) != M0.end())
			S[i].index = 1;
		else
			S[i].index = 0;
	}
	bool ret = true;
	vector<int> ia;
	vector<bool> covered(S.size(), false);
	for (i = 0; i < (int)M0.size(); i++)
		covered[M0[i]] = true;

	while (ret)
	{
		ret = false;
		for (i = 0; i < (int)M0.size(); i++)
		{
			ia = output_arcs(M0[i]);
			for (j = 0; j < (int)ia.size(); j++)
				ret = ret || update(ia[j], &covered);
		}
	}
}

void petri::update_tail(int p)
{
	vector<int> old_tail;
	vector<int> ia, oa;
	int i;
	int s;
	if (is_trans(p))
	{
		old_tail = T[index(p)].tail;
		T[index(p)].tail.clear();

		ia = input_arcs(p);
		s = 1;
		for (i = 0, s = 1; i < (int)ia.size(); i++)
			s = values.apply_and(s, S[ia[i]].index);
		s = values.apply_and(s, T[index(p)].index);
		for (i = 0; (!T[index(p)].active || s > 0) && i < (int)ia.size(); i++)
		{
			T[index(p)].tail.insert(T[index(p)].tail.end(), S[ia[i]].tail.begin(), S[ia[i]].tail.end());
			T[index(p)].tail.push_back(ia[i]);
		}
		unique(&T[index(p)].tail);

		if (T[index(p)].tail != old_tail)
		{
			old_tail.clear();
			oa = output_arcs(p);
			for (i = 0; i < (int)oa.size(); i++)
				update_tail(oa[i]);
		}
	}
	else
	{
		old_tail = S[p].tail;
		S[p].tail.clear();

		ia = input_arcs(p);
		for (i = 0; i < (int)ia.size(); i++)
			S[p].tail.insert(S[p].tail.end(), T[index(ia[i])].tail.begin(), T[index(ia[i])].tail.end());
		unique(&S[p].tail);

		if (S[p].tail != old_tail)
		{
			old_tail.clear();
			oa = output_arcs(p);
			for (i = 0; i < (int)oa.size(); i++)
				update_tail(oa[i]);
		}
	}
}

void petri::connect(vector<int> from, vector<int> to)
{
	for (size_t i = 0; i < from.size(); i++)
		for (size_t j = 0; j < to.size(); j++)
			connect(from[i], to[j]);
}

void petri::connect(vector<int> from, int to)
{
	for (size_t i = 0; i < from.size(); i++)
		connect(from[i], to);
}

void petri::connect(int from, vector<int> to)
{
	for (size_t j = 0; j < to.size(); j++)
		connect(from, to[j]);
}

void petri::connect(int from, int to)
{
	if (is_place(from) && is_trans(to))
	{
		Wn[from][index(to)]++;
		S[from].active = S[from].active || T[index(to)].active;
	}
	else if (is_trans(from) && is_place(to))
		Wp[to][index(from)]++;
	else if (is_place(from) && is_place(to))
		cout << "Error: Illegal arc {S[" << from << "], S[" << to << "]}." << endl;
	else if (is_trans(from) && is_trans(to))
		cout << "Error: Illegal arc {T[" << index(from) << "], T[" << index(to) << "]}." << endl;
}

pair<int, int> petri::closest_input(vector<int> from, vector<int> to, path p)
{
	int count = 0;
	vector<int> next;
	vector<int> it, ip;
	vector<pair<int, int> > results;
	int i;
	int a;

	if (from.size() == 0)
		return pair<int, int>(arcs.size(), -1);

	for (i = 0; i < (int)arcs.size(); i++)
		if (find(to.begin(), to.end(), arcs[i].second) != to.end())
			next.push_back(i);

	while (next.size() == 1)
	{
		count++;
		a = next[0];


		if (p.nodes[a] > 0)
			return pair<int, int>(arcs.size(), -1);

		p.nodes[a]++;

		if (find(from.begin(), from.end(), a) != from.end())
			return pair<int, int>(count, a);

		next.clear();
		for (i = 0; i < (int)arcs.size(); i++)
			if (arcs[i].second == arcs[a].first)
				next.push_back(i);
	}

	for (i = 0; i < (int)next.size(); i++)
		if (p.nodes[next[i]] == 0)
		{
			count++;
			p.nodes[next[i]]++;
			results.push_back(closest_input(from, vector<int>(1, arcs[next[i]].first), p));
		}
	unique(&results);

	if (results.size() > 0)
		return pair<int, int>(results.front().first + count, results.front().second);
	else
		return pair<int, int>(arcs.size(), -1);
}

bool petri::dead(int from)
{
	if (is_place(from))
	{
		for (size_t i = 0; i < Wp[from].size(); i++)
			if (Wp[from][i] > 0)
				return false;

		for (size_t i = 0; i < Wn[from].size(); i++)
			if (Wn[from][i] > 0)
				return false;
	}
	else
	{
		from = index(from);
		for (size_t i = 0; i < Wn.size(); i++)
			if (Wn[i][from] > 0)
				return false;

		for (size_t i = 0; i < Wp.size(); i++)
			if (Wp[i][from] > 0)
				return false;
	}

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

int petri::base(vector<int> idx)
{
	int res = 1, i;
	for (i = 0; i < (int)idx.size(); i++)
		res = values.apply_and(res, S[idx[i]].index);

	return res;
}

bool petri::connected(int from, int to)
{
	vector<int> i1 = Wn[from];
	vector<int> o1 = Wp[to];
	vector<int> i2 = Wn[to];
	vector<int> o2 = Wp[from];

	for (int k = 0; k < (int)i1.size(); k++)
		if ((i1[k] > 0 && o1[k] > 0) || (i2[k] > 0 && o2[k] > 0) || (i1[k] > 0 && i2[k] > 0) || (o1[k] > 0 && o2[k] > 0))
			return true;

	return false;
}

int petri::psiblings(int p0, int p1)
{
	map<int, int>::iterator bi, bj;
	for (bi = (*this)[p0].pbranch.begin(); bi != (*this)[p0].pbranch.end(); bi++)
		for (bj = (*this)[p1].pbranch.begin(); bj != (*this)[p1].pbranch.end(); bj++)
			if (bi->first == bj->first && bi->second != bj->second)
				return bi->first;
	return -1;
}

int petri::csiblings(int p0, int p1)
{
	map<int, int>::iterator bi, bj;
	for (bi = (*this)[p0].cbranch.begin(); bi != (*this)[p0].cbranch.end(); bi++)
		for (bj = (*this)[p1].cbranch.begin(); bj != (*this)[p1].cbranch.end(); bj++)
			if (bi->first == bj->first && bi->second != bj->second)
				return bi->first;
	return -1;
}

bool petri::same_inputs(int p0, int p1)
{
	vector<int> it0, it1;
	bool diff_in = true;
	int k, l;

	it0 = input_arcs(p0);
	it1 = input_arcs(p1);

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
	vector<int> ot0, ot1;
	bool diff_out = true;
	int k, l;

	ot0 = output_arcs(p0);
	ot1 = output_arcs(p1);

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

vector<int> petri::duplicate_nodes(vector<int> from)
{
	vector<int> ret;
	for (size_t i = 0; i < from.size(); i++)
	{
		if (is_place(from[i]))
			ret.push_back(insert_place(input_arcs(from[i]), S[from[i]].pbranch, S[from[i]].cbranch, S[from[i]].owner));
		else
			ret.push_back(insert_transition(input_arcs(from[i]), T[index(from[i])].index, T[index(from[i])].pbranch, T[index(from[i])].cbranch, T[index(from[i])].owner));
	}
	sort(ret.rbegin(), ret.rend());
	return ret;
}

int petri::duplicate_node(int from)
{
	if (is_place(from))
		return insert_place(input_arcs(from), S[from].pbranch, S[from].cbranch, S[from].owner);
	else
		return insert_transition(input_arcs(from), T[index(from)].index, T[index(from)].pbranch, T[index(from)].cbranch, T[index(from)].owner);
}

int petri::merge_places(vector<int> from)
{
	map<int, int> pbranch;
	map<int, int> cbranch;
	vector<pair<int, int> >::iterator ai;
	int j, k, p;
	pbranch = S[from[0]].pbranch;
	cbranch = S[from[0]].cbranch;
	for (j = 0, p = 0; j < (int)from.size(); j++)
	{
		intersect(pbranch, S[from[j]].pbranch, &pbranch);
		intersect(cbranch, S[from[j]].cbranch, &cbranch);
		p = values.apply_or(p, S[from[j]].index);
	}

	p = new_place(p, pbranch, cbranch, NULL);
	for (j = 0; j < (int)from.size(); j++)
	{
		for (k = 0; k < (int)T.size(); k++)
		{
			Wn[p][k] = (Wn[p][k] || Wn[from[j]][k]);
			Wp[p][k] = (Wp[p][k] || Wp[from[j]][k]);
		}
	}

	return p;
}

int petri::merge_places(int a, int b)
{
	map<int, int> pbranch;
	map<int, int> cbranch;
	vector<pair<int, int> >::iterator ai;
	int k, p;

	intersect(S[a].pbranch, S[b].pbranch, &pbranch);
	intersect(S[a].cbranch, S[b].cbranch, &cbranch);
	p = new_place(values.apply_or(S[a].index, S[b].index), pbranch, cbranch, NULL);
	for (k = 0; k < (int)T.size(); k++)
	{
		Wn[p][k] = (Wn[a][k] || Wn[b][k]);
		Wp[p][k] = (Wp[a][k] || Wp[b][k]);
	}

	return p;
}

vector<int> petri::input_arcs(int from)
{
	vector<int> ret;
	if (is_place(from))
	{
		for (int i = 0; i < (int)Wp[from].size(); i++)
			if (Wp[from][i] > 0)
				ret.push_back(trans_id(i));
	}
	else
	{
		from = index(from);
		for (int i = 0; i < (int)Wn.size(); i++)
			if (Wn[i][from] > 0)
				ret.push_back(place_id(i));
	}
	sort(ret.rbegin(), ret.rend());
	return ret;
}

vector<int> petri::output_arcs(int from)
{
	vector<int> ret;
	if (is_place(from))
	{
		for (int i = 0; i < (int)Wn[from].size(); i++)
			if (Wn[from][i] > 0)
				ret.push_back(trans_id(i));
	}
	else
	{
		from = index(from);
		for (int i = 0; i < (int)Wp.size(); i++)
			if (Wp[i][from] > 0)
				ret.push_back(place_id(i));
	}
	return ret;
}

int petri::get_split_place(int merge_place, vector<bool> *covered)
{
	int i = merge_place, j, k, l;
	vector<int> ot, op, it, ip;
	vector<bool> c;
	bool loop;

	if ((*covered)[i])
		return -1;

	loop = true;
	it = input_arcs(i);
	if ((int)it.size() <= 0)
		return i;
	else if ((int)it.size() == 1)
	{
		ot = output_arcs(i);
		for (j = 0; j < (int)ot.size() && loop; j++)
		{
			op = output_arcs(ot[j]);
			for (k = 0; k < (int)op.size() && loop; k++)
				if (!(*covered)[op[k]])
					loop = false;
		}
	}

	(*covered)[i] = true;

	while (loop)
	{
		it = input_arcs(i);
		if ((int)it.size() <= 0)
			return i;
		else if ((int)it.size() == 1)
		{
			ip = input_arcs(it[0]);
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
				ip = input_arcs(it[l]);
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
		ot = output_arcs(i);
		for (j = 0; j < (int)ot.size() && loop; j++)
		{
			op = output_arcs(ot[j]);
			for (k = 0; k < (int)op.size() && loop; k++)
				if (!(*covered)[op[k]])
					loop = false;
		}
	}

	return i;
}

void petri::add_conflict_pair(map<int, list<vector<int> > > *c, int i, int j)
{
	map<int, list<vector<int> > >::iterator ri;
	list<vector<int> >::iterator li;
	vector<list<vector<int> >::iterator > gi;
	vector<int> group;
	int k;

	ri = c->find(i);
	if (ri != c->end())
	{
		gi.clear();
		for (li = ri->second.begin(); li != ri->second.end(); li++)
			for (k = 0; k < (int)li->size(); k++)
				if (connected(j, (*li)[k]))
				{
					gi.push_back(li);
					k = (int)li->size();
				}

		group = vector<int>(1, j);
		for (k = 0; k < (int)gi.size(); k++)
		{
			group.insert(group.end(), gi[k]->begin(), gi[k]->end());
			ri->second.erase(gi[k]);
		}
		unique(&group);
		ri->second.push_back(group);
	}
	else
		c->insert(pair<int, list<vector<int> > >(i, list<vector<int> >(1, vector<int>(1, j))));
}

void petri::gen_mutables()
{
	int i, j;
	vector<int> ia;
	vector<int> vl;

	map<int, int>::iterator bi, bj;

	for (i = 0; i < (int)S.size(); i++)
	{
		S[i].mutables.clear();

		for (j = 0; j < (int)T.size(); j++)
			if (T[j].active && psiblings(i, trans_id(j)) >= 0)
					values.extract(T[j].index, &S[i].mutables);

		ia = input_arcs(i);
		for (j = 0; j < (int)ia.size(); j++)
			if (T[index(ia[j])].active)
			{
				vl.clear();
				values.allvars(T[index(ia[j])].index, &vl);
				vars->x_channel(vl, &S[i].mutables);
			}
	}
}

void petri::gen_conditional_places()
{
	vector<int> it, ip;
	int sibs[2];
	vector<bool> covered(S.size(), false);
	int i, j, r;

	conditional_places.clear();
	for (i = 0; i < (int)S.size(); i++)
	{
		it = input_arcs(i);
		if ((int)it.size() > 1)
		{
			for (j = 0; j < (int)covered.size(); j++)
				covered[j] = (j == i ? true : false);

			for (j = 0, r = 0; j < 2 && r < (int)it.size(); r++)
			{
				ip = input_arcs(it[r]);
				if (ip.size() > 0)
					sibs[j++] = ip[0];
			}
			if ((int)(ip = input_arcs(it[0])).size() > 0 && j >= 2)
				if ((r = get_split_place(ip[0], &covered)) != -1)
					conditional_places.insert(pair<int, pair<int, int>>(csiblings(sibs[0], sibs[1]), pair<int, int>(i, r)));
		}
	}
}

void petri::gen_conflicts()
{
	map<int, list<vector<int> > >::iterator ri;
	list<vector<int> >::iterator li;
	vector<list<vector<int> >::iterator > gi;
	map<int, int>::iterator bi, bj;
	vector<int> group;
	vector<int> oa;
	vector<int> vl;
	vector<int> temp;
	int i, j;
	int t, s1, s;

	conflicts.clear();
	indistinguishable.clear();

	for (i = 0; i < (int)S.size(); i++)
	{
		oa = output_arcs(i);

		// INDEX
		vl.clear();
		t = 1;
		for (j = 0; j < (int)oa.size(); j++)
			if (T[index(oa[j])].active)
			{
				values.allvars(T[index(oa[j])].index, &vl);
				t = values.apply_and(t, T[index(oa[j])].index);
			}
		unique(&vl);
		s1 = values.smooth(S[i].index, vl);

		for (j = 0; j < (int)S.size(); j++)
		{
			/* States are conflicting if:
			 *  - they are not the same state
			 *  - one is not in the tail of another (the might as well be here case)
			 *  - the transition which causes the conflict is not a vacuous firing in the other state
			 *  - they are indistinguishable
			 *  - the two states do not exist in parallel
			 */

			// INDEX
			if (i != j && psiblings(i, j) < 0 && find(S[i].tail.begin(), S[i].tail.end(), j) == S[i].tail.end() && values.apply_and(s1, S[j].index) > 0)
			{
				// is it a conflicting state? (e.g. not vacuous)
				if (S[i].active && values.apply_and(t, S[j].index) == 0)
					add_conflict_pair(&conflicts, i, j);

				// it is at least an indistinguishable state at this point
				add_conflict_pair(&indistinguishable, i, j);
			}
		}
	}
}

void petri::gen_bubbleless_conflicts()
{
	map<int, list<vector<int> > >::iterator ri;
	list<vector<int> >::iterator li;
	vector<list<vector<int> >::iterator > gi;
	map<int, int>::iterator bi, bj;
	vector<int> group;
	vector<int> oa;
	vector<int> vlp, vln;
	vector<int> temp;
	int i, j;
	int tp, sp1, sp;
	int tn, sn1, sn;
	bool parallel;

	positive_conflicts.clear();
	positive_indistinguishable.clear();
	negative_conflicts.clear();
	negative_indistinguishable.clear();

	for (i = 0; i < (int)S.size(); i++)
	{
		oa = output_arcs(i);

		// POSITIVE
		vlp.clear();
		tp = 1;
		for (j = 0; j < (int)oa.size(); j++)
			if (T[index(oa[j])].active)
			{
				values.allvars(T[index(oa[j])].positive, &vlp);
				tp = values.apply_and(tp, T[index(oa[j])].negative);
			}
		unique(&vlp);
		sp1 = values.smooth(S[i].positive, vlp);

		// NEGATIVE
		vln.clear();
		tn = 1;
		for (j = 0; j < (int)oa.size(); j++)
			if (T[index(oa[j])].active)
			{
				values.allvars(T[index(oa[j])].negative, &vln);
				tn = values.apply_and(tn, T[index(oa[j])].positive);
			}
		unique(&vln);
		sn1 = values.smooth(S[i].negative, vln);

		for (j = 0; j < (int)S.size(); j++)
		{
			/* States are conflicting if:
			 *  - they are not the same state
			 *  - one is not in the tail of another (the might as well be here case)
			 *  - the transition which causes the conflict is not a vacuous firing in the other state
			 *  - they are indistinguishable
			 *  - the two states do not exist in parallel
			 */

			parallel = (psiblings(i, j) >= 0);
			// POSITIVE
			if (i != j && !parallel && find(S[i].tail.begin(), S[i].tail.end(), j) == S[i].tail.end() && values.apply_and(sp1, S[j].index) > 0)
			{
				// is it a conflicting state? (e.g. not vacuous)
				if (S[i].active && values.apply_and(tp, S[j].index) == 0)
					add_conflict_pair(&positive_conflicts, i, j);

				// it is at least an indistinguishable state at this point
				add_conflict_pair(&positive_indistinguishable, i, j);
			}

			// NEGATIVE
			if (i != j && !parallel && find(S[i].tail.begin(), S[i].tail.end(), j) == S[i].tail.end() && values.apply_and(sn1, S[j].index) > 0)
			{
				// is it a conflicting state? (e.g. not vacuous)
				if (S[i].active && values.apply_and(tn, S[j].index) == 0)
					add_conflict_pair(&negative_conflicts, i, j);

				// it is at least an indistinguishable state at this point
				add_conflict_pair(&negative_indistinguishable, i, j);
			}
		}
	}
}

void petri::gen_senses()
{
	int i;
	for (i = 0; i < (int)S.size(); i++)
	{
		S[i].positive = values.get_pos(S[i].index);
		S[i].negative = values.get_neg(S[i].index);
	}

	for (i = 0; i < (int)T.size(); i++)
	{
		T[i].positive = values.get_pos(T[i].index);
		T[i].negative = values.get_neg(T[i].index);
	}
}

void petri::trim_branch_ids()
{
	map<int, vector<int> > pbranch_counts;
	map<int, vector<int> > cbranch_counts;
	map<int, vector<int> >::iterator bci;
	map<int, int>::iterator bi;
	vector<map<int, int>::iterator > remove;
	int i, j;

	for (i = 0; i < pbranch_count; i++)
		pbranch_counts.insert(pair<int, vector<int> >(i, vector<int>()));

	for (i = 0; i < cbranch_count; i++)
		cbranch_counts.insert(pair<int, vector<int> >(i, vector<int>()));

	for (i = 0; i < (int)T.size(); i++)
	{
		for (bi = T[i].pbranch.begin(); bi != T[i].pbranch.end(); bi++)
			pbranch_counts[bi->first].push_back(bi->second);
		for (bi = T[i].cbranch.begin(); bi != T[i].cbranch.end(); bi++)
			cbranch_counts[bi->first].push_back(bi->second);
	}

	for (bci = pbranch_counts.begin(); bci != pbranch_counts.end(); bci++)
		unique(&bci->second);

	for (bci = cbranch_counts.begin(); bci != cbranch_counts.end(); bci++)
		unique(&bci->second);

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

	for (int i = 0; i < (int)T.size(); i++)
		update_tail(trans_id(i));
}

void petri::gen_arcs()
{
	arcs.clear();
	for (int i = 0; i < (int)S.size(); i++)
		for (int j = 0; j < (int)T.size(); j++)
		{
			if (Wn[i][j] > 0)
				arcs.push_back(pair<int, int>(i, trans_id(j)));
			if (Wp[i][j] > 0)
				arcs.push_back(pair<int, int>(trans_id(j), i));
		}
}

bool petri::trim()
{
	bool result = false;
	int i, j, k, l;
	vector<int> ia, oa;
	map<int, int>::iterator bi;
	vector<pair<int, int> >::iterator ai;
	bool vacuous;

	// Remove unreachable places
	/**
	 * A place is "unreachable" when there is no possible state encoding
	 * that would enable an input transition.
	 */
	for (i = 0; i < (int)S.size(); i++)
		while (i < (int)S.size() && S[i].index == 0)
			remove_place(i);

	i = 0;
	while (i < (int)T.size())
	{
		ia = input_arcs(trans_id(i));
		oa = output_arcs(trans_id(i));

		l = 1;
		for (j = 0; j < (int)ia.size(); j++)
			l = values.apply_and(l, S[ia[j]].index);

		vacuous = true;
		for (k = 0; k < (int)oa.size() && vacuous; k++)
			if (values.apply_and(S[oa[k]].index, l) != l)
				vacuous = false;

		// Impossible and dangling transitions
		/**
		 * A transition is "impossible" it can never fire, regardless of the state
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
			Wp.remr(i);
			Wn.remr(i);
			T.erase(T.begin() + i);
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
		 */
		else if (vacuous && ia.size() == 1)
		{
			for (k = 0; k < (int)oa.size(); k++)
				for (l = 0; l < (int)Wn[oa[k]].size(); l++)
				{
					Wn[oa[k]][l] = (Wn[oa[k]][l] || Wn[ia[0]][l]);
					Wp[oa[k]][l] = (Wp[oa[k]][l] || Wp[ia[0]][l]);
				}

			remove_place(ia[0]);

			Wp.remr(i);
			Wn.remr(i);
			T.erase(T.begin() + i);
			result = true;
		}
		else if (vacuous && oa.size() == 1 && input_arcs(oa[0]).size() == 1)
		{
			for (j = 0; j < (int)ia.size(); j++)
				for (l = 0; l < (int)Wn[oa[0]].size(); l++)
				{
					Wn[ia[j]][l] = (Wn[ia[j]][l] || Wn[oa[0]][l]);
					Wp[ia[j]][l] = (Wp[ia[j]][l] || Wp[oa[0]][l]);
				}

			remove_place(oa[0]);

			Wp.remr(i);
			Wn.remr(i);
			T.erase(T.begin() + i);
			result = true;
		}
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
	i = 0;
	while (i < (int)S.size())
	{
		vacuous = true;
		for (j = 0; j < (int)T.size() && vacuous; j++)
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

	/* Despite our best efforts to fix up the pbranch id's on the fly, we still
	 * need to clean up.
	 */
	trim_branch_ids();

	return result;
}

void petri::merge_conflicts()
{
	vector<int>				remove;
	vector<int>				ia, oa;
	vector<int>				vli, vlji, vljo;
	vector<vector<int> >	groups(S.size());
	vector<pair<int, int> > merge_value(S.size());	// the aggregated state encoding of input places and aggregated state encoding of output places for each group
	vector<pair<int, int> >::iterator ai;
	map<int, int>			pbranch, cbranch;
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
	for (i = 0; i < (int)S.size(); i++)
	{
		groups[i].push_back(i);
		if (input_arcs(i).size() == 0)
		{
			merge_value[i].first = S[i].index;
			merge_value[i].second = 0;
		}
		else if (output_arcs(i).size() == 0)
		{
			merge_value[i].first = 1;
			merge_value[i].second = S[i].index;
		}
	}

	for (i = 0; i < (int)S.size(); i++)
	{
		p = (int)groups.size();
		for (j = 0; j < p; j++)
			if (find(groups[j].begin(), groups[j].end(), i) == groups[j].end())
			{
				ia = input_arcs(i);
				oa = output_arcs(i);

				conflict = true;
				for (k = 0; k < (int)groups[j].size() && conflict; k++)
					if (values.apply_and(S[i].index, S[groups[j][k]].index) == 0)
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

					values.allvars(S[i].index, &vli);
					values.allvars(merge_value[j].first, &vlji);
					values.allvars(merge_value[j].second, &vljo);

					unique(&vli);
					unique(&vlji);
					unique(&vljo);

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
							merge_value[j].first = values.apply_and(merge_value[j].first, S[i].index);

						if (oa.size() == 0)
							merge_value[j].second = values.apply_or(merge_value[j].second, S[i].index);
					//}
				}
			}
	}

	/* The code segment above results in many duplicate groups and groups that
	 * are subsets of other groups. We need to remove these to make sure that we
	 * only get one place for each unique set of indistinguishable places.
	 */
	for (i = 0; i < (int)groups.size(); i++)
		unique(&(groups[i]));

	for (i = 0; i < (int)groups.size(); i++)
		for (j = 0; j < (int)groups.size(); j++)
			if (i != j && (includes(groups[i].begin(), groups[i].end(), groups[j].begin(), groups[j].end()) || (int)groups[j].size() <= 1))
			{
				groups.erase(groups.begin() + j);
				merge_value.erase(merge_value.begin() + j);
				j--;
				if (i > j)
					i--;
			}

	cout << "MERGES" << endl;
	for (i = 0; i < (int)groups.size(); i++)
	{
		for (j = 0; j < (int)groups[i].size(); j++)
			cout << groups[i][j] << " ";
		cout << endl;
	}
	cout << endl << endl;

	// Merge all of the places in each group identified above.
	for (i = 0; i < (int)groups.size(); i++)
	{
		pbranch.clear();
		cbranch.clear();
		for (j = 0; j < (int)groups[i].size(); j++)
		{
			pbranch.insert(S[groups[i][j]].pbranch.begin(), S[groups[i][j]].pbranch.end());
			cbranch.insert(S[groups[i][j]].cbranch.begin(), S[groups[i][j]].cbranch.end());
		}
		if (merge_value[i].second == 0)
			merge_value[i].second = 1;

		p = new_place(values.apply_and(merge_value[i].first, merge_value[i].second), pbranch, cbranch, NULL);
		for (j = 0; j < (int)groups[i].size(); j++)
		{
			for (k = 0; k < (int)T.size(); k++)
			{
				Wn[p][k] = (Wn[p][k] || Wn[groups[i][j]][k]);
				Wp[p][k] = (Wp[p][k] || Wp[groups[i][j]][k]);
			}
			remove.push_back(groups[i][j]);
		}
	}

	unique(&remove);
	for (i = (int)remove.size()-1; i >= 0; i--)
		remove_place(remove[i]);
}

void petri::zip()
{
	vector<int> it0, ot0, it1, ot1, remove;
	vector<vector<int> > groups;
	map<int, int> pbranch;
	map<int, int>::iterator bi, bj;
	vector<pair<int, int> >::iterator ai;
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
	for (i = 0; i < (int)S.size(); i++)
		groups.push_back(vector<int>(1, i));
	for (i = 0; i < (int)S.size(); i++)
	{
		p = (int)groups.size();
		for (j = 0; j < p; j++)
			if (find(groups[j].begin(), groups[j].end(), i) == groups[j].end() && same_inputs(i, groups[j][0]) && same_outputs(i, groups[j][0]))
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
	for (i = 0; i < (int)groups.size(); i++)
		unique(&(groups[i]));
	unique(&groups);
	for (i = 0; i < (int)groups.size(); i++)
		for (j = 0; j < (int)groups.size(); j++)
			if (i != j && (includes(groups[i].begin(), groups[i].end(), groups[j].begin(), groups[j].end()) || (int)groups[j].size() <= 1))
			{
				groups.erase(groups.begin() + j);
				j--;
				if (i > j)
					i--;
			}

	for (i = 0; i < (int)groups.size(); i++)
		if (groups[i].size() > 1)
		{
			merge_places(groups[i]);
			remove.insert(remove.end(), groups[i].begin(), groups[i].end());
		}

	unique(&remove);
	for (i = (int)remove.size()-1; i >= 0; i--)
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
	for (i = 0; i < (int)T.size(); i++)
		for (j = i+1; j < (int)T.size(); j++)
			if (T[i].index == T[j].index && input_arcs(trans_id(i)) == input_arcs(trans_id(j)))
			{
				for (k = 0; k < (int)S.size(); k++)
					Wp[k][i] = (Wp[k][i] || Wp[k][j]);

				intersect(T[i].pbranch, T[j].pbranch, &T[i].pbranch);
				intersect(T[i].cbranch, T[j].cbranch, &T[i].cbranch);

				remove.push_back(j);
			}

	unique(&remove);
	for (i = (int)remove.size()-1; i >= 0; i--)
	{
		T.erase(T.begin() + remove[i]);
		for (j = 0; j < (int)S.size(); j++)
		{
			Wn[j].erase(Wn[j].begin() + remove[i]);
			Wp[j].erase(Wp[j].begin() + remove[i]);
		}
	}

	/* Despite our best efforts to fix up the branch id's on the fly, we still
	 * need to clean up.
	 */
	trim_branch_ids();
}

path_space petri::get_paths(int from, int to, path p)
{
	path_space result(arcs.size());
	path temp;
	vector<int> next;
	vector<int> curr;
	int i, j;

	if (p.from.size() == 0)
		p.from.push_back(from);

	for (i = 0; i < (int)arcs.size(); i++)
		if (from == arcs[i].first)
			next.push_back(i);

	while (next.size() == 1)
	{
		from = next[0];
		if (p.nodes[from] > 1)
			return path_space(p.nodes.size());

		p.nodes[from]++;

		if (arcs[from].second == to)
		{
			p.to.push_back(to);
			return path_space(p.mask());
		}

		curr = next;
		next.clear();
		for (i = 0; i < (int)arcs.size(); i++)
			for (j = 0; j < (int)curr.size(); j++)
			if (arcs[curr[j]].second == arcs[i].first)
				next.push_back(i);
	}

	for (i = 0; i < (int)next.size(); i++)
	{
		temp = p;
		temp.nodes[next[i]]++;
		result.merge(get_paths(arcs[next[i]].second, to, temp));
	}

	return result;
}

path_space petri::get_paths(int from, vector<int> to, path p)
{
	path_space result(arcs.size());
	path temp;
	vector<int> next;
	vector<int> curr;
	int i, j;

	if (p.from.size() == 0)
		p.from.push_back(from);

	for (i = 0; i < (int)arcs.size(); i++)
		if (from == arcs[i].first)
			next.push_back(i);

	while (next.size() == 1)
	{
		from = next[0];
		if (p.nodes[from] > 1)
			return path_space(p.nodes.size());

		p.nodes[from]++;

		for (i = 0; i < (int)to.size(); i++)
			if (arcs[from].second == to[i])
			{
				p.to.push_back(to[i]);
				return path_space(p.mask());
			}

		curr = next;
		next.clear();
		for (i = 0; i < (int)arcs.size(); i++)
			for (j = 0; j < (int)curr.size(); j++)
			if (arcs[curr[j]].second == arcs[i].first)
				next.push_back(i);
	}

	for (i = 0; i < (int)next.size(); i++)
	{
		temp = p;
		temp.nodes[next[i]]++;
		result.merge(get_paths(arcs[next[i]].second, to, temp));
	}

	return result;
}

path_space petri::get_paths(int from, int to, vector<int> ex, path p)
{
	path_space result(arcs.size());
	path temp;
	vector<int> next;
	vector<int> curr;
	int i, j;

	if (p.from.size() == 0)
		p.from.push_back(from);

	for (i = 0; i < (int)arcs.size(); i++)
		if (from == arcs[i].first)
			next.push_back(i);

	while (next.size() == 1)
	{
		from = next[0];

		if (find(ex.begin(), ex.end(), arcs[from].first) != ex.end())
			return path_space(arcs.size());

		if (p.nodes[from] > 1)
			return path_space(p.nodes.size());

		p.nodes[from]++;

		if (arcs[from].second == to)
		{
			p.to.push_back(to);
			return path_space(p.mask());
		}

		curr = next;
		next.clear();
		for (i = 0; i < (int)arcs.size(); i++)
			for (j = 0; j < (int)curr.size(); j++)
			if (arcs[curr[j]].second == arcs[i].first)
				next.push_back(i);
	}

	for (i = 0; i < (int)next.size(); i++)
	{
		temp = p;
		temp.nodes[next[i]]++;
		result.merge(get_paths(arcs[next[i]].second, to, temp));
	}

	return result;
}

path_space petri::get_paths(int from, vector<int> to, vector<int> ex, path p)
{
	path_space result(arcs.size());
	path temp;
	vector<int> next;
	vector<int> curr;
	int i, j;

	if (p.from.size() == 0)
		p.from.push_back(from);

	for (i = 0; i < (int)arcs.size(); i++)
		if (from == arcs[i].first)
			next.push_back(i);

	while (next.size() == 1)
	{
		from = next[0];

		if (find(ex.begin(), ex.end(), arcs[from].first) != ex.end())
			return path_space(arcs.size());

		if (p.nodes[from] > 1)
			return path_space(p.nodes.size());

		p.nodes[from]++;

		for (i = 0; i < (int)to.size(); i++)
			if (arcs[from].second == to[i])
			{
				p.to.push_back(to[i]);
				return path_space(p.mask());
			}

		curr = next;
		next.clear();
		for (i = 0; i < (int)arcs.size(); i++)
			for (j = 0; j < (int)curr.size(); j++)
			if (arcs[curr[j]].second == arcs[i].first)
				next.push_back(i);
	}

	for (i = 0; i < (int)next.size(); i++)
	{
		temp = p;
		temp.nodes[next[i]]++;
		result.merge(get_paths(arcs[next[i]].second, to, temp));
	}

	return result;
}

path_space petri::get_paths(vector<int> t1, int t2, path p)
{
	path_space result(p.nodes.size());
	vector<int> ex;
	for (int i = 0; i < (int)t1.size(); i++)
	{
		ex = t1;
		ex.erase(ex.begin() + i);
		result.merge(get_paths(t1[i], t2, ex, p));
	}

	return result;
}

path_space petri::get_paths(vector<int> t1, vector<int> t2, path p)
{
	path_space result(p.nodes.size());
	vector<int> ex;
	for (int i = 0; i < (int)t1.size(); i++)
	{
		ex = t1;
		ex.erase(ex.begin() + i);
		result.merge(get_paths(t1[i], t2, ex, p));
	}
	return result;
}

void petri::filter_path_space(path_space *p)
{
	map<int, pair<int, int> >::iterator ci;
	vector<int> it, ip;
	list<path>::iterator pi;
	int i, j;
	int count, total;

	for (ci = conditional_places.begin(); ci != conditional_places.end(); ci++)
	{
		cout << ci->first << "{" << ci->second.first << ", " << ci->second.second << "}" << endl;
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
				if (pi->nodes[it[i]] > 0)
					filter_path(ci->second.second, it[i], &(*pi));

		for (int j = 0; j < (int)p->total.nodes.size(); j++)
			p->total.nodes[j] = 0;

		for (pi = p->paths.begin(); pi != p->paths.end(); pi++)
		{
			count = 0;
			for (int j = 0; j < (int)pi->nodes.size(); j++)
			{
				p->total.nodes[j] += pi->nodes[j];
				count += pi->nodes[j];
			}
			if (count == 0)
				pi = p->paths.erase(pi);
		}
	}
}

void petri::filter_path(int from, int to, path *p)
{
	vector<int> next;
	vector<int> it, ip;
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

void petri::zero_paths(path_space *paths, vector<int> from)
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

void petri::zero_ins(path_space *paths, vector<int> from)
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

void petri::zero_outs(path_space *paths, vector<int> from)
{
	for (int i = 0; i < (int)from.size(); i++)
		for (int j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].first == from[i])
				paths->zero(j);
}

node &petri::operator[](int i)
{
	if (is_place(i))
		return S[i];
	else
		return T[index(i)];
}

void petri::print_dot(ostream *fout, string name)
{
	int i, j;
	string label;
	gen_arcs();
	(*fout) << "digraph " << name << endl;
	(*fout) << "{" << endl;

	for (i = 0; i < (int)S.size(); i++)
		if (!dead(i))
		{
			if (S[i].index >= 0)
				label = values.expr(S[i].index, vars->get_names());//values.trace(S[i].index, vars->get_names());
			else
				label = "";

			for (j = 0; j < (int)label.size(); j++)
				if (label[j] == '|')
					label = label.substr(0, j+1) + "\\n" + label.substr(j+1);

			label = to_string(i) + " " + label;

			(*fout) << "\tS" << i << " [label=\"" << label << "\"];" << endl;
		}

	for (i = 0; i < (int)T.size(); i++)
	{
		label = values.expr(T[i].index, vars->get_names());
		label = to_string(i) + " " + label;
		if (label != "")
			(*fout) << "\tT" << i << " [shape=box] [label=\"" << label << "\"];" << endl;
		else
			(*fout) << "\tT" << i << " [shape=box];" << endl;
	}

	for (i = 0; i < (int)arcs.size(); i++)
		(*fout) << "\t" << (is_trans(arcs[i].first) ? "T" : "S") << index(arcs[i].first) << " -> " << (is_trans(arcs[i].second) ? "T" : "S") << index(arcs[i].second) << "[label=\" " << i << " \"];" <<  endl;

	(*fout) << "}" << endl;
}

void petri::print_petrify(string name)
{
	int i, j;
	vector<string> labels;
	map<string, int> labelmap;
	map<string, int>::iterator li;
	string label;
	FILE *file;
	map<string, variable>::iterator vi;
	bool first;

	file = fopen((name + ".g").c_str(), "wb");

	fprintf(file, ".model %s\n", name.c_str());

	first = true;
	for (vi = vars->global.begin(); vi != vars->global.end(); vi++)
		if (vi->second.arg && !vi->second.driven)
		{
			if (first)
			{
				fprintf(file, ".inputs");
				first = false;
			}
			fprintf(file, " %s", vi->second.name.c_str());
		}
	if (!first)
		fprintf(file, "\n");

	first = true;
	for (vi = vars->global.begin(); vi != vars->global.end(); vi++)
		if (vi->second.arg && vi->second.driven)
		{
			if (first)
			{
				fprintf(file, ".outputs");
				first = false;
			}
			fprintf(file, " %s", vi->second.name.c_str());
		}
	if (!first)
		fprintf(file, "\n");

	first = true;
	for (vi = vars->global.begin(); vi != vars->global.end(); vi++)
		if (!vi->second.arg)
		{
			if (first)
			{
				fprintf(file, ".internal");
				first = false;
			}
			fprintf(file, " %s", vi->second.name.c_str());
		}
	if (!first)
		fprintf(file, "\n");

	first = true;
	for (i = 0; i < (int)T.size(); i++)
	{
		label = values.expr(T[i].index, vars->get_names());
		if (label == "0" || label == "1")
		{
			if (first)
			{
				fprintf(file, ".dummy");
				first = false;
			}
			label = string("T") + to_string(i);
			fprintf(file, " %s", label.c_str());
		}
		li = labelmap.find(label);
		if (li == labelmap.end())
			labelmap.insert(pair<string, int>(label, 1));
		else
		{
			label += string("/") + to_string(li->second);
			li->second++;
		}

		labels.push_back(label);
	}
	if (!first)
		fprintf(file, "\n");

	string from;
	vector<string> to;

	fprintf(file, ".graph\n");
	for (i = 0; i < (int)Wp.size(); i++)
		for (j = 0; j < (int)Wp[i].size(); j++)
			if (Wp[i][j] > 0)
				fprintf(file, "%s S%d\n", labels[j].c_str(), i);

	for (i = 0; i < (int)Wn.size(); i++)
		for (j = 0; j < (int)Wn[i].size(); j++)
			if (Wn[i][j] > 0)
				fprintf(file, "S%d %s\n", i, labels[j].c_str());

	first = true;
	fprintf(file, ".marking {");
	for (i = 0; i < (int)M0.size(); i++)
	{
		if (!dead(M0[i]))
		{
			if (first)
				first = false;
			else
				fprintf(file, " ");
			fprintf(file, "S%d", M0[i]);
		}
	}
	fprintf(file, "}\n");
	fprintf(file, ".end\n");

	fclose(file);
}

void petri::print_branch_ids()
{
	for (int i = 0; i < (int)S.size(); i++)
	{
		cout << "S" << i << ": ";
		for (map<int, int>::iterator j = S[i].pbranch.begin(); j != S[i].pbranch.end(); j++)
			cout << "p{" << j->first << " " << j->second << "} ";
		for (map<int, int>::iterator j = S[i].cbranch.begin(); j != S[i].cbranch.end(); j++)
			cout << "c{" << j->first << " " << j->second << "} ";
		cout << endl;
	}
	for (int i = 0; i < (int)T.size(); i++)
	{
		cout << "T" << i << ": ";
		for (map<int, int>::iterator j = T[i].pbranch.begin(); j != T[i].pbranch.end(); j++)
			cout << "p{" << j->first << " " << j->second << "} ";
		for (map<int, int>::iterator j = T[i].cbranch.begin(); j != T[i].cbranch.end(); j++)
			cout << "c{" << j->first << " " << j->second << "} ";
		cout << endl;
	}
	cout << endl;
}
