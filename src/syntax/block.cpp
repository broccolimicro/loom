/*
 * block.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "block.h"
#include "parallel.h"
#include "conditional.h"
#include "loop.h"

block::block()
{
	_kind = "block";
	chp = "";
}

block::block(string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	this->_kind = "block";
	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->vars = vars;

	expand_shortcuts();
	parse();
}

block::~block()
{
	chp = "";
	_kind = "block";

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
}

block &block::operator=(block b)
{
	this->chp		= b.chp;
	this->instrs	= b.instrs;
	this->rules		= b.rules;
	this->vars		= b.vars;
	this->tab		= b.tab;
	this->verbosity	= b.verbosity;
	return *this;
}

void block::init(string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	this->_kind = "block";
	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->vars = vars;

	expand_shortcuts();
	parse();
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *block::duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	block *instr;

	instr 				= new block();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;

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

	list<instruction*>::iterator l;
	for (l = instrs.begin(); l != instrs.end(); l++)
		instr->instrs.push_back((*l)->duplicate(vars, convert, tab+"\t", verbosity));

	return instr;
}

void block::expand_shortcuts()
{
}

void block::parse()
{
	if (verbosity >= VERB_PARSE)
		cout << tab << "Block: " << chp << endl;

	string				raw_instr;	// chp of a sub block
	string::iterator	i, j;
	bool				para = false;
	int					depth[3] = {0};
	size_t				k;

	// Parse the instructions, making sure to stay in the current scope (outside of any bracket/parenthesis)
	for (i = chp.begin(), j = chp.begin(); i != chp.end()+1; i++)
	{
		if (i != chp.end())
		{
			if (*i == '(')
				depth[0]++;
			else if (*i == '[')
				depth[1]++;
			else if (*i == '{')
				depth[2]++;
			else if (*i == ')')
				depth[0]--;
			else if (*i == ']')
				depth[1]--;
			else if (*i == '}')
				depth[2]--;
		}

		// We are in the current scope, and the current character
		// is a semicolon or the end of the chp string. This is
		// the end of an instruction.
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == ';' || i == chp.end()))
		{
			// Get the instruction string.
			raw_instr = chp.substr(j-chp.begin(), i-j);

			// This sub block is a set of parallel sub sub blocks. s0 || s1 || ... || sn
			if (para && raw_instr.length() > 0)
				push(new parallel(raw_instr, vars, tab+"\t", verbosity));
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')' && raw_instr.length() > 0)
				push(new block(raw_instr.substr(1, raw_instr.length()-2), vars, tab+"\t", verbosity));
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(new loop(raw_instr, vars, tab+"\t", verbosity));
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(new conditional(raw_instr, vars, tab+"\t", verbosity));
			// This sub block is a variable instantiation.
			else if (vars->vdef(raw_instr) && raw_instr.length() > 0)
				push(expand_instantiation(raw_instr, vars, NULL, tab+"\t", verbosity, true));
			// This sub block is a communication instantiation.
			else if ((k = raw_instr.find_first_of("?!@")) != raw_instr.npos && raw_instr.find(":=") == raw_instr.npos && raw_instr.length() > 0)
				push(add_unique_variable(raw_instr.substr(0, k) + "._fn", "(" + (k+1 < raw_instr.length() ? raw_instr.substr(k+1) : "") + ")", vars->get_type(raw_instr.substr(0, k)) + ".operator" + raw_instr[k] + "()", vars, tab, verbosity).second);
			// This sub block is an assignment instruction.
			else if ((raw_instr.find(":=") != raw_instr.npos || raw_instr[raw_instr.length()-1] == '+' || raw_instr[raw_instr.length()-1] == '-') && raw_instr.length() > 0)
				push(expand_assignment(raw_instr, vars, tab+"\t", verbosity));
			else if (raw_instr.find("skip") == raw_instr.npos && raw_instr.length() > 0)
				push(new guard(raw_instr, vars, tab, verbosity));

			j = i+1;
			para = false;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp string. This is
		// the middle of a parallel sub block.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
			para = true;
	}
}

int block::generate_states(graph *trans, int init)
{
	cout << tab << "Block " << chp << endl;
	list<instruction*>::iterator instr_iter;
	instruction *instr;

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = *instr_iter;
		init = instr->generate_states(trans, init);
	}

	return init;
}

void block::generate_prs()
{

}

void block::generate_statevars()
{
	//TODO: Remember "factoring" idea

}

/* This function cleans up all of the memory allocated
 * during parsing, and prepares for the next parsing.
 */
