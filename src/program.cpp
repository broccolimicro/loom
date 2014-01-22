#include "program.h"
#include "utility.h"

program::program()
{
}

program::~program()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	types.clear();
}

program &program::operator=(program p)
{
	types = p.types;
	return *this;
}

void program::compile()
{
	sstring prgm;
	int i;
	int open, close;

	//While there are and #includes (of the form #include "foo.chp") in the program
	if (flags.input_files.size() > 0)
	{
		if (flags.pre())
		{
			prgm = sstring((istreambuf_iterator<char>(*(flags.input_files[0].second))),
						   istreambuf_iterator<char>());
			(*flags.log_file) << prgm << endl;
			while ((i = prgm.find("#include")) != prgm.npos)
			{
				//Find the file name
				open = prgm.find_first_of("\"", i+1);
				close = prgm.find_first_of("\"", open+1);

				if (flags.log_precompile())
					(*flags.log_file) << "Expanding Inclusion: " << prgm.substr(open+1, close-open-1) << endl;

				cout << flags.input_files[0].first << endl;
				ifstream s(flags.input_files[0].first + "/" + prgm.substr(open+1, close-open-1).c_str());
				sstring f((istreambuf_iterator<char>(s)),
						 istreambuf_iterator<char>());
				if (!s.is_open())
					cerr << "Error: File not found " << flags.input_files[0].first + "/" + prgm.substr(open+1, close-open-1) << endl;

				//Place the contents of the file where the #include statement was
				prgm = prgm.substr(0, i) + f + prgm.substr(close+1);

			}

			(*flags.log_file) << endl;
		}

		// CHP to HSE
		if (flags.hse())
		{
			parse(prgm);
			//simulate();
			rewrite();
			//project();
			//decompose();
			//reshuffle();
		}

		// HSE to State Space
		if (flags.sse())
		{
			generate_states();
			trim_states();
		}
		else if (flags.hse())
			print_hse();

		if (flags.svib())
			insert_state_vars();
		else if (flags.svi())
			insert_bubbleless_state_vars();

		// State Space to PRS
		if (flags.prsb())
		{
			generate_prs();
			//bubble_reshuffle();
			//factor_prs();

			print_hse();
			print_dot();
			print_prs();
		}
		else if (flags.prs())
		{
			generate_bubbleless_prs();

			print_hse();
			print_dot();
			print_prs();
		}
		else if (flags.hse() && flags.sse())
		{
			print_hse();
			print_dot();
		}
	}
}

void program::parse(sstring chp)
{
	//TODO: THIS BREAKS IF THERE ARE NO IMPLICANTS FOR A OUTPUT
	sstring::iterator i, j;
	sstring cleaned_chp = "";
	sstring word;
	int error_start, error_len;

	process *p;
	operate *o;
	record *r;
	channel *c;

	// Define the basic types. In this case, 'wire'
	types.clear();
	types.insert(pair<sstring, keyword*>("node", new keyword("node")));
	c = new channel("channel __sync{node<1>r:=0;node<1>a:=0;operator?(){[r];a+;[~r];a-}operator!(){r+;[a];r-;[~a]}operator#(){r}}", &types, &flags);
	types.insert(pair<sstring, channel*>(c->name, c));
	types.insert(pair<sstring, operate*>(c->name + "." + c->send->name, c->send));
	types.insert(pair<sstring, operate*>(c->name + "." + c->recv->name, c->recv));
	types.insert(pair<sstring, operate*>(c->name + "." + c->probe->name, c->probe));

	chp			= remove_comments(chp);
	cleaned_chp = remove_whitespace(chp);

	if (flags.log_precompile())
		(*flags.log_file) << chp << endl;

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
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && *i == '}' && (i+1 == cleaned_chp.end() || *(i+1) != '{'))
		{
			// Make sure this isn't vacuous
			if (i-j+1 > 0)
			{
				// Is this a process?
				if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "process ") == 0 ||
					cleaned_chp.compare(j-cleaned_chp.begin(), 15, "inline process ") == 0)
					p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &types, &flags);
				// Is this an operator?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "operator") == 0)
					o = new operate(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &types, &flags);
				// This isn't a process, is it a record?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "record ") == 0)
					r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &types, &flags);
				// Is it a channel definition?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "channel ") == 0)
					c = new channel(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &types, &flags);
				// This isn't either a process or a record, this is an error.
				else
				{
					error_start = j-cleaned_chp.begin();
					error_len = min(min(cleaned_chp.find("process ", error_start), cleaned_chp.find("record ", error_start)), cleaned_chp.find("channel ", error_start)) - error_start;
					cerr << "Error: CHP block outside of process." << endl << "Ignoring block:\t" << cleaned_chp.substr(error_start, error_len) << endl;
					j += error_len;

					// Make sure we don't miss the next record or process though.
					if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "process ") == 0 ||
						cleaned_chp.compare(j-cleaned_chp.begin(), 15, "inline process ") == 0)
						p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &types, &flags);
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "operator") == 0)
						o = new operate(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &types, &flags);
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "record ") == 0)
						r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &types, &flags);
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "channel ") == 0)
						c = new channel(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), &types, &flags);
				}
			}
			j = i+1;
		}
	}
}

