/*
 * rule_space.cpp
 *
 *  Created on: Aug 23, 2013
 *      Author: nbingham
 */

#include "rule_space.h"

rule_space::rule_space()
{
	vars = NULL;
}

rule_space::~rule_space()
{
	vars = NULL;
}

void rule_space::insert(rule r)
{
	if ((int)rules.size() <= r.uid)
		rules.resize(r.uid+1);
	rules[r.uid] = r;
}

int rule_space::size()
{
	return (int)rules.size();
}

rule &rule_space::operator[](int i)
{
	return rules[i];
}

void rule_space::apply_one_of(logic *s, vector<int> a, int v)
{
	logic temp;
	minterm ex;
	int j;
	bool first = true;

	for (j = 0; j < (int)a.size(); j++)
		if (rules[a[j]].vars != NULL)
		{
			if (first)
			{
				ex = minterm(rules[a[j]].uid, 1-v);
				first = false;
			}
			else
				ex &= minterm(rules[a[j]].uid, 1-v);
		}

	if (((*s) & ex) != 0)
		for (j = 0; j < (int)a.size(); j++)
			if (rules[a[j]].vars != NULL && (rules[a[j]].guards[v] & (*s)) != 0)
				temp |= ((rules[a[j]].guards[v] & ex & (*s)) >> logic(rules[a[j]].uid, v)) | ((~rules[a[j]].guards[v] | ~ex) & (*s));

	if (temp.terms.size() != 0)
		*s = temp;
}

logic rule_space::apply(logic s, logic covered, string t)
{
	//cout << t << "Covered " << covered.print(vars) << endl;
	logic result, test;
	int i, j, k, l;
	vector<pair<vector<int>, int> > applicable;
	logic trans;
	logic temp;
	logic c;
	logic ex;
	minterm extemp;

	for (k = 0; k < (int)s.terms.size(); k++)
	{
		//cout << t << "Minterm " << s.terms[k].print(vars) << endl;
		trans.clear();
		trans.terms.push_back(minterm(1));
		for (i = 0; i < (int)excl.size(); i++)
		{
			ex.clear();
			for (j = 0; j < (int)excl[i].first.size(); j++)
				if (rules[excl[i].first[j]].vars != NULL)
				{
					extemp = 1;
					for (l = 0; l < (int)excl[i].first.size(); l++)
						if (l != j)
							extemp &= minterm(rules[excl[i].first[l]].uid, 1-excl[i].second);
					ex.push_back(extemp);
				}

			temp = trans;
			trans.clear();
			for (j = 0; j < (int)excl[i].first.size(); j++)
				if (rules[excl[i].first[j]].vars != NULL && (rules[excl[i].first[j]].guards[excl[i].second] & (s.terms[k] & ex.terms[j])) != 0)
					for (l = 0; l < (int)temp.terms.size(); l++)
						trans.terms.push_back(temp.terms[l] & minterm(excl[i].first[j], excl[i].second));

			if (trans.terms.size() == 0)
				trans = temp;
		}

		//cout << t << "Transitions " << trans.print(vars) << endl;

		temp.clear();
		test = s.terms[k];
		for (i = 0; i < (int)trans.terms.size(); i++)
			temp.terms.push_back((s.terms[k] >> trans.terms[i]).xoutnulls());
		temp.mccluskey();

		//cout << t << "Applied " << temp.print(vars) << endl;

		if (find(covered.begin(), covered.end(), s.terms[k]) != covered.end())
		{
			temp = covered;
		//	cout << t << "Found Cycle " << s.terms[k].print(vars) << endl;
		}
		else if (temp != s.terms[k])
		{
			c = covered;
			c.push_back(s.terms[k]);
			temp = apply(temp, c, t + "\t");
		}

		result |= temp;
	}

	result.mccluskey();
	//cout << t << "Result " << result.print(vars) << endl;

	return result;
}