void block::clear()
{
	chp = "";
	_kind = "block";

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
	rules.clear();
}

void block::print_hse()
{
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin())
			cout << ";";
		(*i)->print_hse();
	}
}

void block::push(instruction *i)
{
	if (i == NULL)
		return;

	list<instruction*>::iterator j;
	if (i->kind() == "parallel")
	{
		if (((parallel*)i)->instrs.size() <= 1)
		{
			for (j = ((parallel*)i)->instrs.begin(); j != ((parallel*)i)->instrs.end(); j++)
				push(*j);
			((parallel*)i)->instrs.clear();
			delete (parallel*)i;
		}
		else
			instrs.push_back(i);
	}
	else if (i->kind() == "block")
	{
		for (j = ((block*)i)->instrs.begin(); j != ((block*)i)->instrs.end(); j++)
			push(*j);
		((block*)i)->instrs.clear();
		delete (block*)i;
	}
	else
		instrs.push_back(i);
}

/*bool cycle(rule start, rule end, list<rule> *prs)
{
	list<rule>::iterator ri;

	bool result = false;
	if (end.left.var.find(start.right.var.substr(0, start.right.var.length()-1)) != end.left.var.npos)
		result = true;
	else
		for (ri = prs->begin(); ri != prs->end(); ri++)
			if (end.left.var.find(ri->right.var.substr(0, ri->right.var.length() - 1)) != end.left.var.npos)
				result = result || cycle(start, *ri, prs);

	return result;
}*/

/* This function generates a set of production rules for the given state space and variable space.
 * It uses two primary measurements to help with this: the count, and the strict count. The count
 * is the number of states in a state space that are in the set {1, X}. The strict count is the number
 * of states in a state space that are in the set {1}.
 *
 * This doesn't always do a perfect job. However, it does the best it can given the state space it has.
 * It is the job of the state variable generation algorithm and the handshaking reshuffling algorithms
 * to remove all of the conflicts that this algorithm leaves behind.
 *
 * The handshaking reshuffling algorithm has not yet been completed.
 */
/*list<rule> production_rule(list<instruction*> instrs, map<string, space> states, string tab, int verbosity)
{
	list<rule> prs;
	list<instruction*>::iterator ii;
	list<rule>::iterator ri, rj;
	map<string, space>::iterator si;
	string v;
	rule r;

	for (ii = instrs.begin(); ii != instrs.end(); ii++)
	{
		for (ri = (*ii)->rules.begin(); ri != (*ii)->rules.end(); ri++)
		{
			for (rj = prs.begin(); rj != prs.end() && rj->right.var != ri->right.var; rj++);

			v = ri->right.var.substr(0, ri->right.var.length()-1);
			si = states.find(v);
			if (rj == prs.end() && si != states.end())
			{
				r.clear(0);
				r.left = expression(ri->left.var, states, tab+"\t", verbosity);
				if (ri->right.var.substr(ri->right.var.length()-1) == "+")
					r.right = up((space&)si->second);
				else
					r.right = down((space&)si->second);

				// TODO I'm not sure if checking against the strict count right now is correct.
				if (strict_count(r.right) > 0)
					prs.push_back(r);
			}
			else if (rj != prs.end())
				rj->left = rj->left | expression(ri->left.var, states, tab+"\t", verbosity);
			else
			{
				r.clear(0);
				r.left = expression(ri->left.var, states, tab+"\t", verbosity);
				for (size_t i = 0; i < states.begin()->second.states.size(); i++)
					r.right.states.push_back(state("0", false));
				r.right.var = ri->right.var;

				prs.push_back(r);

				cout << "Ah Hell... " << v << endl;
			}
		}
	}

	return prs;
}*/

/* Search a backward in a conflict string starting at a necessary
 * firing for a clean state variable position.
 *
 * I think it is valid to place a state variable transition
 * right before an indistinguishable state, but not after. Right
 * now we make sure we have a distinguishable state both before and
 * after the state variable transition, but I don't think this is necessary.
 */
