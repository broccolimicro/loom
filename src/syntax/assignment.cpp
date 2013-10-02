
#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "assignment.h"
#include "parallel.h"
#include "condition.h"

assignment::assignment()
{
	_kind = "assignment";
}

assignment::assignment(instruction *parent, sstring chp, variable_space *vars, flag_space *flags)
{
	this->_kind		= "assignment";
	this->chp		= chp;
	this->flags 	= flags;
	this->vars		= vars;
	this->parent	= parent;

	expand_shortcuts();
	parse();
}

assignment::~assignment()
{
	_kind = "assignment";
}

/* duplicate()
 *
 * This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *assignment::duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert)
{
	assignment *instr;

	instr 				= new assignment();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags 		= flags;
	instr->expr			= this->expr;
	instr->parent		= parent;

	int idx;
	sstring rep;

	smap<sstring, sstring>::iterator i, j;
	list<pair<sstring, sstring> >::iterator e;
	int k = 0, min, curr;
	while (k != instr->chp.npos)
	{
		j = convert.end();
		min = instr->chp.length();
		curr = 0;
		for (i = convert.begin(); i != convert.end(); i++)
		{
			curr = find_name(instr->chp, i->first, k);
			if (curr < min && curr >= 0)
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

	for (e = instr->expr.begin(); e != instr->expr.end(); e++)
	{
		k = 0;
		while (k != e->first.npos)
		{

			j = convert.end();
			min = e->first.length();
			curr = 0;
			for (i = convert.begin(); i != convert.end(); i++)
			{
				curr = find_name(e->first, i->first, k);
				if (curr < min && curr >= 0)
				{
					min = curr;
					j = i;
				}
			}

			if (j != convert.end())
			{
				rep = j->second;
				e->first.replace(min, j->first.length(), rep);
				if (e->first[min + rep.length()] == '[' && e->first[min + rep.length()-1] == ']')
				{
					idx = e->first.find_first_of("]", min + rep.length()) + 1;
					rep = flatten_slice(e->first.substr(min, idx - min));
					e->first.replace(min, idx - min, rep);
				}

				k = min + rep.length();
			}
			else
				k = e->first.npos;
		}

		k = 0;
		while (k != e->second.npos)
		{
			j = convert.end();
			min = e->second.length();
			curr = 0;
			for (i = convert.begin(); i != convert.end(); i++)
			{
				curr = find_name(e->second, i->first, k);
				if (curr < min && curr >= 0)
				{
					min = curr;
					j = i;
				}
			}

			if (j != convert.end())
			{
				rep = j->second;
				e->second.replace(min, j->first.length(), rep);
				if (e->second[min + rep.length()] == '[' && e->second[min + rep.length()-1] == ']')
				{
					idx = e->second.find_first_of("]", min + rep.length()) + 1;
					rep = flatten_slice(e->second.substr(min, idx - min));
					e->second.replace(min, idx - min, rep);
				}

				k = min + rep.length();
			}
			else
				k = e->second.npos;
		}
	}

	list<pair<sstring, sstring> >::iterator ei;
	variable *ev;
	for (ei = instr->expr.begin(); ei != instr->expr.end(); ei++)
	{
		ev = vars->find(ei->first);
		if (ev != NULL)
			ev->driven = true;
	}

	return instr;
}

/* Expand shortcut handles cases of implied syntax.
 * It runs before parsing, and translates user input from main.chp into a standard
 * form that parse can recognize. In assignment, the implied syntax is var+ and var-
 */
void assignment::expand_shortcuts()
{
	// Convert var+ to var:=1
	if(chp.find(":=") == chp.npos && chp.find("+") != chp.npos)
		chp = chp.substr(0, chp.find("+")) + ":=1";

	// Convert var- to var:=0
	if(chp.find(":=") == chp.npos && chp.find("-") != chp.npos)
		chp = chp.substr(0, chp.find("-")) + ":=0";
}

//Parse populates information about this assignment, such as
//TODO go through
void assignment::parse()
{
	int middle;
	sstring left_raw, right_raw;
	int i, j;
	int k, l;
	sstring left, right;
	variable *v;

	flags->inc();
	if (flags->log_base_hse())
		(*flags->log_file) << flags->tab << "Assignment:\t" + chp << endl;

	// Identify that this instruction is an assign.
	if (chp.find(":=") != chp.npos)
	{
		// Separate the two operands (the variable to be assigned and the value to assign)
		middle = chp.find(":=");
		left_raw = chp.substr(0, middle);
		right_raw = chp.substr(middle+2);
		for (i = left_raw.find_first_of(","), j = right_raw.find_first_of(","), k = 0, l = 0; i != left_raw.npos && j != right_raw.npos; i = left_raw.find_first_of(",", i+1), j = right_raw.find_first_of(",", j+1))
		{
			left = left_raw.substr(k, i-k);
			right = right_raw.substr(l, j-l);
			expr.push_back(pair<sstring, sstring>(left, right));
			k = i+1;
			l = j+1;

			v = vars->find(left);
			if (v != NULL)
				v->driven = true;
		}
		left = left_raw.substr(k);
		right = right_raw.substr(l);
		expr.push_back(pair<sstring, sstring>(left, right));

		v = vars->find(left);
		if (v != NULL)
			v->driven = true;
	}
	// If all else fails, complain to the user.
	else
		cerr << "Error: Instruction not handled: " << chp << endl;



	flags->dec();
}

void assignment::simulate()
{
	vars->increment_pcs(logic(), true);
}

void assignment::rewrite()
{

}

void assignment::reorder()
{

}

svector<int> assignment::generate_states(petri *n, rule_space *p, svector<int> f, smap<int, int> pbranch, smap<int, int> cbranch)
{
	list<pair<sstring, sstring> >::iterator ei, ej;
	svector<int> next, end;
	svector<int> allends;
	int pbranch_id;
	smap<int, int> npbranch;
	variable *v;
	int k;
	int l;

	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Assignment " << chp << endl;

	net  = n;
	prs = p;
	from = f;

	pbranch_id = net->pbranch_count;
	net->pbranch_count++;
	for (ei = expr.begin(), k = expr.size()-1; ei != expr.end(); ei++, k--)
	{
		v = vars->find(ei->first);
		if (v != NULL)
		{
			v->driven = true;

			npbranch = pbranch;
			if (expr.size() > 1)
				npbranch.insert(pair<int, int>(pbranch_id, (int)k));

			next.clear();
			next = (k == 0 ? from : net->duplicate_nodes(from));
			for (l = 0; l < (int)next.size(); l++)
				net->S[next[l]].pbranch = npbranch;

			end.clear();
			end.push_back(net->insert_transition(next, logic(v->uid, (uint32_t)(ei->second == "1")), npbranch, cbranch, this));
			allends.push_back(net->insert_place(end, npbranch, cbranch, this));
		}
		else
			cerr << "Error: Undefined variable " << ei->first << "." << endl;
	}
	uid.push_back(net->insert_dummy(allends, pbranch, cbranch, this));

	return uid;
}

void assignment::print_hse(sstring t, ostream *fout)
{
	if (expr.size() > 1)
		(*fout) << "(";

	list<pair<sstring, sstring> >::iterator i;
	for (i = expr.begin(); i != expr.end(); i++)
	{
		if (i != expr.begin())
			(*fout) << ",";
		if (i->second == "0")
			(*fout) << i->first << "-";
		else if (i->second == "1")
			(*fout) << i->first << "+";
	}

	if (expr.size() > 1)
		(*fout) << ")";
}
