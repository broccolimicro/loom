#include "program.h"
#include "utility.h"

program::program()
{
	type_space.insert(pair<string, keyword*>("int", new keyword("int")));
	vars.types = &type_space;
}

program::program(string chp, int verbosity)
{
	vars.types = &type_space;
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
	prs_up = p.prs_up;
	prs_down = p.prs_down;
	errors = p.errors;
	return *this;
}


void program::parse(string chp, int verbosity)
{
	//TODO: Lost information in statespace from guard (example a xor b) not incorperated. Copy into PRS?
	//TODO: THIS BREAKS IF THERE ARE NO IMPLICANTS FOR A OUTPUT
	//TODO: Logic minimization
	//TODO: Figure out indistinguishable states
	//TODO: Add state variables
	//TODO: Explore State Variable Factorization
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
					type_space.insert(pair<string, operate*>(c->name + "." + c->send.name, &c->send));
					type_space.insert(pair<string, operate*>(c->name + "." + c->recv.name, &c->recv));
					type_space.insert(pair<string, operate*>(c->name + "." + c->probe.name, &c->probe));
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
						type_space.insert(pair<string, operate*>(c->name + "." + c->send.name, &c->send));
						type_space.insert(pair<string, operate*>(c->name + "." + c->recv.name, &c->recv));
						type_space.insert(pair<string, operate*>(c->name + "." + c->probe.name, &c->probe));
					}
				}
			}
			j = i+1;
		}
	}

	vars.insert(variable("Reset", "int", value("0"), 1, false));
	vars.insert(variable("_Reset", "int", value("1"), 1, false));

	prgm = (parallel*)expand_instantiation("main _()", &vars, NULL, "", verbosity, true);

	//At this point in the program, 'parsing' is done. Launching State Space Gen

	state sr, s;
	for (map<string, variable>::iterator ri = vars.global.begin(); ri != vars.global.end(); ri++)
	{
		if (ri->second.name == "Reset")
			sr.assign(ri->second.uid, value("1"));
		else if (ri->second.name == "_Reset")
			sr.assign(ri->second.uid, value("0"));
		else
			sr.assign(ri->second.uid, value("X"));

		s.assign(ri->second.uid, ri->second.reset);
	}
	space.states.push_back(sr);
	space.states.push_back(s);
	trans.insert_edge(0, 1, "Reset");

	cout << "Generating State Space" << endl;
	prgm->generate_states(&space, &trans, 1);

	//Generate states is done. Launching post-state info gathering

	//The whole program has states now!

	if(STATESP_CO)
	{
		print_space_to_console();
	}
	if(STATESP_GR)
	{
		print_space_graph_to_console();
	}
	//Generate+print diff_space
	state_space diff_space = delta_space_gen(space, trans);
	print_diff_space_to_console(diff_space);

	cout << vars << endl;

	prgm->print_hse();
	cout << endl;
	//Create an up and down PRS for each variable  (UID indexed)
	prs_up.resize(vars.global.size());
	prs_down.resize(vars.global.size());
	//Inserting names into each PRS
	for(int i = 0; i < (int)prs_up.size(); i++)
	{
		prs_up[i].right = vars.get_name(i)+"+";
		prs_down[i].right = vars.get_name(i)+"-";
	}


	//Find the implicants of the diff space
	//TODO: Move this out into a function that returns a pair<vector<rule>, vector<rule> >
	for(int i = 0; i < diff_space.size();i++)
	{
		for(int j = 0; j< diff_space[i].size(); j++)
		{
			if((diff_space[i])[j].data == "1")
			{
				if(diff_space[i].tag!=-1)	//Output variable needs to fire high
					prs_up[j].implicants.push_back(space[diff_space[i].tag]);
			}
			if((diff_space[i])[j].data == "0")
			{
				if(diff_space[i].tag!=-1)	//Output variable needs to fire low
					prs_down[j].implicants.push_back(space[diff_space[i].tag]);
			}

		}
	}

	merge_implicants();
	print_prs();

	cout << "trying to minimize..." << endl << endl;

	//Bogus test implicant
	state temp;
	temp = prs_up[2].implicants[0];
	temp.values[0].data = "1";
	cout << "Original: " << prs_up[2].implicants[0] << endl;
	cout << "Adding: " << temp << endl;
	prs_up[2].implicants.push_back(temp);

	merge_implicants();
	print_prs();

	prs_up = minimize_rule_vector(prs_up);
	prs_down = minimize_rule_vector(prs_down);

	merge_implicants();
	print_prs();

	cout << "Done!" << endl<< endl << endl;
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
//Merges all the implicants and puts them into the .left fields
void program::merge_implicants()
{
	//Remove whatever might have been in there before
	for(int i = 0; i < (int)prs_up.size(); i++)
	{
		prs_up[i].left.clear();
		prs_down[i].left.clear();
	}

	map<string, variable>::iterator globali = vars.global.begin();
	for (int i = 0; i< (int)prs_up.size(); i++, globali++)
	{
		//Print out the implicants
		//for(int j = 0; j < (int)prs_up[i].implicants.size(); j++)
		//	cout << i << "+ "<<   prs_up[i].implicants[j] << endl;
		//for(int j = 0; j < (int)prs_down[i].implicants.size(); j++)
		//	cout << i << "- "<< prs_down[i].implicants[j] << endl;

		for(int upi = 0; upi<prs_up[i].implicants.size(); upi++)
		{
			if(!is_all_x(prs_up[i].implicants[upi]))
			{
				prs_up[i].left += "(";
				bool first = true;
				for(int j = 0; j < prs_up[i].implicants[upi].size(); j++)
				{
					if ( (prs_up[i].implicants[upi][j].data != "X") && (vars.get_name(j) != prs_up[i].right.substr(0,prs_up[i].right.size()-1))     )
					{
						if(!first)
							prs_up[i].left += " & ";
						else
							first = false;

						//if(vars.get_name(j) != prs_up[i].right.substr(0,prs_up[i].right.size()-1))
						//{
						if((prs_up[i].implicants[upi])[j].data == "0")
							prs_up[i].left += "~";
						prs_up[i].left += vars.get_name(j);
						//}
					}
				}
				prs_up[i].left += ")";
				prs_up[i].left += " | ";
			}
		}
		if(prs_up[i].left.size() >= 3)
			prs_up[i].left = prs_up[i].left.substr(0, prs_up[i].left.size() - 3);

		for(int downi = 0; downi<prs_down[i].implicants.size(); downi++)
		{
			if(!is_all_x(prs_down[i].implicants[downi]))
			{
				prs_down[i].left += "(";
				bool first = true;
				for(int j = 0; j < prs_down[i].implicants[downi].size(); j++)
				{
					if ( ((prs_down[i].implicants[downi])[j].data != "X")  && (vars.get_name(j) != prs_down[i].right.substr(0,prs_down[i].right.size()-1)))
					{
						if (!first)
							prs_down[i].left += " & ";
						else
							first = false;

						//if(vars.get_name(j) != prs_down[i].right.substr(0,prs_down[i].right.size()-1)){
						if((prs_down[i].implicants[downi])[j].data == "0")
							prs_down[i].left += "~";
						prs_down[i].left += vars.get_name(j);
						//}

					}
				}
				prs_down[i].left += ")";
				prs_down[i].left += " | ";
			}
		}
		if(prs_down[i].left.size() >= 3)
			prs_down[i].left = prs_down[i].left.substr(0, prs_down[i].left.size() - 3);

	}
}

