/*
 * petri.cpp
 *
 *  Created on: May 12, 2013
 *      Author: nbingham
 */

#include "petri.h"
#include "vspace.h"
#include "../syntax/instruction.h"

node::node()
{
	owner = NULL;
}

node::node(int index, bool active, vector<int> mutables, map<int, int> branch, instruction *owner)
{
	this->index = index;
	this->active = active;
	this->mutables = mutables;
	this->branch = branch;
	this->owner = owner;
}

node::~node()
{
}

petri::petri()
{
	branch_count = 0;
}

petri::~petri()
{

}

int petri::new_transition(int root, bool active, map<int, int> branch, instruction *owner)
{
	int ret = T.size() | 0x80000000;
	T.push_back(node(root, active, vector<int>(), branch, owner));
	Wp.addr();
	Wn.addr();
	return ret;
}

vector<int> petri::new_transitions(vector<int> root, bool active, map<int, int> branch, instruction *owner)
{
	vector<int> result;
	for (int i = 0; i < (int)root.size(); i++)
		result.push_back(new_transition(root[i], active, branch, owner));
	return result;
}

int petri::new_place(int root, vector<int> mutables, map<int, int> branch, instruction *owner)
{
	int ret = S.size();
	S.push_back(node(root, false, mutables, branch, owner));
	Wp.addc();
	Wn.addc();
	return ret;
}

int petri::insert_transition(int from, int root, map<int, int> branch, instruction *owner)
{
	int t = new_transition(root, (owner->kind() == "assignment"), branch, owner);
	if (is_trans(from))
		cout << "Error: Illegal arc {T[" << index(from) << "], T[" << index(t) << "]}." << endl;
	else
	{
		Wn[from][index(t)]++;
		S[from].active = S[from].active || T[index(t)].active;
	}

	return t;
}

