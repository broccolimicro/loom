#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "guard.h"
#include "assignment.h"

guard::guard()
{
	_kind = "guard";
}

guard::guard(instruction *parent, string chp, vspace *vars, string tab, int verbosity)
{
	this->_kind		= "guard";
	this->chp		= chp;
	this->tab		= tab;
	this->verbosity = verbosity;
	this->vars		= vars;
	this->parent	= parent;

	expand_shortcuts();
	parse();
}

guard::~guard()
{
	_kind = "guard";
}

guard &guard::operator=(guard g)
{
	this->uid		= g.uid;
	this->chp		= g.chp;
	this->vars		= g.vars;
	this->net		= g.net;
	this->tab		= g.tab;
	this->verbosity	= g.verbosity;
	this->parent	= g.parent;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *guard::duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	guard *instr;

	instr 				= new guard();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;
	instr->parent		= parent;

	size_t idx;
	string rep;

	map<string, string>::iterator i, j;
	size_t k = 0, min, curr;
	while (k != instr->chp.npos)
	{
		j = convert.end();
		min = instr->chp.length();
		curr = 0;
		for (i = convert.begin(); i != convert.end(); i++)
		{
			curr = find_name(instr->chp, i->first, k);
			if (curr < min)
			{
				min = curr;
				j = i;
			}
		}

		if (j != convert.end())
		{
			rep = j->second;
			instr->chp.replace(min, j->first.length(), rep);
			if (instr->chp[min + rep.length()] == '[' && instr->chp[min + rep.length()-1] == ']')
			{
				idx = instr->chp.find_first_of("]", min + rep.length()) + 1;
				rep = flatten_slice(instr->chp.substr(min, idx - min));
				instr->chp.replace(min, idx - min, rep);
			}

			k = min + rep.length();
		}
		else
			k = instr->chp.npos;
	}

	instr->chp = expression(instr->chp).simple;

	return instr;
}

vector<int> guard::variant()
{
	return extract_ids(chp, vars);
}

vector<int> guard::active_variant()
{
	return vector<int>();
}

vector<int> guard::passive_variant()
{
	return extract_ids(chp, vars);
}

void guard::expand_shortcuts()
{
}

void guard::parse()
{
	// TODO Expand multi-bit guard expressions using operators
	chp = expression(chp).simple;
	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
		cout << tab << "Guard:\t" + chp << endl;
}

void guard::merge()
{

}

vector<int> build(petri *net, vector<int> transout, vector<int> transin, vector<int> root, instruction *owner, vspace *vars, vector<string> exp, map<int, int> branch, vector<int> filter)
{
	if (exp.size() == 0)
	{
		net->connect(root, transout);
		net->connect(transin, root);
		return vector<int>();
	}
	else
	{
		map<vector<string>, vector<int> > groups;
		map<vector<string>, vector<int> >::iterator groupi;
		vector<string> group;
		vector<string>::const_iterator vari;
		vector<string> outexp;
		vector<int> retout, retin;
		vector<int> intlist;
		vector<int> p, t, t1;
		size_t i;
		string temp;

		for (i = 0; i < transout.size() && i < exp.size(); i++)
		{
			group = extract_names(exp[i]);
			groupi = groups.find(group);
			if (groupi == groups.end())
				groupi = groups.insert(pair<vector<string>, vector<int> >(group, vector<int>())).first;

			groupi->second.push_back(i);
		}

		for (groupi = groups.begin(); groupi != groups.end(); groupi++)
		{
			p.clear();
			t.clear();
			t1.clear();
			p.push_back(net->new_place(0, filter, branch, owner));
			cout << "guard?" << endl;
			for (i = 0; i < groupi->second.size(); i++)
			{
				t.push_back(transout[groupi->second[i]]);
				if (transin.size() == transout.size())
					t1.push_back(transin[groupi->second[i]]);
			}
			net->connect(p, t);
			net->connect(t1, p);

			t.clear();
			t1.clear();
			for (vari = groupi->first.begin(); vari != groupi->first.end(); vari++)
			{
				intlist = net->new_transitions(net->values->build(expression(*vari, vars).expr), false, branch, owner);
				t.insert(t.end(), intlist.begin(), intlist.end());
				intlist = net->new_transitions(net->values->build(expression("~(" + *vari + ")", vars).expr), false, branch, owner);
				t1.insert(t1.end(), intlist.begin(), intlist.end());
				for (i = 0; i < groupi->second.size(); i++)
				{
					temp = remove_var(exp[groupi->second[i]], *vari);
					if (temp != "")
						outexp.push_back(temp);
				}
			}
			net->connect(t, p);
			net->connect(p, t1);
			retout.insert(retout.end(), t.begin(), t.end());
			retin.insert(retin.end(), t1.begin(), t1.end());
		}

		build(net, retout, retin, root, owner, vars, outexp, branch, filter);

		return retout;
	}
}