void rule_space::generate_minterms(petri *net, flag_space *flags)
{
	vector<int> vl, ia;
	int i, j, k, tid;
	logic t;
	vector<bool> covered;
	minterm reset;
	map<int, logic> mutables;

	rules.resize(vars->global.size());
	for (i = 0; i < (int)net->T.size(); i++)
	{
		vl.clear();
		net->T[i].index.vars(&vl);
		unique(&vl);
		for (j = 0; net->T[i].active && j < (int)vl.size(); j++)
		{
			if ((net->T[i].index & logic(vl[j], 1)) == 0)
				rules[vl[j]].implicants[0].push_back(net->trans_id(i));
			else if ((net->T[i].index & logic(vl[j], 0)) == 0)
				rules[vl[j]].implicants[1].push_back(net->trans_id(i));
		}
	}

	int r = vars->get_uid("reset");

	for (j = 0; j < (int)net->M0.size(); j++)
	{
		for (k = 0; k < (int)net->S[net->M0[j]].index.terms.size(); k++)
		{
			if (j == 0 && k == 0)
				reset = net->S[net->M0[j]].index.terms[k];
			else
				reset |= net->S[net->M0[j]].index.terms[k];
		}
	}

	for (map<string, variable>::iterator vi = vars->global.begin(); vi != vars->global.end(); vi++)
		if (vi->second.driven)
		{
			rules[vi->second.uid].uid = vi->second.uid;
			rules[vi->second.uid].vars = vars;
			rules[vi->second.uid].net = net;
			rules[vi->second.uid].flags = flags;
			rules[vi->second.uid].guards[1] = 0;
			rules[vi->second.uid].guards[0] = 0;
			rules[vi->second.uid].explicit_guards[1] = 0;
			rules[vi->second.uid].explicit_guards[0] = 0;

			for (i = 0; i < 2; i++)
			{
				for (j = 0; j < (int)rules[vi->second.uid].implicants[i].size(); j++)
				{
					tid = rules[vi->second.uid].implicants[i][j];
					vl.clear();
					net->T[net->index(tid)].index.vars(&vl);
					unique(&vl);
					ia = net->input_nodes(tid);
					for (k = 0, t = 1; k < (int)ia.size(); k++)
					{
						t &= net->S[ia[k]].index;
						mutables = (k == 0 ? net->S[ia[k]].mutables : intersect(mutables, net->S[ia[k]].mutables));
					}
					t = t.hide(vl);
					rules[vi->second.uid].explicit_guards[i] |= t;

					covered.clear();
					//cout << "Start " << net->T[net->index(tid)].index.print(vars) <<  " ";
					//for (k = 0; k < (int)net->T[net->index(tid)].tail.size(); k++)
					//	cout << net->T[net->index(tid)].tail[k] << " ";
					//cout << endl;
					for (k = 0; k < (int)net->arcs.size(); k++)
						if (net->arcs[k].second == tid)
							rules[vi->second.uid].guards[i] |= rules[vi->second.uid].strengthen(k, &covered, logic(1), t, i, net->T[net->index(tid)].tail, mutables).second;
					//cout << endl;
				}
			}

			if (reset.val(vi->second.uid) == 1)
			{
				rules[vi->second.uid].guards[1] |= logic(r, 1);
				rules[vi->second.uid].guards[0] &= logic(r, 0);
			}
			else
			{
				rules[vi->second.uid].guards[0] |= logic(r, 1);
				rules[vi->second.uid].guards[1] &= logic(r, 0);
			}
		}
}