size_t search_back(string s, size_t offset)
{
	// Find the next necessary firing
	size_t st = s.find_first_of("!", offset+1);
	int mindots = 1, numdots = 0;

	if (st == s.npos)
		return s.npos;

	// Starting at the next necessary firing, loop backward until we hit a
	// conflict or the current necessary firing. We must see at least
	// mindots '.' to have a clean state variable position
	for (size_t ct = st-1; ct > offset && ct < s.length() && ct >= 0; ct--)
	{
		if (s[ct] == 'C')
		{
			// We didn't find enough clean states. We must
			// pass this and keep going
			if (numdots < mindots)
			{
				numdots = 0;
				mindots = 2;
				st = ct;
			}
			// We found a valid position. Loop back until
			// we get as close to the necessary firing as possible
			else
			{
				ct = (st+ct)/2 + 1;
				for (; s[ct+1] != 'C' && s[ct] != '!'; ct++);

				return ct;
			}
		}
		else if (s[ct] == '.')
			numdots++;
	}

	return s.npos;
}

/* Search a forward in a conflict string starting at a necessary
 * firing for a clean state variable position.
 *
 * !.C
 * !..C
 * !...C
 * !....C
 * ect
 *
 *
 * The following cannot happen. If a production rule fires, it must
 * remain firing until another production rule has fired.
 * x+;x- is invalid CHP that will produce !C
 * x+;y+;x- is valid CHP that can only produce !.C
 *
 * !C
 * !CC
 * !CCC
 *
 */
size_t search_front(string s, size_t offset)
{
	// Find the previous necessary firing
	size_t st = s.find_last_of("!", offset-1);
	int mindots = 1, numdots = 0;

	if (st == s.npos)
		return s.npos;

	// Starting at the previous necessary firing, loop until we hit a
	// conflict or the current necessary firing. We must see at least
	// mindots '.' to have a clean state variable position
	for (size_t ct = st; ct < offset && ct < s.length() && ct >= 0; ct++)
	{
		if (s[ct] == 'C')
		{
			// We didn't find enough clean states. We must
			// pass this and keep going
			if (numdots < mindots)
			{
				numdots = 0;
				mindots = 2;
				st = ct;
			}
			// We found a valid position. Loop back until
			// we get as close to the necessary firing as possible
			else
			{
				ct = (st+ct)/2 + 1;
				for (; s[ct-2] != 'C' && s[ct-1] != '!'; ct--);

				return ct;
			}
		}
		else if (s[ct] == '.')
			numdots++;
	}

	return s.npos;
}

/* There can be four states: 1, 0, X, _ where underscore is empty set and X is full set.
 * Each index in the returned list represents the instruction before which there should be
 * a state variable transition.
 *
 * Ex: 3, 5 means sv0 goes high at 3 and sv2 goes high at 5.
 * !.C
 * C.!
 * C..C!
 * !C..C
*/
/*list<size_t> state_variable_positions(space left, space right, string tab, int verbosity)
{
	list<size_t> state_locations;
	list<size_t>::iterator si;
	size_t i;

	// Generate the conflict string
	string conflict = conflicts(left, right);

	// If no conflicts, we are done.
	if (conflict.find_first_of("C") == conflict.npos)
		return state_locations;

	if (verbosity >= VERB_STATEVAR)
		cout << tab << "Conflict:\t" << conflict << "\t\t" << left << " -> " << right << endl;

	// Loop through all necessary firings
	size_t foundb = conflict.npos, foundf = conflict.npos;
	size_t offset = 0;
	while (offset != conflict.npos)
	{
		// Search backward from the necessary firing
		// for a clean state variable position
		foundb = search_back(conflict, offset);

		if (foundb != conflict.npos && foundf != foundb)
			state_locations.push_back(foundb);

		// Search forward from the necessary firing
		// for a clean state variable position
		offset = conflict.find_first_of("!", offset+1);

		foundf = search_front(conflict, offset);

		if (foundf != conflict.npos && foundf != foundb)
			state_locations.push_back(foundf);
	}

	state_locations.sort();
	state_locations.unique();

	if (verbosity >= VERB_STATEVAR)
	{
		cout << tab << "Resolved:\t";
		for (i = 0, si = state_locations.begin(); i < conflict.length(); i++)
		{
			if (*si == i)
			{
				cout << "S";
				si++;
			}
			cout << conflict[i];
		}
		cout << "\t\t" << left << " -> " << right << endl;
	}

	return state_locations;
}*/

