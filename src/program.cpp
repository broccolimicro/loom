#include "program.h"

program::program()
{
	type_space.insert(pair<string, keyword*>("int", new keyword("int")));
}

program::program(string chp, int verbosity)
{
	parse(chp, verbosity);
}

program::~program()
{
	map<string, keyword*>::iterator i;
	for (i = type_space.begin(); i != type_space.end(); i++)
		delete i->second;

	type_space.clear();
}

program &program::operator=(program p)
{
	type_space = p.type_space;
	prs = p.prs;
	errors = p.errors;
	return *this;
}


void program::parse(string chp, int verbosity)
{
	string::iterator i, j;
	string cleaned_chp = "";
	string word;
	string error;
	int error_start, error_len;

	process *p;
	operate *o;
	record *r;
	channel *c;

	// Define the basic types. In this case, 'int'
	type_space.insert(pair<string, keyword*>("int", new keyword("int")));

	//Remove line comments:
	size_t comment_begin = chp.find("//");
	size_t comment_end = chp.find("\n", comment_begin);
	while (comment_begin != chp.npos && comment_end != chp.npos){
		chp = chp.substr(0,comment_begin) + chp.substr(comment_end);
		comment_begin = chp.find("//");
		comment_end = chp.find("\n", comment_begin);
	}

	//Remove block comments:
	comment_begin = chp.find("/*");
	comment_end = chp.find("*/");
	while (comment_begin != chp.npos && comment_end != chp.npos){
		chp = chp.substr(0,comment_begin) + chp.substr(comment_end+2);
		comment_begin = chp.find("/*");
		comment_end = chp.find("*/");
	}

	// remove extraneous whitespace
	for (i = chp.begin(); i != chp.end(); i++)
	{
		if (!sc(*i))
			cleaned_chp += *i;
		else if (nc(*(i-1)) && (i == chp.end()-1 || nc(*(i+1))))
			cleaned_chp += ' ';
	}

	// split the program into records and processes
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
				if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "process ") == 0)
				{
					p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, map<string, variable>(), verbosity);
					type_space.insert(pair<string, process*>(p->name, p));
				}
				// Is this an operator?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "operator") == 0)
				{
					o = new operate(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, map<string, variable>(), verbosity);
					type_space.insert(pair<string, operate*>(o->name, o));
				}
				// This isn't a process, is it a record?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "record ") == 0)
				{
					r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, "", verbosity);
					type_space.insert(pair<string, record*>(r->name, r));
				}
				// Is it a channel definition?
				else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "channel ") == 0)
				{
					c = new channel(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, "", verbosity);
					type_space.insert(pair<string, channel*>(c->name, c));
					type_space.insert(pair<string, operate*>(c->send.name, &c->send));
					type_space.insert(pair<string, operate*>(c->recv.name, &c->recv));
					type_space.insert(pair<string, operate*>(c->probe.name, &c->probe));
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
					if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "process ") == 0)
					{
						p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, map<string, variable>(), verbosity);
						type_space.insert(pair<string, process*>(p->name, p));
					}
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "operator") == 0)
					{
						o = new operate(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, map<string, variable>(), verbosity);
						type_space.insert(pair<string, operate*>(o->name, o));
					}
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "record ") == 0)
					{
						r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, "", verbosity);
						type_space.insert(pair<string, record*>(r->name, r));
					}
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "channel ") == 0)
					{
						c = new channel(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, "", verbosity);
						type_space.insert(pair<string, channel*>(c->name, c));
						type_space.insert(pair<string, operate*>(c->send.name, &c->send));
						type_space.insert(pair<string, operate*>(c->recv.name, &c->recv));
						type_space.insert(pair<string, operate*>(c->probe.name, &c->probe));
					}
				}
			}
			j = i+1;
		}
	}

	map<string, variable> global, label;
	prgm = (parallel*)expand_instantiation("main _()", type_space, &global, &label, NULL, "", verbosity, true);

	cout << "Generating State Space" << endl;
	space.states.push_back(state(value("X"), global.size()));
	prgm->generate_states(&space, &trans, 0);
	if(STATESP_CO)
	{
		cout << "\t " << global << endl;
		print_space_to_console();
	}
	if(STATESP_GR)
	{
		cout << "\t " << global << endl;
		print_space_graph_to_console();
	}
	//Generate+print diff_space
	state_space diff_space = delta_space_gen(space, trans);
	print_diff_space_to_console(diff_space);
	prgm->print_hse();
}


