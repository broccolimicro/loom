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
	generate_states();
	generate_prs();
	cout << "Did I get here?" << endl;
	//insert_state_vars();
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
}

void program::generate_states()
{
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
	s.prs = true;
	space.insert(sr, -1, "Power On");
	space.insert(s, 0, "Reset");

	cout << "Generating State Space" << endl;
	prgm->generate_states(&space, 1);
	space.close();

	if(STATESP_CO)
	{
		cout << vars << endl;
		cout << space << endl << endl;
		cout >> space << endl << endl;
		space.print_up();
		space.print_down();

		space.print_delta();
	}
	if(STATESP_GR)
	{
		space.print_dot();
	}
}

void program::generate_prs()
{
	//Create an up and down PRS for each variable  (UID indexed)
	prs_up.resize(vars.global.size());
	prs_down.resize(vars.global.size());

	//Inserting info into each PRS
	for(size_t i = 0; i < prs_up.size(); i++)
	{
		prs_up[i].right = vars.get_name(i)+"+";
		prs_up[i].uid = i;
		prs_up[i].up = true;
		prs_down[i].right = vars.get_name(i)+"-";
		prs_down[i].uid = i;
		prs_down[i].up = true;
	}

	//Find the implicants of the diff space
	build_implicants();

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

void program::insert_state_vars()
{
	//Indistinguishable states before PRS?

	//The below, or a totally crazy idea:
	//Make a datastructure containing 'indistinguishable from's for each state
	//Somehow shrink these lists so they don't matter anymore or something. Magic.
	//Other thoughts: Would it be dumb to iterate through once with all implicants instead?

	//Set up data structures
	vector<trace> up_conflicts;
	vector<trace> down_conflicts;

	up_conflicts.resize(prs_up.size());
	down_conflicts.resize(prs_down.size());
	for(size_t i = 0; i < prs_up.size(); i ++)
	{
		up_conflicts[i].values.resize(space.size());
		down_conflicts[i].values.resize(space.size());
	}

	cout << " SV up start " << endl;
	//===== SEARCH FOR NEEDED UP UP ======
	// === iterate through every rule's implicants
	for(size_t rulei = 0; rulei < prs_up.size(); rulei++)
	{
		cout << " rulei " << endl;
		// === For each implicant...
		for(size_t impi = 0; impi < prs_up[rulei].implicants.size(); impi++)
		{
			cout << " impi " << endl;
			//...iterate once through the state space.
			for(int statei = 0; statei < space.size(); statei++)
			{
				cout << " statei " << statei<< endl;
				//Write down if the state is an okay firing, conflict firing, or mandatory firing (vector<trace>)
				int weaker = who_weaker(prs_up[rulei].implicants[impi], space.states[statei]);
				//cout << "prs_up[rulei].implicants[impi]" << prs_up[rulei].implicants[impi] << " space.states[statei] " << space.states[statei] << endl;
				//It is supposed to fire here!
				cout << " who weaker: " << weaker << " prs_up[rulei].implicants[impi].tag " << prs_up[rulei].implicants[impi].tag << endl;
				if(statei == (prs_up[rulei].implicants[impi].tag))
					up_conflicts[rulei][statei].data = "!";
				//It doesn't fire here.
				else if(weaker == 0 || weaker == 2)
					up_conflicts[rulei][statei].data = "_";
				//It fires here. Should it?
				else if(weaker == -1 || weaker == 1)
				{
					//TODO! Figure out if it may fire here.
					//(space[statei][prs_up[rulei].uid]=="0"))
					up_conflicts[rulei][statei].data = "C";
				}
				else
					up_conflicts[rulei][statei].data = "?";
				cout << "bottom of loop " << endl;
			} //statei for
		}//impi for
	}//rulei for

	cout << "Up conflict traces:" << endl;
	for (size_t i = 0; i < prs_up.size(); i++)
		cout<< prs_up[i].right << " " << up_conflicts[i] << endl;

	// === Chose indices in state space to insert state variables
	// === Insert these into the CHP and reparse? Is better way?



	//===== SEARCH FOR NEEDED DOWN SV ======
	// === iterate through every rule's implicants

	cout << "Done state var insert" << endl;
}

// Counts the number of illegal firings for a certain variable given an implicant
int program::conflict_count(state impl, int fire_uid, string fire_dir)
{
	int count = 0;
	string var_after_edge;
	//Look at every state...
	for(size_t spacei = 0; spacei < space.states.states.size(); spacei++ )
	{
		int weaker = who_weaker(impl, space.states.states[spacei]);
		//And if the implicant fires in this state...
		if(weaker == 0 || weaker == 1)
		{
			var_after_edge = "X";
			//THE ABOVE IS INDEX OUT OF BOUNDS AT 42. TODO TODO TODO TODO .

			//Look at all the states this state connects to...
			for(size_t edgei = 0; edgei < space.edges[spacei].size() && (var_after_edge == "X" || var_after_edge == fire_dir); edgei++)
			{

				//      variable      =         [the uid of the "to" state][the variable we want to know fired].data
				var_after_edge = space.states.states[space.edges[spacei][edgei]][fire_uid].data;
				//And if it isn't an dont care or a desired firing...

				if(var_after_edge != "X" && var_after_edge != fire_dir)
				{
					count++; //Count it as a conflict!
					//break;
				}//if
			}//edgei for
		}//if
	}//spaci for
	return count;
}

int conflict(trace proposed, trace desired)
{
	int c = 0;
	vector<value>::iterator i, j;
	for (i = proposed.begin(), j = desired.begin(); i != proposed.end() && j != desired.end(); i++, j++)
		if (i->data != "0" && j->data == "0")
			c++;

	return c;
}

//TODO: SOOO UNTESTED
void program::build_implicants()
{
	list<int> candidates;
	state implier;
	bool fully_strong;

	int best_candidate;
	int best_count;
	int curr_count;
	int vi, vj, ii;
	trace proposal_output;
	list<int>::iterator candi;

	list<int> invars;
	list<int>::iterator vk;
	state implicant;
	trace implicant_output;
	int count, mcount, var;

	//  ================== TOP DOWN ==================
	if(TOP_DOWN)
	{
		cout << "Doing top down. I'm not sure that is a good idea. Hasn't been tested recently. " << endl;

		/*for(size_t i = 0; i < diff_space.size();i++)
		{
			for(int j = 0; j< diff_space[i].size(); j++)
			{
				if((diff_space[i])[j].data == "1")
				{
					if(diff_space[i].tag!=-1)	//Output variable needs to fire high
						prs_up[j].implicants.push_back(space.states[diff_space[i].tag]);
				}
				if((diff_space[i])[j].data == "0")
				{
					if(diff_space[i].tag!=-1)	//Output variable needs to fire low
						prs_down[j].implicants.push_back(space.states[diff_space[i].tag]);
				}
			}
		}*/


	}// TOP DOWN
	//  ================== BOTTOM UP ==================
	else
	{
		// Up Production Rules
		for (vi = 0; vi < (int)space.up_firings.size(); vi++)
		{
			for (ii = 0; ii < (int)space.up_firings[vi].size(); ii++)
			{
				implier			 = space.states[space.up_firings[vi][ii]];
				implicant		 = state(value("X"), space.width());
				implicant_output = trace(value("1"), space.size());

				invars.clear();
				for (vj = 0; vj < space.width(); vj++)
					if (((BUBBLELESS && implier[vj].data == "0") || !BUBBLELESS) && vj != vi)
						invars.push_back(vj);

				mcount = 9999999;
				var = 0;
				while (invars.size() > 0 && var != -1)
				{
					var = -1;
					for (vk = invars.begin(); vk != invars.end(); vk++)
					{
						proposal_output = implicant_output & ~space.delta[*vk];

						count = conflict(proposal_output, space.up[vi]);
						if (count < mcount)
						{
							mcount = count;
							var = *vk;
						}
					}

					if (var != -1)
					{
						implicant[var] = value("0");
						implicant_output = implicant_output & ~space.delta[var];
						invars.remove(var);
					}
				}

				implicant.tag = space.up_firings[vi][ii];
				prs_up[vi].implicants.push_back(implicant);
			}
		}

		// Down Production Rules
		for (vi = 0; vi < (int)space.down_firings.size(); vi++)
		{
			for (ii = 0; ii < (int)space.down_firings[vi].size(); ii++)
			{
				implier			 = space.states[space.down_firings[vi][ii]];
				implicant		 = state(value("X"), space.width());
				implicant_output = trace(value("1"), space.size());

				invars.clear();
				for (vj = 0; vj < space.width(); vj++)
					if (((BUBBLELESS && implier[vj].data == "1") || !BUBBLELESS) && vj != vi)
						invars.push_back(vj);

				mcount = 9999999;
				var = 0;
				while (invars.size() > 0 && var != -1)
				{
					var = -1;
					for (vk = invars.begin(); vk != invars.end(); vk++)
					{
						proposal_output = implicant_output & space.delta[*vk];

						count = conflict(proposal_output, space.down[vi]);
						if (count < mcount)
						{
							mcount = count;
							var = *vk;
						}
					}

					if (var != -1)
					{
						implicant[var] = value("1");
						implicant_output = implicant_output & space.delta[var];
						invars.remove(var);
					}
				}

				implicant.tag = space.down_firings[vi][ii];
				prs_down[vi].implicants.push_back(implicant);
			}
		}
	}
}

//Merges all the implicants and puts them into the .left fields
void program::merge_implicants()
{
	//Remove whatever might have been in there before
	for(size_t i = 0; i < prs_up.size(); i++)
	{
		prs_up[i].left.clear();
		prs_down[i].left.clear();
	}

	map<string, variable>::iterator globali = vars.global.begin();
	for (size_t i = 0; i< prs_up.size(); i++, globali++)
	{
		//Print out the implicants
		//for(int j = 0; j < prs_up[i].implicants.size(); j++)
		//	cout << i << "+ "<<   prs_up[i].implicants[j] << endl;
		//for(int j = 0; j < prs_down[i].implicants.size(); j++)
		//	cout << i << "- "<< prs_down[i].implicants[j] << endl;

		// ================== PRS UP ==================
		for(size_t upi = 0; upi<prs_up[i].implicants.size(); upi++)
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
		for(size_t downi = 0; downi<prs_down[i].implicants.size(); downi++)
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

	for (size_t i = 0; i< prs_up.size(); i++, globali++)
	{
		if (prs_up[i].left != "")
		{
			cout << prs_up[i].left << " -> " << prs_up[i].right << " ";
			print_implicant_tags(prs_up[i].implicants);
			cout<< endl;
		}
		if (prs_down[i].left != "")
		{
			cout << prs_down[i].left << " -> " << prs_down[i].right  << " ";
			print_implicant_tags(prs_down[i].implicants);
			cout<< endl;
		}

	}

}

/*
//TODO: Test test!!
 * Do not delete without Nicholas' consent!! He has not proven that the construction method
 * used during bottom up will yield the same results! In fact, he thinks there is a defininte
 * region of the design space where this is not the case.
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