int petri::insert_transition(vector<int> from, int root, map<int, int> branch, instruction *owner)
{
	int t = new_transition(root, (owner->kind() == "assignment"), branch, owner);
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

void petri::insert_sv_before(int from, int root)
{
	map<int, int>::iterator bi, bj;
	vector<int> input_places = input_arcs(from);
	vector<int> mutables;
	int next;
	int i;

	for (i = 0; i < (int)T.size(); i++)
	{
		for (bi = T[i].branch.begin(); bi != T[i].branch.end(); bi++)
		{
			bj = T[index(from)].branch.find(bi->first);
			if (bj != T[index(from)].branch.end() && bj->second != bi->second)
				values.variable_list(T[i].index, &mutables);
		}
		unique(&mutables);
	}

	for (i = 0; i < (int)input_places.size(); i++)
		Wn[input_places[i]][index(from)] = 0;

	next = insert_transition(input_places, root, T[index(from)].branch, T[index(from)].owner);
	next = insert_place(next, mutables, T[index(from)].branch, T[index(from)].owner);
	connect(next, from);
}

void petri::insert_sv_parallel(int from, int root)
{
	map<int, int>::iterator bi, bj;
	vector<int> ip = input_arcs(from);
	vector<int> op = output_arcs(from);
	vector<int> input_places;
	vector<int> output_places;
	vector<int> mutables;
	vector<int> fvl;
	vector<int> tvl;
	int trans;
	int i;

	values.variable_list(T[index(from)].index, &fvl);
	merge_vectors(&fvl, vars->x_channel(fvl));
	values.variable_list(root, &tvl);

	// Implement the Physical Structure
	input_places = duplicate(ip);
	trans = insert_transition(input_places, root, T[index(from)].branch, T[index(from)].owner);
	for (i = 0; i < (int)op.size(); i++)
	{
		output_places.push_back(insert_place(trans, S[op[i]].mutables, S[op[i]].branch, S[op[i]].owner));
		connect(output_places.back(), output_arcs(op[i]));
	}

	// Update Branch IDs
	for (i = 0; i < (int)input_places.size(); i++)
	{
		S[ip[i]].branch.insert(pair<int, int>(branch_count, 0));
		S[input_places[i]].branch.insert(pair<int, int>(branch_count, 1));
	}
	T[index(from)].branch.insert(pair<int, int>(branch_count, 0));
	T[index(trans)].branch.insert(pair<int, int>(branch_count, 1));
	for (i = 0; i < (int)output_places.size(); i++)
	{
		S[op[i]].branch.insert(pair<int, int>(branch_count, 0));
		S[output_places[i]].branch.insert(pair<int, int>(branch_count, 1));
	}

	// Update the Mutables
	for (i = 0; i < (int)input_places.size(); i++)
	{
		merge_vectors(&S[input_places[i]].mutables, fvl);
		unique(&S[input_places[i]].mutables);
	}
	for (i = 0; i < (int)output_places.size(); i++)
	{
		merge_vectors(&S[output_places[i]].mutables, fvl);
		unique(&S[output_places[i]].mutables);
	}

	for (i = 0; i < (int)S.size(); i++)
	{
		for (bi = T[index(trans)].branch.begin(); bi != T[index(trans)].branch.end(); bi++)
		{
			bj = S[i].branch.find(bi->first);
			if (bj != S[i].branch.end() && bj->second != bi->second)
				values.variable_list(root, &S[i].mutables);
		}
		unique(&S[i].mutables);
	}

	branch_count++;
}

void petri::insert_sv_after(int from, int root)
{
	map<int, int>::iterator bi, bj;
	vector<int> output_places = output_arcs(from);
	vector<int> mutables;
	vector<int> tmutables;
	vector<int> vl;
	int next;
	int i;

	for (i = 0; i < (int)T.size(); i++)
	{
		for (bi = T[i].branch.begin(); bi != T[i].branch.end(); bi++)
		{
			bj = T[index(from)].branch.find(bi->first);
			if (bj != T[index(from)].branch.end() && bj->second != bi->second)
				values.variable_list(T[i].index, &mutables);
		}
		unique(&mutables);
	}

	tmutables = mutables;
	values.variable_list(T[index(from)].index, &vl);
	merge_vectors(&tmutables, vars->x_channel(vl));

	for (i = 0; i < (int)output_places.size(); i++)
	{
		Wp[output_places[i]][index(from)] = 0;
		S[output_places[i]].mutables = mutables;
	}

	next = insert_place(from, tmutables, T[index(from)].branch, T[index(from)].owner);
	next = insert_transition(next, root, T[index(from)].branch, T[index(from)].owner);

	connect(next, output_places);
}

vector<int> petri::insert_transitions(int from, vector<int> root, map<int, int> branch, instruction *owner)
{
	int t, i;
	vector<int> result;
	for (i = 0; i < (int)root.size(); i++)
	{
		t = new_transition(root[i], (owner->kind() == "assignment"), branch, owner);
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

vector<int> petri::insert_transitions(vector<int> from, vector<int> root, map<int, int> branch, instruction *owner)
{
	int t, i, j;
	vector<int> result;
	for (i = 0; i < (int)root.size(); i++)
	{
		t = new_transition(root[i], (owner->kind() == "assignment"), branch, owner);
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

int petri::insert_dummy(int from, map<int, int> branch, instruction *owner)
{
	int t = new_transition(1, false, branch, owner);
	if (is_trans(from))
		cout << "Error: Illegal arc {T[" << index(from) << "], T[" << index(t) << "]}." << endl;
	else
		Wn[from][index(t)]++;
	return t;
}

int petri::insert_dummy(vector<int> from, map<int, int> branch, instruction *owner)
{
	int t = new_transition(1, false, branch, owner);
	for (size_t i = 0; i < from.size(); i++)
	{
		if (is_trans(from[i]))
			cout << "Error: Illegal arc {T[" << index(from[i]) << "], T[" << index(t) << "]}." << endl;
		else
			Wn[from[i]][index(t)]++;
	}
	return t;
}

int petri::insert_place(int from, vector<int> mutables, map<int, int> branch, instruction *owner)
{
	vector<int> vl;
	int r = values.transition(base(input_arcs(from)), T[index(from)].index);
	if (T[index(from)].active)
	{
		values.variable_list(T[index(from)].index, &vl);
		r = values.smooth(r, vars->x_channel(vl));
	}
	r = values.smooth(r, mutables);
	int p = new_place(r, mutables, branch, owner);

	if (is_place(from))
		cout << "Error: Illegal arc {S[" << from << "], S[" << p << "]}." << endl;
	else
		Wp[p][index(from)]++;
	return p;
}

int petri::insert_place(vector<int> from, vector<int> mutables, map<int, int> branch, instruction *owner)
{
	int r = 1, q;
	vector<int> vl;
	if (from.size() > 0)
	{
		r = values.transition(base(input_arcs(from[0])), T[index(from[0])].index);
		if (T[index(from[0])].active)
		{
			values.variable_list(T[index(from[0])].index, &vl);
			r = values.smooth(r, vars->x_channel(vl));
		}

		for (int i = 1; i < (int)from.size(); i++)
		{
			q = values.transition(base(input_arcs(from[i])), T[index(from[i])].index);
			if (T[index(from[i])].active)
			{
				values.variable_list(T[index(from[i])].index, &vl);
				q = values.smooth(q, vars->x_channel(vl));
			}
			r = values.apply_or(r, q);
		}
	}
	r = values.smooth(r, mutables);

	int p = new_place(r, mutables, branch, owner);
	for (size_t i = 0; i < from.size(); i++)
	{
		if (is_place(from[i]))
			cout << "Error: Illegal arc {S[" << from[i] << "], S[" << p << "]}." << endl;
		else
			Wp[p][index(from[i])]++;
	}

	return p;
}

vector<int> petri::insert_places(vector<int> from, vector<int> mutables, map<int, int> branch, instruction *owner)
{
	vector<int> res;
	for (size_t i = 0; i < from.size(); i++)
		res.push_back(insert_place(from[i], mutables, branch, owner));
	return res;
}

void petri::remove_place(int from)
{
	if (is_place(from))
	{
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
	{
		if (is_place(from[i]))
		{
			S.erase(S.begin() + from[i]);
			Wp.remc(from[i]);
			Wn.remc(from[i]);
		}
		else
			cout << "Error: " << index(from[i]) << " must be a place." << endl;
	}
}

void petri::update(int p)
{
	vector<int> ia = input_arcs(p);
	vector<int> oa = output_arcs(p);
	vector<int> op;
	vector<int> ip;
	vector<int> vl;
	int o = S[p].index;
	int i, j, t;
	int q;

	if (is_trans(p))
		for (j = 0; j < (int)oa.size(); j++)
			update(oa[j]);
	else
	{
		ip = input_arcs(ia[0]);
		t = S[ip[0]].index;

		for (j = 1; j < (int)ip.size(); j++)
			t = values.apply_and(t, S[ip[j]].index);
		S[p].index = values.transition(t, T[index(ia[0])].index);
		if (T[index(ia[0])].active)
		{
			values.variable_list(T[index(ia[0])].index, &vl);
			S[p].index = values.smooth(S[p].index, vars->x_channel(vl));
		}

		for (i = 1; i < (int)ia.size(); i++)
		{
			ip = input_arcs(ia[i]);
			t = S[ip[0]].index;
			for (j = 1; j < (int)ip.size(); j++)
				t = values.apply_and(t, S[ip[j]].index);

			q = values.transition(t, T[index(ia[i])].index);
			if (T[index(ia[i])].active)
			{
				values.variable_list(T[index(ia[i])].index, &vl);
				q = values.smooth(q, vars->x_channel(vl));
			}
			S[p].index = values.apply_or(q, S[p].index);
		}
		S[p].index = values.smooth(S[p].index, S[p].mutables);

		if (S[p].index != o)
		{
			for (i = 0; i < (int)oa.size(); i++)
			{
				op = output_arcs(oa[i]);
				for (j = 0; j < (int)op.size(); j++)
					update(op[j]);
			}
		}
	}
}

void petri::update_tail(int p)
{
	vector<int> old_tail;
	vector<int> ia, oa;
	int i;
	if (is_trans(p))
	{
		old_tail = T[index(p)].tail;
		T[index(p)].tail.clear();

		ia = input_arcs(p);
		for (i = 0; !T[index(p)].active && i < (int)ia.size(); i++)
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
		{
			if (is_place(from[i]) && is_trans(to[j]))
			{
				Wn[from[i]][index(to[j])]++;
				S[from[i]].active = S[from[i]].active || T[index(to[j])].active;
			}
			else if (is_trans(from[i]) && is_place(to[j]))
				Wp[to[j]][index(from[i])]++;
			else if (is_place(from[i]) && is_place(to[j]))
				cout << "Error: Illegal arc {S[" << from[i] << "], S[" << to[j] << "]}." << endl;
			else if (is_trans(from[i]) && is_trans(to[j]))
				cout << "Error: Illegal arc {T[" << index(from[i]) << "], T[" << index(to[j]) << "]}." << endl;
		}

	for (size_t j = 0; j < to.size(); j++)
		update(to[j]);
}

void petri::connect(vector<int> from, int to)
{
	for (size_t i = 0; i < from.size(); i++)
	{
		if (is_place(from[i]) && is_trans(to))
		{
			Wn[from[i]][index(to)]++;
			S[from[i]].active = S[from[i]].active || T[index(to)].active;
		}
		else if (is_trans(from[i]) && is_place(to))
			Wp[to][index(from[i])]++;
		else if (is_place(from[i]) && is_place(to))
			cout << "Error: Illegal arc {S[" << from[i] << "], S[" << to << "]}." << endl;
		else if (is_trans(from[i]) && is_trans(to))
			cout << "Error: Illegal arc {T[" << index(from[i]) << "], T[" << index(to) << "]}." << endl;
	}

	update(to);
}

void petri::connect(int from, vector<int> to)
{
	for (size_t j = 0; j < to.size(); j++)
	{
		if (is_place(from) && is_trans(to[j]))
		{
			Wn[from][index(to[j])]++;
			S[from].active = S[from].active || T[index(to[j])].active;
		}
		else if (is_trans(from) && is_place(to[j]))
			Wp[to[j]][index(from)]++;
		else if (is_place(from) && is_place(to[j]))
			cout << "Error: Illegal arc {S[" << from << "], S[" << to[j] << "]}." << endl;
		else if (is_trans(from) && is_trans(to[j]))
			cout << "Error: Illegal arc {T[" << index(from) << "], T[" << index(to[j]) << "]}." << endl;
	}

	for (size_t j = 0; j < to.size(); j++)
		update(to[j]);
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

	update(to);
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
	int res = S[idx[0]].index;
	for (int i = 1; i < (int)idx.size(); i++)
		res = values.apply_and(res, S[idx[0]].index);

	return res;
}

bool petri::connected(int from, int to)
{
	vector<int> i1 = Wn[from];
	vector<int> o1 = Wp[to];
	vector<int> i2 = Wn[to];
	vector<int> o2 = Wp[from];

	for (int k = 0; k < (int)i1.size(); k++)
		if ((i1[k] > 0 && o1[k] > 0) || (i2[k] > 0 && o2[k] > 0))
			return true;

	return false;
}

vector<int> petri::duplicate(vector<int> from)
{
	vector<int> ret;
	for (size_t i = 0; i < from.size(); i++)
	{
		if (is_place(from[i]))
			ret.push_back(insert_place(input_arcs(from[i]), S[from[i]].mutables, S[from[i]].branch, S[from[i]].owner));
		else
			ret.push_back(insert_transition(input_arcs(from[i]), T[index(from[i])].index, T[index(from[i])].branch, T[index(from[i])].owner));
	}
	sort(ret.rbegin(), ret.rend());
	return ret;
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

void petri::trim()
{
	int i, j, k, l;
	vector<int> ia, oa;
	for (i = 0; i < (int)S.size(); i++)
		while (i < (int)S.size() && S[i].index == 0)
			remove_place(i);

	for (i = 0; i < (int)T.size(); i++)
	{
		ia = input_arcs(trans_id(i));
		oa = output_arcs(trans_id(i));
		while (i < (int)T.size() && (T[i].index < 2 || ia.size() == 0))
		{
			if (T[i].index == 0 || ia.size() == 0 || oa.size() == 0)
			{
				Wp.remr(i);
				Wn.remr(i);
				T.erase(T.begin() + i);
			}
			else if (T[i].index == 1 && ia.size() == 1)
			{
				for (j = 0; j < (int)ia.size(); j++)
				{
					for (k = 0; k < (int)oa.size(); k++)
						for (l = 0; l < (int)Wn[oa[k]].size(); l++)
						{
							Wn[oa[k]][l] += Wn[ia[j]][l];
							Wp[oa[k]][l] += Wp[ia[j]][l];
						}

					remove_place(ia[j]);
				}

				Wp.remr(i);
				Wn.remr(i);
				T.erase(T.begin() + i);
			}
			else if (T[i].index == 1 && oa.size() == 1)
			{
				for (k = 0; k < (int)oa.size(); k++)
				{
					for (j = 0; j < (int)ia.size(); j++)
						for (l = 0; l < (int)Wn[oa[k]].size(); l++)
						{
							Wn[ia[j]][l] += Wn[oa[k]][l];
							Wp[ia[j]][l] += Wp[oa[k]][l];
						}

					remove_place(oa[k]);
				}

				Wp.remr(i);
				Wn.remr(i);
				T.erase(T.begin() + i);
			}
			else
				i++;

			ia = input_arcs(trans_id(i));
			oa = output_arcs(trans_id(i));
		}
	}

}

void petri::tails()
{
	for (int i = 0; i < (int)S.size(); i++)
		S[i].tail.clear();
	for (int i = 0; i < (int)T.size(); i++)
		T[i].tail.clear();

	for (int i = 0; i < (int)T.size(); i++)
		update_tail(trans_id(i));
}

void petri::gen_conflicts()
{
	map<int, list<vector<int> > >::iterator ri;
	list<vector<int> >::iterator li;
	vector<list<vector<int> >::iterator > gi;
	vector<int> group;
	vector<int> oa;
	vector<int> vl;
	vector<int> temp;
	int i, j, k;
	int t, t1;

	conflicts.clear();
	indistinguishable.clear();

	for (i = 0; i < (int)S.size(); i++)
		if (S[i].active)
		{
			oa = output_arcs(i);

			vl.clear();
			t1 = 1;
			for (j = 0; j < (int)oa.size(); j++)
				if (T[index(oa[j])].active)
				{
					values.variable_list(T[index(oa[j])].index, &vl);
					t1 = values.apply_and(t1, T[index(oa[j])].index);
				}
			unique(&vl);
			t = values.smooth(S[i].index, vl);

			for (j = 0; j < (int)S.size(); j++)
			{
				cout << i << " " << j << " " << values.expr(t, vars->get_names()) << " " << values.expr(S[j].index, vars->get_names()) << endl;

				/* States are conflicting if:
				 *  - they are not the same state
				 *  - one is not in the tail of another (the might as well be here case)
				 *  - the transition which causes the conflict is not a vacuous firing in the other state
				 *  - they are indistinguishable
				 */
				if (i != j && find(S[i].tail.begin(), S[i].tail.end(), j) == S[i].tail.end() && values.apply_and(t, S[j].index) > 0)
				{
					// is it a conflicting state? (e.g. not vacuous)
					if (values.apply_and(t1, S[j].index) == 0)
					{
						ri = conflicts.find(i);
						if (ri != conflicts.end())
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
							conflicts.insert(pair<int, list<vector<int> > >(i, list<vector<int> >(1, vector<int>(1, j))));
					}

					// it is at least an indistinguishable state at this point
					ri = indistinguishable.find(i);
					if (ri != indistinguishable.end())
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
						indistinguishable.insert(pair<int, list<vector<int> > >(i, list<vector<int> >(1, vector<int>(1, j))));
				}
			}
		}
}

path_space petri::get_paths(int t1, int t2, path p)
{
	path_space result(p.nodes.size());
	vector<int> next;
	vector<int> ot, op;

	if (p.from.size() == 0 && p.to.size() == 0)
	{
		p.from.push_back(t1);
		p.to.push_back(t2);
	}

	next.push_back(t1);

	do
	{
		if (p.nodes[index(next[0])] > 0)
			return path_space();

		p.nodes[index(next[0])]++;

		if (next[0] == t2)
			return path_space(p);

		op = output_arcs(next[0]);
		next.clear();
		for (int i = 0; i < (int)op.size(); i++)
		{
			ot = output_arcs(op[i]);
			next.insert(next.end(), ot.begin(), ot.end());
		}
	} while (next.size() == 1);

	for (int j = 0; j < (int)next.size(); j++)
		result.merge(get_paths(next[j], t2, p));

	return result;
}

path_space petri::get_paths(int t1, vector<int> t2, path p)
{
	path_space result(p.nodes.size());
	vector<int> next;
	vector<int> ot, op;

	if (p.from.size() == 0)
		p.from.push_back(t1);

	next.push_back(t1);

	do
	{
		if (p.nodes[index(next[0])] > 0)
			return path_space();

		p.nodes[index(next[0])]++;

		if (find(t2.begin(), t2.end(), next[0]) != t2.end())
		{
			p.to.push_back(next[0]);
			return path_space(p);
		}

		op = output_arcs(next[0]);
		next.clear();
		for (int i = 0; i < (int)op.size(); i++)
		{
			ot = output_arcs(op[i]);
			next.insert(next.end(), ot.begin(), ot.end());
		}
	} while (next.size() == 1);

	for (int j = 0; j < (int)next.size(); j++)
		result.merge(get_paths(next[j], t2, p));

	return result;
}

path_space petri::get_paths(int t1, int t2, vector<int> ex, path p)
{
	path_space result(p.nodes.size());
	vector<int> next;
	vector<int> ot, op;

	if (p.from.size() == 0)
		p.from.push_back(t1);

	next.push_back(t1);

	do
	{
		if (p.nodes[index(next[0])] > 0 || find(ex.begin(), ex.end(), next[0]) != ex.end())
			return path_space();

		p.nodes[index(next[0])]++;

		if (t2 == next[0])
		{
			p.to.push_back(next[0]);
			return path_space(p);
		}

		op = output_arcs(next[0]);
		next.clear();
		for (int i = 0; i < (int)op.size(); i++)
		{
			ot = output_arcs(op[i]);
			next.insert(next.end(), ot.begin(), ot.end());
		}
	} while (next.size() == 1);

	for (int j = 0; j < (int)next.size(); j++)
		result.merge(get_paths(next[j], t2, ex, p));

	return result;
}

path_space petri::get_paths(int t1, vector<int> t2, vector<int> ex, path p)
{
	path_space result(p.nodes.size());
	vector<int> next;
	vector<int> ot, op;

	if (p.from.size() == 0)
		p.from.push_back(t1);

	next.push_back(t1);

	do
	{
		if (p.nodes[index(next[0])] > 0 || find(ex.begin(), ex.end(), next[0]) != ex.end())
			return path_space();

		p.nodes[index(next[0])]++;

		if (find(t2.begin(), t2.end(), next[0]) != t2.end())
		{
			p.to.push_back(next[0]);
			return path_space(p);
		}

		op = output_arcs(next[0]);
		next.clear();
		for (int i = 0; i < (int)op.size(); i++)
		{
			ot = output_arcs(op[i]);
			next.insert(next.end(), ot.begin(), ot.end());
		}
	} while (next.size() == 1);

	for (int j = 0; j < (int)next.size(); j++)
		result.merge(get_paths(next[j], t2, ex, p));

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

path petri::restrict_path(path p, vector<int> implicants)
{
	path result(implicants.size());
	vector<int> places;
	vector<int> temp;
	int success;

	for (int i = 0; i < (int)implicants.size(); i++)
	{
		places = input_arcs(implicants[i]);
		if (intersect(places, p.from).size() == 0 &&
			intersect(places, p.to).size() == 0)
		{
			temp = output_arcs(implicants[i]);
			places.insert(places.end(), temp.begin(), temp.end());

			success = 0x7FFFFFFF;
			for (int j = 0; j < (int)places.size() && success > 0; j++)
				success = min(p.nodes[places[j]], success);
			result.nodes[i] = success;
		}
	}

	return result;
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
		if (label != "")
			(*fout) << "\tT" << i << " [shape=box] [label=\"" << label << "\"];" << endl;
		else
			(*fout) << "\tT" << i << " [shape=box];" << endl;
	}

	for (i = 0; i < (int)Wp.size(); i++)
		for (j = 0; j < (int)Wp[i].size(); j++)
			if (Wp[i][j] > 0)
				(*fout) << "\tT" << j << " -> " << "S" << i << ";" <<  endl;

	for (i = 0; i < (int)Wn.size(); i++)
		for (j = 0; j < (int)Wn[i].size(); j++)
			if (Wn[i][j] > 0)
				(*fout) << "\tS" << i << " -> " << "T" << j << ";" <<  endl;

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
