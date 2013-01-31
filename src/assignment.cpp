#include "assignment.h"

assignment::assignment()
{
	_kind = "assignment";
}

assignment::assignment(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity)
{
	this->_kind		= "assignment";
	this->chp		= chp;
	this->tab		= tab;
	this->verbosity = verbosity;
	this->global	= globals;
	this->label		= label;

	expand_shortcuts();
	parse(types);
}

assignment::~assignment()
{
	_kind = "assignment";
}

void assignment::expand_shortcuts()
{
	// Convert var+ to var:=1
	if(chp.find(":=") == chp.npos && chp.find("+") != chp.npos)
		chp = chp.substr(0, chp.find("+")) + ":=1";

	// Convert var- to var:=0
	if(chp.find(":=") == chp.npos && chp.find("-") != chp.npos)
		chp = chp.substr(0, chp.find("-")) + ":=0";
}

void assignment::parse(map<string, keyword*> types)
{
	size_t middle;
	string left_raw, right_raw;
	size_t i, j;
	size_t k, l;

	if (verbosity >= VERB_PARSE)
		cout << tab << "Assignment:\t" + chp << endl;

	// TODO we need to involve the defined operators here as well, not just solve for the state space.

	// Identify that this instruction is an assign.
	if (chp.find(":=") != chp.npos)
	{
		// Separate the two operands (the variable to be assigned and the value to assign)
		middle = chp.find(":=");
		left_raw = chp.substr(0, middle);
		right_raw = chp.substr(middle+2);
		for (i = left_raw.find_first_of(","), j = left_raw.find_first_of(","), k = 0, l = 0; i != left_raw.npos && j != right_raw.npos; i = left_raw.find_first_of(",", i+1), j = right_raw.find_first_of(",", j+1))
		{
			expr.insert(pair<string, string>(left_raw.substr(k, i-k), right_raw.substr(l, j-l)));
			k = i;
			l = j;
		}
		expr.insert(pair<string, string>(left_raw.substr(k), right_raw.substr(l)));
	}
	// If all else fails, complain to the user.
	else
		cout << "Error: Instruction not handled: " << chp << endl;
}

int assignment::generate_states(state_space *space, graph *trans, int init)
{
	cout << tab << "Assignment " << chp << endl;

	uid = space->size();

	map<string, variable>::iterator vi;
	map<string, string>::iterator ei;
	int i;

	state s;

	for (vi = global->begin(); vi != global->end(); vi++)
		s.assign(vi->second.uid, value("X"));

	if (init != -1)
		for (i = 0; i < (*space)[init].size(); i++)
			s.assign(i, (*space)[init][i]);

	for (ei = expr.begin(); ei != expr.end(); ei++)
	{
		vi = global->find(ei->first);

		if (vi != global->end())
			s.assign(vi->second.uid, evaluate(ei->second, global, s.values, tab, verbosity));
		else
			cout << "Error: Undefined variable " << vi->first << "." << endl;
	}

	cout << tab << s << endl;

	space->push_back(s);
	if (init != -1)
		trans->insert_edge(init, uid, chp);


	return uid;
}

void assignment::generate_prs()
{


	print_prs();
}