void rule_space::check(petri *net)
{
	int i, j, k, l, m;
	vector<int> oa, ia;
	bool ok;
	bool error;
	logic applied_guard;
	logic oguard;
	logic temp;
	bool para;
	bool imp;

	error = false;
	for (i = 0; i < (int)net->S.size(); i++)
	{
		oa = net->output_nodes(i);
		for (j = 0; j < (int)rules.size(); j++)
			for (k = 0; k < 2 && rules[j].vars != NULL && vars->find(rules[j].uid)->driven; k++)
			{
				para = false;
				for (l = 0; l < (int)rules[j].implicants[k].size() && !para; l++)
					if (net->psiblings(i, rules[j].implicants[k][l]) != -1)
						para = true;

				imp = false;
				for (l = 0; l < (int)oa.size() && !imp; l++)
					if (find(rules[j].implicants[k].begin(), rules[j].implicants[k].end(), oa[l]) != rules[j].implicants[k].end())
						imp = true;

				applied_guard = rules[j].guards[k] & net->S[i].index & logic(vars->get_uid("reset"), 0);
				// Does it fire when it shouldn't?
				if (!imp && !para && applied_guard != 0)
				{
					ok = false;
					// check if firing is vacuous
					temp = logic(rules[j].uid, 1-k);
					if (is_mutex(&applied_guard, &temp))
						ok = true;

					// check if firing is inside the tail and check to make sure that if it is in the tail,
					// it is correctly separated by the guards
					for (l = 0; l < (int)rules[j].implicants[k].size() && !ok; l++)
						if (net->T[net->index(rules[j].implicants[k][l])].is_in_tail(i))
						{
							oguard = 0;
							for (m = 0; m < (int)oa.size(); m++)
								oguard |= net->T[net->index(oa[m])].index;
							oguard = ~oguard;

							ok = (ok || is_mutex(&applied_guard, &oguard));
						}

					if (!ok)
					{
						cout << "Error: " << vars->get_name(rules[j].uid) << (k == 0 ? "-" : "+") << "\tfires when it shouldn't at state " << i << "\t" << applied_guard.print(vars) << endl;
						error = true;
					}
				}
				// Does it fire when it should?
				else if ((imp || para) && applied_guard == 0 && !is_mutex(&rules[j].explicit_guards[k], &net->S[i].index))
				{
					cout << "Error: " << vars->get_name(rules[j].uid) << (k == 0 ? "-" : "+") << "\tdoesn't fire when it should in " << (para ? "parallel " : "") << (imp ? "implicant " : "") << "state " << i << "\t";
					for (l = 0; l < (int)rules[j].guards[k].terms.size(); l++)
						for (m = 0; m < (int)net->S[i].index.terms.size(); m++)
						{
							if (l != 0 || m != 0)
								cout << " | ";
							cout << (rules[j].guards[k].terms[l] & net->S[i].index.terms[m]).print(vars);
						}
					cout << endl;
					error = true;
				}
			}
	}

	if (error)
		print_enable_graph(&cout, net, "g");
}

void rule_space::print(ostream *fout)
{
	int i, j;
	for (i = 0; i < (int)rules.size(); i++)
		if (rules[i].vars != NULL)
			(*fout) << rules[i];

	for (i = 0; i < (int)excl.size(); i++)
		if (excl[i].first.size() > 1)
		{
			(*fout) << "mk_excl" << (excl[i].second == 0 ? "lo" : "hi") << "(";
			for (j = 0; j < (int)excl[i].first.size(); j++)
			{
				if (j != 0)
					(*fout) << ",";
				(*fout) << vars->get_name(excl[i].first[j]);
			}
			(*fout) << ")" << endl;
		}
}

void rule_space::print_enable_graph(ostream *fout, petri *net, string name)
{
	int i, j, k;
	string label;
	net->gen_arcs();
	(*fout) << "digraph " << name << endl;
	(*fout) << "{" << endl;

	for (i = 0; i < (int)net->S.size(); i++)
		if (!net->dead(i))
		{
			(*fout) << "\tS" << i << " [label=\"" << to_string(i) << " ";
			/*label = net->S[i].index.print(vars);

			for (j = 0, k = 0; j < (int)label.size(); j++)
				if (label[j] == '|')
				{
					(*fout) << label.substr(k, j+1 - k) << "\\n";
					k = j+1;
				}

			(*fout) << label.substr(k) << "\\n";*/

			for (j = 0; j < (int)rules.size(); j++)
			{
				if (rules[j].vars != NULL && !is_mutex(&rules[j].guards[0], &net->S[i].index))
					cout << vars->get_name(rules[j].uid) << "-, ";
				if (rules[j].vars != NULL && !is_mutex(&rules[j].guards[1], &net->S[i].index))
					cout << vars->get_name(rules[j].uid) << "+, ";
			}

			(*fout) << "\"];" << endl;
		}

	for (i = 0; i < (int)net->T.size(); i++)
	{
		label = net->T[i].index.print(vars);
		label = to_string(i) + " " + label;
		if (label != "")
			(*fout) << "\tT" << i << " [shape=box] [label=\"" << label << "\"];" << endl;
		else
			(*fout) << "\tT" << i << " [shape=box];" << endl;
	}

	for (i = 0; i < (int)net->arcs.size(); i++)
		(*fout) << "\t" << (net->is_trans(net->arcs[i].first) ? "T" : "S") << net->index(net->arcs[i].first) << " -> " << (net->is_trans(net->arcs[i].second) ? "T" : "S") << net->index(net->arcs[i].second) << "[label=\" " << i << " \"];" <<  endl;

	(*fout) << "}" << endl;
}
