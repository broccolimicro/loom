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

	cout << vars << endl;

	prgm->print_hse();
	cout << endl;

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


	//Create an up and down PRS for each variable  (UID indexed)
	prs_up.resize(vars.global.size());
	prs_down.resize(vars.global.size());

	//Inserting info into each PRS
	for(int i = 0; i < (int)prs_up.size(); i++)
	{
		prs_up[i].right = vars.get_name(i)+"+";
		prs_up[i].uid = i;
		prs_up[i].up = true;
		prs_down[i].right = vars.get_name(i)+"-";
		prs_down[i].uid = i;
		prs_down[i].up = true;
	}

	//Find the implicants of the diff space
	build_implicants(diff_space);

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

// Counts the number of illegal firings for a certain variable given an implicant
int program::conflict_count(state impl, int fire_uid, string fire_dir)
{
	int count = 0;
	//Look at every state...
	for(int spacei = 0; spacei < space.states.size(); spacei++ )
	{
		int weaker = who_weaker(impl, space.states[spacei]);
		//And if the implicant fires in this state...
		if(weaker == 0 || weaker == 1)
		{
			//Look at all the states this state connects to...
			for(int edgei = 0; edgei < trans.edges[spacei].size(); edgei++)
			{
				//      variable      =         [the uid of the "to" state][the variable we want to know fired].data
				string var_after_edge = space.states[trans.edges[spacei][edgei]][fire_uid].data;
				//And if it isn't an dont care or a desired firing...
				if(var_after_edge != "X" && var_after_edge != fire_dir)
				{
					count++; //Count it as a conflict!
					break;
				}//if
			}//edgei for
		}//if
	}//spaci for
}
//TODO: SOOO UNTESTED
void program::build_implicants(state_space diff_space)
{
	//  ================== TOP DOWN ==================
	if(TOP_DOWN)
	{


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


	}// TOP DOWN
	//  ================== BOTTOM UP ==================
	else
	{

		cout << "Populating up PRS" << endl;
		//====POPULATE PRS UP====
		for(int vari = 0; vari < vars.global.size(); vari++)
		{
			cout << "Outer For" << endl;
			//Look for potential implicants
			for(int diffi = 0; diffi < diff_space.size();diffi++)
			{
				cout << "Inner For" << endl;
				//This will turn into an implicant
				if(diff_space[diffi][vari].data == "1")
				{
					//This is the state that will be added as an implicant
					value v;
					v.data = "X";
					state to_add_impl(v,vars.global.size());

					//List of variables by UID
					list<int> candidates, needed;
					state implier = space[diff_space[diffi].tag];

					//Populate list of candidates
					for(int impi = 0; impi < implier.size();impi++)
					{
						if(implier[impi].data == "0")//TODO: BUBBLE? || (!BUBBLELESS && implier[impi].data == "0"))
							candidates.push_back(impi);
					}

					//We now  have a list of candidate variables to potentially add to the implicant for the firing of diff_space[diffi][vari]
					bool fully_strong = false;
					while(!fully_strong)
					{
						cout << "While Not Fully Strong" << endl;
						int best_candidate = -1;
						int best_count = 999999; //TODO: Make this better

						for(list<int>::iterator candi = candidates.begin(); candi != candidates.end(); candi++)
						{
							cout << "Candi For" << endl;
							state proposed_impl = to_add_impl;
							proposed_impl[*candi] = "0";
							int curr_count = conflict_count(proposed_impl, vari, "1");
							if(curr_count < best_count)
							{
								best_count = curr_count;
								best_candidate = *candi;
							}
						}//candi for

						//At this point, we should have selected the var that causes the fewest conflict states.
						//Add it to our implicant!
						if(best_candidate == -1)
							fully_strong = true; //No one left to add
						else
						{
							to_add_impl[best_candidate].data = "0"; //add a 0 to the implicant in the spot of the candidate var
							candidates.remove(best_candidate);
						}
						if(best_count == 0)
							fully_strong = true; //We had no conflicts. Avoid strengthening

					}//fully_strong while
					//The implicant that we just spent a while making? Add that.
					prs_up[vari].implicants.push_back(to_add_impl);
				}//if
			}//diffi for
		}//vari for

		cout << "Populating down PRS" << endl;

		//====POPULATE PRS DOWN====
		for(int vari = 0; vari < vars.global.size(); vari++)
		{
			//Look for potential implicants
			for(int diffi = 0; diffi < diff_space.size();diffi++)
			{
				//This will turn into an implicant
				if(diff_space[diffi][vari].data == "0")
				{
					//This is the state that will be added as an implicant
					value v;
					v.data = "X";
					state to_add_impl(v,vars.global.size());

					//List of variables by UID
					list<int> candidates, needed;
					state implier = space[diff_space[diffi].tag];

					//Populate list of candidates
					for(int impi = 0; impi < implier.size();impi++)
					{
						if(implier[impi].data == "1")//TODO: BUBBLE? || (!BUBBLELESS && implier[impi].data == "0"))
							candidates.push_back(impi);
					}

					//We now  have a list of candidate variables to potentially add to the implicant for the firing of diff_space[diffi][vari]
					bool fully_strong = false;
					while(!fully_strong)
					{
						int best_candidate = -1;
						int best_count = 999999; //TODO: Make this better

						for(list<int>::iterator candi = candidates.begin(); candi != candidates.end(); candi++)
						{
							state proposed_impl = to_add_impl;
							proposed_impl[*candi] = "1";
							int curr_count = conflict_count(proposed_impl, vari, "0");
							if(curr_count < best_count)
							{
								best_count = curr_count;
								best_candidate = *candi;
							}
						}//candi for

						//At this point, we should have selected the var that causes the fewest conflict states.
						//Add it to our implicant!
						if(best_candidate == -1)
							fully_strong = true; //No one left to add
						else
						{
							to_add_impl[best_candidate].data = "1"; //add a 1 to the implicant in the spot of the candidate var
							candidates.remove(best_candidate);
						}
						if(best_count == 0)
							fully_strong = true; //We had no conflicts. Avoid strengthening

					}//fully_strong while
					//The implicant that we just spent a while making? Add that.
					prs_down[vari].implicants.push_back(to_add_impl);
				}//if
			}//diffi for
		}//vari for

		cout << "Done Bottom Up" << endl;

	}//BOTTOM UP
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

		// ================== PRS UP ==================
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

		// ================== PRS DOWN ==================
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

/*
//TODO: Test test!!
void program::weaken_guard(rule pr)
{
	//Go through every implicant of the rule
	for(int impi = 0; impi < pr.implicants.size(); impi++)
	{
		//Go through every variable of the given implicant
		for(int vari = 0; vari < pr.implicants[impi].size(); vari++)
		{
			//proposal will be the given implicant missing the vari-th variable
			state proposal = pr.implicants[impi];
			proposal[vari].data = "X";
			//Compare this proposal to the whole state space
			bool not_needed = true;
			for(int spacei = 0; spacei < space.states.size(); spacei++)
			{
				int weaker = who_weaker(proposal,space.states[spacei]);
				//If the current state is weaker than our proposal, or they are the same...
				if(weaker == 0 || weaker == 2)
				{
					//Check if it is not allowed to fire here
					if((space.states[spacei][pr.uid] == "1"&& pr.up == false) || (space.states[spacei][pr.uid] == "0"&& pr.up == true))
						not_needed = false;
				}//if

			}//spaci for

		}//vari for
	}//impi for
}
*/

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