void program::simulate()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || i->second->kind() == "operate")
			((process*)i->second)->simulate();
}

void program::rewrite()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || i->second->kind() == "operate")
			((process*)i->second)->rewrite();
}

// TODO Projection algorithm - when do we need to do projection? when shouldn't we do projection?
void program::project()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos))
			((process*)i->second)->project();
}

// TODO Process decomposition - How big should we make processes?
void program::decompose()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos))
			((process*)i->second)->decompose();
}

// TODO Handshaking Reshuffling
void program::reshuffle()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos))
			((process*)i->second)->reshuffle();
}

void program::generate_states()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos)
			((operate*)i->second)->generate_states();

	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process")
			((process*)i->second)->generate_states();
}

void program::trim_states()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos)
		{
			cout << "Trimming: " << i->first << endl;
			((operate*)i->second)->trim_states();
		}

	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process")
		{
			cout << "Trimming: " << i->first << endl;
			((process*)i->second)->trim_states();
		}
}

void program::insert_state_vars()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos))
			for (int j = 0; j < 100 && ((process*)i->second)->insert_state_vars(); j++)
			{
				if (i->second->kind() == "process")
					((process*)i->second)->update();
				else if (i->second->kind() == "operate")
					((operate*)i->second)->update();
			}
}

void program::insert_bubbleless_state_vars()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos))
			for (int j = 0; j < 100 && ((process*)i->second)->insert_bubbleless_state_vars(); j++)
			{
				if (i->second->kind() == "process")
					((process*)i->second)->update();
				else if (i->second->kind() == "operate")
					((operate*)i->second)->update();
			}
}

void program::generate_prs()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos))
			((process*)i->second)->generate_prs();
}

void program::generate_bubbleless_prs()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos))
			((process*)i->second)->generate_bubbleless_prs();
}

void program::bubble_reshuffle()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process" || (i->second->kind() == "operate" && i->first.find_first_of("!?") != sstring::npos))
			((process*)i->second)->bubble_reshuffle();
}

/* TODO: Factoring - production rules should be relatively short.
 * Look for common expressions between production rules and factor them
 * out into their own variable.
 */
void program::factor_prs()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process")
			((process*)i->second)->factor_prs();
}

void program::print_hse()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process")
			((process*)i->second)->print_hse(flags.hse_file);
}

void program::print_dot()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process")
			((process*)i->second)->print_dot(flags.state_file);
}

void program::print_prs()
{
	type_space::iterator i;
	for (i = types.begin(); i != types.end(); i++)
		if (i->second->kind() == "process")
			((process*)i->second)->print_prs(flags.output_file);
}