void print_line(int from, graph *trans)
{
	int i;
	if(from >= (int)trans->edges.size())
		trans->edges.resize(from+1, vector<int>());

	cout << from << ": ";
	for (i = 0; i < (int)trans->edges[from].size(); i++)
		cout << (trans->edges[from])[i] << " ";
	cout << endl;
}

void print_line_dot(int from, state_space *spaces, graph *trans) // Print a line following .dot graphvis formatting
{
	int i;
	if(from >= (int)trans->edges.size())
		trans->edges.resize(from+1, vector<int>());

	//Each edge should be: 	"Node 1" -> "Node 2" [ label = "trans" ];
	for (i = 0; i < (int)trans->edges[from].size(); i++)
	{
		//	"Node 1" -> "Node 2" [ label = "trans" ];
		//"Node 1" ->
		cout << "\t\"" << from << ":" << (*spaces)[from] << "\"" << " -> ";
		//"Node 2
		cout << "\"" << trans->edges[from][i] << ":" << (*spaces)[trans->edges[from][i]];
		//" [ label = "trans" ];
		cout << "\" [ label = \"" << (trans->transitions[from])[i] << "\" ];" << endl;
	}

}
void print_line_with_trans(int from, graph *trans)
{
	int i, j;
	if(from >= (int)trans->edges.size())
		trans->edges.resize(from+1, vector<int>());

	cout << from << ":";
	for (i = 0; i < 4 - (int)log10((double)max(from, 1)); i++)
		cout << " ";
	j = 0;
	for (i = 0; i < (int)trans->edges[from].size(); i++)
	{
		cout << trans->edges[from][i] << " ";
		j += (int)log10((double)max(trans->edges[from][i], 1)) + 2;
	}
	for (i = 0; i < 10 - j; i++)
		cout << " ";
	for (i = 0; from < (int)trans->transitions.size() && i < (int)trans->transitions[from].size(); i++)
		cout << (trans->transitions[from])[i] << " ";

	cout << endl;
}
void program::print_space_to_console()
{
	//Print space (for debugging purposes)
	cout << endl << endl << "\tState space:" << endl;
	for(int i = 0; i < space.size(); i++)
	{
		cout << "\t "<< space[i] << "  ";
		print_line_with_trans(i, &trans);
	}
	cout << endl << endl;
	//cout << "Current connections: " << endl;
	//cout << (*trans);
}
void print_diff_space_to_console(state_space diff_space)
{
	//Print space (for debugging purposes)
	cout << endl << endl << "\tDiff state space:" << endl;
	for(int i = 0; i < diff_space.size(); i++)
	{
		cout << "\t "<< diff_space[i] << "  ";
		cout << diff_space[i].tag << endl;
	}
	cout << endl << endl;
	//cout << "Current connections: " << endl;
	//cout << (*trans);
}

void program::print_space_graph_to_console()
{
	//Print space (for debugging purposes)
	cout << endl << endl << "\t.dot formatted graph:" << endl;
	cout << "digraph finite_state_machine {"<< endl << "\tgraph [ label = \"\\n\\nState space graph!\" ];" << endl;
	if (!GRAPH_VERT)
		cout <<"\trankdir=LR;"<<endl;
	cout <<"\tnode [shape = ellipse];"<<endl;
	cout <<"\tgraph [ dpi =" << GRAPH_DPI << " ];" << endl;
	for(int i = 0; i < space.size(); i++)
	{
		//cout << "\t "<< space[i] << "  ";
		print_line_dot(i, &space, &trans);
	}
	cout << "}" << endl << endl;

}

state_space delta_space_gen(state_space spaces, graph trans)
{
	state_space delta_space;
	state leaving_state, incoming_state, result_state;


	for(int i = 0; i < spaces.size(); i++)
	{
		if(i >= (int)trans.edges.size())
		{
			trans.edges.resize(i+1, vector<int>());
			cout << "Does this ever occur???" << endl;
		}

		for (int j = 0; j < (int)trans.edges[i].size(); j++)
		{
			//Node 1
			leaving_state = spaces[i];
			//Node 2
			incoming_state = spaces[trans.edges[i][j]];
			cout << i << " " << trans.edges[i][j] << endl;
			result_state = diff(leaving_state,incoming_state);
			result_state.tag = i;
			delta_space.states.push_back(result_state);
		}
	}
	//TODO! return a graph, too, so that the states mean something.
	return delta_space;

}