vector<int> guard::generate_states(petri *n, vector<int> f, map<int, int> branch, vector<int> filter)
{
	/* TODO If a variable in a guard has a definite value in the previous state (not 'X'), then what do we do?
	 * Choice 1: replace their occurrence in the guard with their current, constant value
	 * Choice 2: X them out in the state before
	 * Choice 3: Set them such that the guard evaluates to false in the state before
	 * Choice 4: Do choice 1, simplify the expression, then do choice 2
	 * Choice 5: Do choice 1, simplify the expression, then do choice 3
	 *
	 * Important cases to consider are:
	 * a,b := 1,2;
	 * c := a+b		<-- this has variables a and b in a guard right after they are assigned. I suggest choice 3
	 *
	 * chan4p0b a
	 * a.r := 1;
	 * a?			<-- This might happen in the HSE due to HSE optimizations... I suggest choice 3
	 *
	 * After executing choice 1, you will be left with a bunch impossible states (underscores). These
	 * represent the branches of conditions that will never happen. Since this state space represents
	 * the union of all execution paths and values, we can say that for sure. This could be a huge optimization,
	 * removing a bunch of hardware that will always run the same way.
	 *
	 * TODO modifying the previous state bypasses the variant x-out system employed by parallel, loop, and condition.
	 */

	/*if (filter.size == 0)
		filter = nullv(vars->size());

	space = g;
	from = init;

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << "Guard " << chp << endl;

	map<string, variable>::iterator vi;
	minterm s, temp;

	// Choice 1
	size_t k = 0, curr;
	string vname;
	temp = estimate(chp, vars);
	for (int i = 0; i < temp.size(); i++)
	{
		if (temp[i].data == "X" && (g->states[init][i].data == "0" || g->states[init][i].data == "1"))
		{
			vname = vars->get_name(i);
			k = 0;
			while (k < chp.length())
			{
				curr = find_name(chp, vname, k);
				if (curr == chp.npos)
					break;
				chp.replace(curr, vname.length(), g->states[init][i].data);
				k = curr + g->states[init][i].data.length();
			}
		}
	}
	chp = expression(chp).simple;*/

	// Choice 2
	//g->states[init] = g->states[init] || estimate(chp, vars);

	// Choice 3
	//g->states[init] = g->states[init] && solve(expression("~(" + chp + ")").simple, vars, "", -1);


#ifdef METHOD_PETRIFY_SIMPLE
	chp = expression(chp).simple;

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << "Guard " << chp << endl;

	net = n;
	from = f;

	string minterm, var;
	int minterm_trans;
	variable *vptr;
	size_t i = 0, j = 0;
	size_t k = 0, l = 0;
	while ((i = chp.find_first_of("|", j)) != string::npos)
	{
		minterm = chp.substr(j, i-j);
		minterm_trans = net->insert_dummy(from, b, this);
		uid.push_back(minterm_trans);
		k = 0;
		l = 0;
		while ((k = minterm.find_first_of("&", l)) != string::npos)
		{
			if (minterm[l] == '~')
			{
				var = minterm.substr(l+1, k-l-1);
				vptr = vars->find(var);
				if (vptr != NULL)
				{
					net->connect(vptr->state0, minterm_trans);
					net->connect(minterm_trans, vptr->state0);
				}
			}
			else
			{
				var = minterm.substr(l, k-l);
				vptr = vars->find(var);
				if (vptr != NULL)
				{
					net->connect(vptr->state1, minterm_trans);
					net->connect(minterm_trans, vptr->state1);
				}
			}
			l = k+1;
		}

		if (minterm[l] == '~')
		{
			var = minterm.substr(l+1);
			vptr = vars->find(var);
			if (vptr != NULL)
			{
				net->connect(vptr->state0, minterm_trans);
				net->connect(minterm_trans, vptr->state0);
			}
		}
		else
		{
			var = minterm.substr(l);
			vptr = vars->find(var);
			if (vptr != NULL)
			{
				net->connect(vptr->state1, minterm_trans);
				net->connect(minterm_trans, vptr->state1);
			}
		}

		j = i+1;
	}

	minterm = chp.substr(j);
	minterm_trans = net->insert_dummy(from, b, this);
	uid.push_back(minterm_trans);
	k = 0;
	l = 0;
	while ((k = minterm.find_first_of("&", l)) != string::npos)
	{
		if (minterm[l] == '~')
		{
			var = minterm.substr(l+1, k-l-1);
			vptr = vars->find(var);
			if (vptr != NULL)
			{
				net->connect(vptr->state0, minterm_trans);
				net->connect(minterm_trans, vptr->state0);
			}
		}
		else
		{
			var = minterm.substr(l, k-l);
			vptr = vars->find(var);
			if (vptr != NULL)
			{
				net->connect(vptr->state1, minterm_trans);
				net->connect(minterm_trans, vptr->state1);
			}
		}
		l = k+1;
	}

	if (minterm[l] == '~')
	{
		var = minterm.substr(l+1);
		vptr = vars->find(var);
		if (vptr != NULL)
		{
			net->connect(vptr->state0, minterm_trans);
			net->connect(minterm_trans, vptr->state0);
		}
	}
	else
	{
		var = minterm.substr(l);
		vptr = vars->find(var);
		if (vptr != NULL)
		{
			net->connect(vptr->state1, minterm_trans);
			net->connect(minterm_trans, vptr->state1);
		}
	}
#endif

#ifdef METHOD_PETRIFY_EXP
	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << "Guard " << chp << endl;

	from = f;
	net = n;
	vector<string> exp;
	exp.push_back(chp);
	uid.push_back(net->new_transition(1, b, this));

	build(net, uid, vector<int>(), from, b, this, vars, exp, filter);
#endif

#ifdef METHOD_NORMAL
	vector<int> temp;
	from = f;
	net = n;
	expression e(chp, vars);
	chp = e.simple;

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << "Guard " << chp << endl;

	temp = net->insert_transitions(from, net->values->build(e.expr), branch, this);
	uid.insert(uid.end(), temp.begin(), temp.end());
#endif


	return uid;
}

void guard::insert_instr(int uid, int nid, instruction *instr)
{
}

void guard::print_hse(string t, ostream *fout)
{
	(*fout) << chp;
}
