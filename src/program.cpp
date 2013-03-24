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
	insert_state_vars();
	generate_prs();
	cout << endl << endl<< "Done!" << endl;

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
	prs = p.prs;
	errors = p.errors;
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
						type_space.insert(pair<string, operate*>(c->name + "." + c->send->name, c->send));
						type_space.insert(pair<string, operate*>(c->name + "." + c->recv->name, c->recv));
						type_space.insert(pair<string, operate*>(c->name + "." + c->probe->name, c->probe));
					}
				}
			}
			j = i+1;
		}
	}

	vars.insert(variable("Reset", "int", 1, false));
	vars.insert(variable("_Reset", "int", 1, false));

	prgm = (parallel*)expand_instantiation(NULL, "main _()", &vars, NULL, "", verbosity, true);

	cout << vars << endl;

	prgm->print_hse();
	cout << endl;
}

/* TODO Projection algorithm - when do we need to do projection? when shouldn't we do projection?
 * TODO Process decomposition - How big should we make processes?
 * TODO There is a problem with the interaction of scribe variables with bubbleless reshuffling because scribe variables insert bubbles
 */
void program::generate_states()
{
	cout << "Generating State Space" << endl;

	space.append_state(state(value("X"), vars.global.size()), -1, "Power On");
	prgm->generate_states(&space, 0, state());
	space.gen_traces();
	prgm->generate_scribes();
	space.gen_deltas();
	space.gen_conflicts();

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

void program::insert_state_vars()
{
	int i, j, k, l, m;
	int w, h;

	vector<int> up, down;
	vector<bool> rup, rdown;
	trace values;
	int benefit;

	vector<int> best_up, best_down;
	trace best_values;
	int best_benefit;

	list<pair<int, int> > up_conflict_firings;
	list<pair<int, int> > down_conflict_firings;

	list<pair<int, int> >::iterator ci;

	int cc0, cc1;

	timeval t0, t1, t2;

	/* THIS IS A STRAIGHTFORWARD BRUTE FORCE ALGORITHM. IT IS NOT SMART. IT IS NOT FAST. BUT IT WORKS.
	 *
	 * This works for loops, conditionals, blocks, assignments, and guards. This does not work for parallel (yet) because it thinks that it has to
	 * separate branches of a parallel block with a state variable transition... This is mildly problematic because it cannot be solved by an
	 * examination of the state space alone. To solve this problem, you need to examine the parse tree as well. This will probably be solved with
	 * a check in the function that generates the conflicting state list and maybe a parallel block id and branch id. The idea is to not generate a
	 * conflicting pair if the parallel block id of both states is the same, but the branch id of both state is different.
	 *
	 * This algorithms execution time balloons very quickly and becomes very very very fucking slow, BUT! it does indeed calculate the optimal
	 * state variable insertion points and resulting trace. So... projection + process decomposition to keep the space we are looking at very small?
	 * or find a less optimal, faster way?
	 */

	/* Turn this knob to control how many up transitions are allowed and how many down transitions are allowed. For a state space of size 25,
	 * 1 is instant, 2 takes exactly 1 minute, and 3 takes hours. This is the glory of a brute force method.
	 */
	int dimension = 1;

	best_benefit = 1;
	for (m = 0; m < 10 && best_benefit > 0; m++)
	{
		/* Step 1: cache the list of conflict firings already present in the state space. This does not include conflicts between two
		 * states in which neither states are a firing for a variable.
		 */

		gettimeofday(&t0, NULL);

		up_conflict_firings.clear();
		down_conflict_firings.clear();

		// Calculate the list of up production rule conflict pairs
		for (i = 0; i < (int)space.up_firings.size(); i++)
			for (k = 0; k < (int)space.up_firings[i].size(); k++)
				for (j = 0; j < (int)space.up_conflicts[space.up_firings[i][k]].size(); j++)
					up_conflict_firings.push_back(pair<int, int>(space.up_firings[i][k], space.up_conflicts[space.up_firings[i][k]][j]));

		// Eliminate duplicate pairs so that we can get an accurate measure
		up_conflict_firings.sort();
		up_conflict_firings.unique();

		cout << "Up Production Rule Conflicts" << endl;
		for (ci = up_conflict_firings.begin(); ci != up_conflict_firings.end(); ci++)
			cout << "[" << ci->first << "," << ci->second << "]";
		cout << endl;

		// Calculate the list of down production rule conflict pairs
		for (i = 0; i < (int)space.down_firings.size(); i++)
			for (k = 0; k < (int)space.down_firings[i].size(); k++)
				for (j = 0; j < (int)space.down_conflicts[space.down_firings[i][k]].size(); j++)
					down_conflict_firings.push_back(pair<int, int>(space.down_firings[i][k], space.down_conflicts[space.down_firings[i][k]][j]));

		// Eliminate duplicate pairs so that we can get an accurate measure
		down_conflict_firings.sort();
		down_conflict_firings.unique();

		cout << "Down Production Rule Conflicts" << endl;
		for (ci = down_conflict_firings.begin(); ci != down_conflict_firings.end(); ci++)
			cout << "[" << ci->first << "," << ci->second << "]";
		cout << endl;

		gettimeofday(&t1, NULL);

		cc0 = cc1;
		cc1 = up_conflict_firings.size() + down_conflict_firings.size();
		cout << cc0 - cc1 << " conflicts eliminated. " << best_benefit << " conflicts promised." << endl;

		cout << "Conflicts left to eliminate: " << up_conflict_firings.size() << " " << down_conflict_firings.size() << endl;

		cout << "Step 1: " << ((double)(t1.tv_sec - t0.tv_sec) + 0.000001*(double)(t1.tv_usec - t0.tv_usec)) << " seconds" << endl;

		/* Step 2: Generate a multidimensional grid with a side length equal to the number of states in the state space and a dimension
		 * equal to the number of up firings plus down firings allowed. The overall size of this grid is space.size^(up.size + down.size). Which
		 * means for a state space of 25 states and 4 possible up firings and 4 possible down firings, there are 25^8 or about 1.526E11 elements
		 * to iterate trough. Each element is a calculated benefit value that represents the total number of conflicts that would be eliminated
		 * were there to be a state variable with up transitions at (x0, y0, z0, ...) and down transitions at (x1, y1, z1, ...) inserted into the
		 * variable space.
		 *
		 * Then, take the element with the max benefit value at address (x0, y0, z0, ...) and (x1, y1, z1, ...), insert into the variable space,
		 * update the state space, update the conflict list, repeat. Theoretically, we should repeat until the maximum benefit achievable is 0
		 * (we cannot eliminate any more conflicts). But this doesn't ever seem to happen. It seems to level off at a maximum benefit of 5.
		 */
		best_benefit = 0;
		up.assign(dimension, -1);
		down.assign(dimension, -1);
		best_up.assign(dimension, -1);
		best_down.assign(dimension, -1);
		w = pow(space.size(), up.size());
		h = pow(space.size(), down.size());
		bool overlap;
		for (i = 0; i < w; i++)
		{
			// Set up our up transition indices: (x0, y0, z0, ...)
			rup.assign(space.size(), false);
			l = 1;
			for (k = 0; k < (int)up.size(); k++)
			{
				up[k] = (i / l)%space.size();
				rup[up[k]] = true;
				l *= space.size();
			}

			for (j = 0; j < h; j++)
			{
				// Set up our down transition indices: (x1, y1, z1, ...)
				rdown.assign(space.size(), false);
				l = 1;
				for (k = 0; k < (int)down.size(); k++)
				{
					down[k] = (j / l)%space.size();
					rdown[down[k]] = true;
					l *= space.size();
				}

				// Check to make sure that we aren't dealing with an up transition happening at the same time as a down transition.
				benefit = 0;
				overlap = false;
				for (k = 0; k < (int)up.size() && !overlap; k++)
					for (l = 0; l < (int)down.size() && !overlap; l++)
						if (up[k] == down[l])
							overlap = true;
				if (!overlap)
				{
					// Get the trace given the up firings at (x0, y0, z0, ...) and down firings at (x1, y1, z1, ...)
					values.values.assign(space.size(), value("_"));
					values[0] = value("X");
					space.get_trace(0, &rup, &rdown, &values);

					// TODO I am counting wrong here... the algorithm is too optimistic right now

					// Add the number of up production rule conflicts eliminated in this trace to the benefit
					for (ci = up_conflict_firings.begin(); ci != up_conflict_firings.end(); ci++)
						if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
							(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
							benefit++;

					// Add the number of down production rule conflicts eliminated in this trace to the benefit
					for (ci = down_conflict_firings.begin(); ci != down_conflict_firings.end(); ci++)
						if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
							(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
							benefit++;

					// Subtract the number of up production rule conflicts added by adding this variable with these up firings from the benefit
					for (k = 0; k < (int)up.size(); k++)
						for (l = 0; l < (int)space.up_conflicts[up[k]].size(); l++)
							if (values[space.up_conflicts[up[k]][l]].data[0] == '0')
								benefit--;

					// Subtract the number of down production rule conflicts added by adding this variable with these down firings from the benefit
					for (k = 0; k < (int)down.size(); k++)
						for (l = 0; l < (int)space.down_conflicts[down[k]].size(); l++)
							if (values[space.down_conflicts[down[k]][l]].data[0] == '1')
								benefit--;

					// Check to see if these firings yield the most benefit
					if (benefit > best_benefit)
					{
						best_up = up;
						best_down = down;
						best_benefit = benefit;
						best_values = values;
					}
				}
				cout << benefit << "\t";
			}
			cout << endl;
		}
		cout << endl;

		gettimeofday(&t2, NULL);

		cout << "Step 2: " << ((double)(t2.tv_sec - t1.tv_sec) + 0.000001*(double)(t2.tv_usec - t1.tv_usec)) << " seconds" << endl;
		cout << "Total: " << ((double)(t2.tv_sec - t0.tv_sec) + 0.000001*(double)(t2.tv_usec - t0.tv_usec)) << " seconds" << endl;

		cout << endl << endl << best_benefit << " Conflicting Paths Eliminated" << endl;
		cout << "Up:\t";
		for (k = 0; k < (int)best_up.size(); k++)
			cout << best_up[k] << ", ";
		cout << endl;
		cout << "Down:\t";
		for (k = 0; k < (int)best_down.size(); k++)
			cout << best_down[k] << ", ";
		cout << endl;
		cout << best_values << endl << endl;

		// Insert new variable
		int vid = vars.insert(variable(vars.unique_name("_sv"), "int", 1, false));
		cout << *(vars.find(vid)) << endl;

		// Update the space
		space.set_trace(vid, best_values);
		i = space.duplicate_state(best_up[0]);
		space.states[i][vid] = value("1");
		space.states[i].prs = true;
		space.traces[vid][i] = value("1");
		i = space.duplicate_state(best_down[0]);
		space.states[i][vid] = value("0");
		space.states[i].prs = true;
		space.traces[vid][i] = value("0");

		// Recalculate the firings and conflicts lists
		space.gen_deltas();
		space.gen_conflicts();
	}

	cout << space << endl;
}

void program::generate_prs()
{
	for (int vi = 0; vi < space.width(); vi++)
		if (vars.get_name(vi).find_first_of("|&~") == string::npos && vars.get_name(vi).find("Reset") == string::npos)
			prs.push_back(rule(vi, &space, &vars));

	print_prs();
}

/* TODO: Factoring - production rules should be relatively short.
 * Look for common expressions between production rules and factor them
 * out into their own variable
 */
void program::factor_prs()
{

}

void program::print_prs()
{
	cout << "Production Rules: " << endl;

	for (size_t i = 0; i < prs.size(); i++)
		cout << prs[i];
}

/*
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