/*bool production_rule_check(string *raw, block *b, string tab, int verbosity)
{
	// At this point, block has a set of candidate rules. Check to see if those rules fire at the
	// appropriate times. If not, add state variables and reparse.

	list<rule>::iterator ri;
	list<size_t> state_locations; 	//See state_variable's return for details of how this is being used.
	list<size_t>::iterator li;
	list<size_t> temp;
	variable *v;
	int depth[3] = {0, 0, 0};

	//Loop through all produced rules run state_locations to get a list of indexes to insert state vars.
	for (ri = b->rules.begin(); ri != b->rules.end(); ri++)
	{
		temp = state_variable_positions(ri->left, ri->right, tab, verbosity);
		state_locations.merge(temp);
	}

	//If there are multiple production rules asking for a state variable to be inserted at a particular
	//index, we only need to insert a single state variable. Sort and unique provides a clean list.
	state_locations.sort();
	state_locations.unique();

	int highest_state_name = 0;				// Used for finding a unique name for the state variable
	int how_many_added = 0;					// How many instructions have we added?
	int how_many_inserted = 0;				// How many instructions have been inserted?
	list<pair<string,int> > to_insertl;

	//Loop through our list of desired state variable insertion indices.
	for (li = state_locations.begin(); li != state_locations.end(); li++)
	{
		//Find the lowest variable name not in globals (no name conflicts)
		highest_state_name = 0;
		while (b->global.find(b->uid + "_" + to_string(highest_state_name)) != b->global.end())
			highest_state_name++;

		//Create a new variable with the unique name.
		v = new variable("int<1>" + b->uid + "_" + to_string(highest_state_name), tab, verbosity);
		//Add to globals
		b->global.insert(pair<string, variable*>(v->name, v));
		//Add to locals
		b->local.insert(pair<string, variable*>(v->name, v));

		//Insert the state variable declaration and state variable transition
		to_insertl.push_back(pair<string,int>(v->name+":=1",*li));
		*raw = "int<1>" + v->name + ":=0;" + v->name + ":=0;" + *raw;
		how_many_added+=2;
	}

	list<pair<string,int> >::iterator instr_adderl;
	//Add leading and tailing semicolon to assist instruction counting
	*raw = ";"  + *raw + ";";
	//For every instruction we are to insert into the instruction stream...
	for(instr_adderl = to_insertl.begin(); instr_adderl != to_insertl.end();instr_adderl++)
	{
		unsigned int insertion_location = 0;
		//Loop through to find the semicolon at which we want to insert the current state variable
		//Note that the offset of how_many_added/inserted is required as the instruction size of
		//the instruction stream grows as instructions are added.
		// Clean this up, there was an error where we were simply looking for semicolons in the string instead of paying attention to depth
		depth[0] = 0;
		depth[1] = 0;
		depth[2] = 0;
		for (int counter = 0; counter <= (instr_adderl->second+how_many_inserted+how_many_added) && insertion_location < raw->length(); insertion_location++)
		{
			if (raw->at(insertion_location) == '(')
				depth[0]++;
			else if (raw->at(insertion_location) == '[')
				depth[1]++;
			else if (raw->at(insertion_location) == '{')
				depth[2]++;
			else if (raw->at(insertion_location) == ')')
				depth[0]--;
			else if (raw->at(insertion_location) == ']')
				depth[1]--;
			else if (raw->at(insertion_location) == '}')
				depth[2]--;

			if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && raw->at(insertion_location) == ';')
				counter++;
		}

		if (insertion_location < 0)
			cout << "Error: State variable insertion failed. " << endl;
		else
		{
			*raw = raw->substr(0, insertion_location) + instr_adderl->first + ";" + raw->substr(insertion_location);
			how_many_inserted++;
		}
	}

	//If we added a state variable...
	if (how_many_added > 0)
	{
		*raw = raw->substr(1, raw->length() - 2);
		return false;
	}
	else
		return true;
}*/
