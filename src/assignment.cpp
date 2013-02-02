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

assignment &assignment::operator=(assignment a)
{
	this->uid		= a.uid;
	this->expr		= a.expr;
	this->chp		= a.chp;
	this->rules		= a.rules;
	this->global	= a.global;
	this->label		= a.label;
	this->tab		= a.tab;
	this->verbosity	= a.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 * TODO Check to make sure that this actually works as specified
 */
instruction *assignment::duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert)
{
	assignment *instr;

	instr 				= new assignment();
	instr->chp			= this->chp;
	instr->global		= globals;
	instr->label		= labels;
	instr->tab			= this->tab;
	instr->verbosity	= this->verbosity;
	instr->expr			= this->expr;

	map<string, string>::iterator i;
	list<pair<string, string> >::iterator j;
	size_t k;
	for (i = convert.begin(); i != convert.end(); i++)
	{
		while ((k = instr->chp.find(i->first)) != instr->chp.npos)
			instr->chp.replace(k, i->first.length(), i->second);
		for (j = instr->expr.begin(); j != instr->expr.end(); j++)
		{
			while ((k = j->first.find(i->first)) != j->first.npos)
				j->first.replace(k, i->first.length(), i->second);
			while ((k = j->second.find(i->first)) != j->second.npos)
				j->second.replace(k, i->first.length(), i->second);
		}
	}

	return instr;
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
			expr.push_back(pair<string, string>(left_raw.substr(k, i-k), right_raw.substr(l, j-l)));
			k = i;
			l = j;
		}
		expr.push_back(pair<string, string>(left_raw.substr(k), right_raw.substr(l)));
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
	list<pair<string, string> >::iterator ei;
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
			cout << "Error: Undefined variable " << ei->first << "." << endl;
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