void program::print_prs()
{
	//Print out implicants
	map<string, variable>::iterator globali = vars.global.begin();

	cout << endl << endl << endl << "Production Rules: " << endl;

	for (int i = 0; i< (int)prs_up.size(); i++, globali++)
	{
		if (prs_up[i].left != "")
			cout << prs_up[i].left << " -> " << prs_up[i].right << endl;
		if (prs_down[i].left != "")
			cout << prs_down[i].left << " -> " << prs_down[i].right << endl;

	}

}

//Reduce all implicants to prime
rule reduce_to_prime(rule pr)
{
	rule result = pr;
	if (result.implicants.size() < 2)
		return result;
	//Reduce to prime guards
	//Totally is a more efficient/logical way to do this
	int i = 0;
	int j = 0;
	bool removed_junk;
	removed_junk = true;
	while(i != result.implicants.size()-1)
	{
		removed_junk = false;
		for (i = 0; (i < result.implicants.size()-1) && !removed_junk; i++)
		{
			for (j = i+1; j < result.implicants.size() && !removed_junk; j++)
			{
				int unneeded_index;
				cout << i << ": " << result.implicants[i] << endl;
				cout << j << ": " << result.implicants[j] << endl;
				unneeded_index = which_index_unneeded(result.implicants[i], result.implicants[j]);
				cout << "Between " << i << " and " << j <<" Unneeded = " << unneeded_index << endl;

				if(unneeded_index != -1)
				{
					result.implicants[i][unneeded_index].data = "X";
					result.implicants[j][unneeded_index].data = "X";
					removed_junk = true;
				}
			}//inner for
		}//Outer for
	} // while
	return result;
}

