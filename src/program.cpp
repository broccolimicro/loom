#include "program.h"
#include "utility.h"

program::program()
{
}

program::program(string chp, int verbosity)
{
	this->verbosity = verbosity;

	// CHP to HSE
	parse(chp, verbosity);
	merge();
	//project();
	//decompose();
	//reshuffle();

	// HSE to State Space
	generate_states();
	insert_state_vars();

	// State Space to PRS
	generate_prs();
	//factor_prs();


	print_hse();
	print_dot();
	print_petrify();
	print_prs();
}

program::~program()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	type_space.clear();
}

program &program::operator=(program p)
{
	type_space = p.type_space;
	return *this;
}

void program::parse(string chp, int verbosity)
{
	//TODO: THIS BREAKS IF THERE ARE NO IMPLICANTS FOR A OUTPUT
	string::iterator i, j;
	string cleaned_chp = "";
	string word;
	string error;
	int error_start, error_len;

	process *p;
	operate *o;
	record *r;
	channel *c;

	// Define the basic types. In this case, 'wire'
	type_space.clear();
	type_space.insert(pair<string, keyword*>("node", new keyword("node")));
	c = new channel("channel __sync{node<1>r;node<1>a;operator?(){[r];a+;[~r];a-}operator!(){r+;[a];r-;[~a]}operator#(){r}}", &type_space, verbosity);
	type_space.insert(pair<string, channel*>(c->name, c));
	type_space.insert(pair<string, operate*>(c->name + "." + c->send->name, c->send));
	type_space.insert(pair<string, operate*>(c->name + "." + c->recv->name, c->recv));
	type_space.insert(pair<string, operate*>(c->name + "." + c->probe->name, c->probe));

	chp			= remove_comments(chp);
	cleaned_chp = remove_whitespace(chp);

	if (verbosity & VERB_PRECOMPILED_CHP)
		cout << chp << endl;

	// Split the program into records and processes
	int depth[3] = {0};
	for (i = cleaned_chp.begin(), j = cleaned_chp.begin(); i != cleaned_chp.end(); i++)
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

		// Are we at the end of a record or process definition?
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && *i == '}')
		{
			// Make sure this isn't vacuous
			if (i-j+1 > 0)
			{
				// Is this a process?
				if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "process ") == 0 ||
					cleaned_chp.compare(j-cleaned_chp.begin(), 15, "inline process ") == 0)
				{
					p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &type_space, verbosity);
					type_space.insert(pair<string, process*>(p->name, p));
				}
				// Is this an operator?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "operator") == 0)
				{
					o = new operate(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &type_space, verbosity);
					type_space.insert(pair<string, operate*>(o->name, o));
				}
				// This isn't a process, is it a record?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "record ") == 0)
				{
					r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &type_space, verbosity);
					type_space.insert(pair<string, record*>(r->name, r));
				}
				// Is it a channel definition?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "channel ") == 0)
				{
					c = new channel(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &type_space, verbosity);
					type_space.insert(pair<string, channel*>(c->name, c));
					type_space.insert(pair<string, operate*>(c->name + "." + c->send->name, c->send));
					type_space.insert(pair<string, operate*>(c->name + "." + c->recv->name, c->recv));
					type_space.insert(pair<string, operate*>(c->name + "." + c->probe->name, c->probe));
				}
				// This isn't either a process or a record, this is an error.
				else
				{
					error = "Error: CHP block outside of process.\nIgnoring block:\t";
					error_start = j-cleaned_chp.begin();
					error_len = min(min(cleaned_chp.find("process ", error_start), cleaned_chp.find("record ", error_start)), cleaned_chp.find("channel ", error_start)) - error_start;
					error += cleaned_chp.substr(error_start, error_len);
					cout << error << endl;
					j += error_len;

					// Make sure we don't miss the next record or process though.
					if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "process ") == 0 ||
						cleaned_chp.compare(j-cleaned_chp.begin(), 15, "inline process ") == 0)
					{
						p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &type_space, verbosity);
						type_space.insert(pair<string, process*>(p->name, p));
					}
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "operator") == 0)
					{
						o = new operate(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &type_space, verbosity);
						type_space.insert(pair<string, operate*>(o->name, o));
					}
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "record ") == 0)
					{
						r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &type_space, verbosity);
						type_space.insert(pair<string, record*>(r->name, r));
					}
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "channel ") == 0)
					{
						c = new channel(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &type_space, verbosity);
						type_space.insert(pair<string, channel*>(c->name, c));
						type_space.insert(pair<string, operate*>(c->name + "." + c->send->name, c->send));
						type_space.insert(pair<string, operate*>(c->name + "." + c->recv->name, c->recv));
						type_space.insert(pair<string, operate*>(c->name + "." + c->probe->name, c->probe));
					}
				}
			}
			j = i+1;
		}
	}
}

void program::merge()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != string::npos))
			((process*)i->second)->merge();
}

// TODO Projection algorithm - when do we need to do projection? when shouldn't we do projection?
void program::project()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != string::npos))
			((process*)i->second)->project();
}

// TODO Process decomposition - How big should we make processes?
void program::decompose()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != string::npos))
			((process*)i->second)->decompose();
}

// TODO Handshaking Reshuffling
void program::reshuffle()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != string::npos))
			((process*)i->second)->reshuffle();
}

// TODO There is a problem with the interaction of scribe variables with bubbleless reshuffling because scribe variables insert bubbles
void program::generate_states()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != string::npos))
			((process*)i->second)->generate_states();
}

void program::insert_state_vars()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != string::npos))
			((process*)i->second)->insert_state_vars();
}

void program::generate_prs()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != string::npos))
			((process*)i->second)->generate_prs();
}

/* TODO: Factoring - production rules should be relatively short.
 * Look for common expressions between production rules and factor them
 * out into their own variable.
 */
void program::factor_prs()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process")
			((process*)i->second)->factor_prs();
}

void program::print_hse()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process")
		{
			ofstream fout(i->first + ".hse");
			((process*)i->second)->print_hse(&fout);
			fout.close();
		}
}

void program::print_dot()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process")
		{
			ofstream fout(i->first + ".dot");
			((process*)i->second)->print_dot(&fout);
			fout.close();
		}
}

void program::print_petrify()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process")
			((process*)i->second)->print_petrify();
}

void program::print_prs()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		if (i->second->kind() == "process")
		{
			ofstream fout(i->first + ".prs");
			((process*)i->second)->print_prs(&fout);
			fout.close();
		}
}