rule remove_too_strong(rule pr)
{
	rule result = pr;
	if (result.implicants.size() < 2)
		return result;
	//Eliminate all 'unneccisarily strong' guards
	//Totally is a more efficient/logical way to do this
	int i = 0;
	int j = 0;
	bool removed_junk;
	removed_junk = true;
	while(i != result.implicants.size()-1)
	{
		removed_junk = false;
		for (i = 0; (i < result.implicants.size()-1) && !removed_junk; i++)
		{
			for (j = i+1; j < result.implicants.size() && !removed_junk; j++)
			{
				int weaker_result;
				cout << i << ": " << result.implicants[i] << endl;
				cout << j << ": " << result.implicants[j] << endl;
				weaker_result = who_weaker(result.implicants[i], result.implicants[j]);
				cout << "Between " << i << " and " << j <<" who_weaker = " << weaker_result << endl;
				if(weaker_result == -1)
				{
					vector<state>::iterator vi = result.implicants.begin();
					for(int counter = 0; counter < i; counter++)
						vi++;
					result.implicants.erase(vi);
					removed_junk = true;
				}
				else if(weaker_result == 1)
				{
					vector<state>::iterator vi = result.implicants.begin();
					for(int counter = 0; counter < j; counter++)
						vi++;
					result.implicants.erase(vi);
					removed_junk = true;
				}
				else if(weaker_result == 2)
				{
					vector<state>::iterator vi = result.implicants.begin();
					for(int counter = 0; counter < i; counter++)
						vi++;
					result.implicants.erase(vi);
					removed_junk = true;
				}
			}//inner for
		}//Outer for
	} // while
	return result;
}

//Given a single rule, minimize the implicants to that rule
rule minimize_rule(rule pr)
{
	rule result = pr;
	result = remove_too_strong(result);
	result = reduce_to_prime(result);
	result = remove_too_strong(result);
	cout << "finished " << pr.right << endl;
	return result;
}

//Given a vector of rules, minimize every implicant in that vector
vector<rule> minimize_rule_vector(vector<rule> prs)
{
	vector<rule> result = prs;

	for (int i = 0; i < (int)result.size(); i++)
	{
		cout << "trying " << prs[i].right << " ("<< i << ")" << endl;
		result[i] = minimize_rule(result[i]);
	}
	return result;

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
			result_state = diff(leaving_state,incoming_state);
			if(incoming_state.prs)
				result_state.tag = i;
			else
				result_state.tag = -1;

			if(incoming_state.prs || SHOW_ALL_DIFF_STATES)
				delta_space.states.push_back(result_state);
		}
	}
	//TODO: return a graph, too, so that the states mean something mathematically?
	return delta_space;

}
