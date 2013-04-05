#include "program.h"
#include "utility.h"

program::program()
{
	type_space.insert(pair<string, keyword*>("int", new keyword("int")));
	vars.types = &type_space;
}

program::program(string chp, int verbosity)
{
	this->verbosity = verbosity;
	vars.types = &type_space;

	// CHP to HSE
	parse(chp, verbosity);
	merge();
	//project();
	//decompose();
	//reshuffle();

	// HSE to State Space
	generate_states();
	insert_scribe_vars();
	insert_state_vars();

	// State Space to PRS
	generate_prs();
	//factor_prs();
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

	// Remove line comments:
	size_t comment_begin = chp.find("//");
	size_t comment_end = chp.find("\n", comment_begin);
	while (comment_begin != chp.npos && comment_end != chp.npos){
		chp = chp.substr(0,comment_begin) + chp.substr(comment_end);
		comment_begin = chp.find("//");
		comment_end = chp.find("\n", comment_begin);
	}

	// Remove block comments:
	comment_begin = chp.find("/*");
	comment_end = chp.find("*/");
	while (comment_begin != chp.npos && comment_end != chp.npos){
		chp = chp.substr(0,comment_begin) + chp.substr(comment_end+2);
		comment_begin = chp.find("/*");
		comment_end = chp.find("*/");
	}

	// Remove extraneous whitespace
	for (i = chp.begin(); i != chp.end(); i++)
	{
		if (!sc(*i))
			cleaned_chp += *i;
		else if (nc(*(i-1)) && (i == chp.end()-1 || nc(*(i+1))))
			cleaned_chp += ' ';
	}

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

	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
		cout << vars << endl;

	if (verbosity & VERB_BASE_HSE)
	{
		prgm->print_hse("");
		cout << endl;
	}
}

void program::merge()
{
	prgm->merge();

	if (verbosity & VERB_MERGED_HSE)
	{
		prgm->print_hse("");
		cout << endl;
	}
}

// TODO Projection algorithm - when do we need to do projection? when shouldn't we do projection?
void program::project()
{
	if (verbosity & VERB_PROJECTED_HSE)
	{
		prgm->print_hse("");
		cout << endl;
	}
}

// TODO Process decomposition - How big should we make processes?
void program::decompose()
{
	if (verbosity & VERB_DECOMPOSED_HSE)
	{
		prgm->print_hse("");
		cout << endl;
	}
}

// TODO Handshaking Reshuffling
void program::reshuffle()
{
	if (verbosity & VERB_RESHUFFLED_HSE)
	{
		prgm->print_hse("");
		cout << endl;
	}
}

// TODO There is a problem with the interaction of scribe variables with bubbleless reshuffling because scribe variables insert bubbles
void program::generate_states()
{
	space.append_state(state(value("X"), vars.global.size()), -1, "Power On");
	//TODO: Okay, I am going to say it. I think it is weird that we are passing a list of instructions to parallel to state space.
	//It does not feel like a syntax structure's job to do that kind of thing.
	prgm->generate_states(&space, 0, state());
	space.gen_traces();

	if (verbosity & VERB_BASE_STATE_SPACE)
		space.print_states(&vars);
	if (verbosity & VERB_BASE_STATE_SPACE_DOT)
		space.print_dot();
}

void program::insert_scribe_vars()
{
	prgm->generate_scribes();

	if (verbosity & VERB_SCRIBEVAR_STATE_SPACE)
		space.print_states(&vars);
	if (verbosity & VERB_SCRIBEVAR_STATE_SPACE_DOT)
		space.print_dot();
}

void program::insert_state_vars()
{
	srand(time(0));

	int i, j, k, l, m;
	int w, h;
	int vid = -1;

	vector<int> up, down;
	vector<bool> rup, rdown;
	trace values;
	int benefit;

	int up_benefit;
	int down_benefit;
	int up_deficit;
	int down_deficit;

	vector<int> best_up, best_down;
	vector<bool> best_rup, best_rdown;
	trace best_values;
	int best_benefit;

	list<pair<int, int> > up_conflict_firings;
	list<pair<int, int> > down_conflict_firings;

	list<pair<int, int> > old_up_conflict_firings;
	list<pair<int, int> > old_down_conflict_firings;

	list<pair<int, int> >::iterator ci;

	int cc0, cc1;

	timeval t0, t1;

	/* THIS IS A STRAIGHTFORWARD BRUTE FORCE ALGORITHM. IT IS NOT SMART. IT IS NOT FAST. BUT IT WORKS.
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
	gettimeofday(&t1, NULL);
	for (m = 0; m < 30 && best_benefit > 0; m++)
	{
		t0 = t1;
		gettimeofday(&t1, NULL);
		/*space.print_traces(&vars);
		space.print_up(&vars);
		space.print_down(&vars);
		space.print_delta(&vars);
		space.print_conflicts();
		space.print_firings(&vars);*/

		space.gen_deltas();
		space.gen_conflicts();

		/* Step 1: cache the list of conflict firings already present in the state space. This does not include conflicts between two
		 * states in which neither states are a firing for a variable.
		 */

		up_conflict_firings.clear();
		down_conflict_firings.clear();

		// Calculate the list of up production rule conflict pairs
		for (i = 0; i < (int)space.up_firings.size(); i++)
			for (k = 0; k < (int)space.up_firings[i].size(); k++)
				for (j = 0; j < (int)space.up_conflicts[space.up_firings[i][k]].size(); j++)
					if (space.traces[i][space.up_conflicts[space.up_firings[i][k]][j]].data[0] != '1')
						up_conflict_firings.push_back(pair<int, int>(space.up_firings[i][k], space.up_conflicts[space.up_firings[i][k]][j]));

		// Eliminate duplicate pairs so that we can get an accurate measure
		up_conflict_firings.sort();
		up_conflict_firings.unique();

		// Calculate the list of down production rule conflict pairs
		for (i = 0; i < (int)space.down_firings.size(); i++)
			for (k = 0; k < (int)space.down_firings[i].size(); k++)
				for (j = 0; j < (int)space.down_conflicts[space.down_firings[i][k]].size(); j++)
					if (space.traces[i][space.down_conflicts[space.down_firings[i][k]][j]].data[0] != '0')
						down_conflict_firings.push_back(pair<int, int>(space.down_firings[i][k], space.down_conflicts[space.down_firings[i][k]][j]));

		// Eliminate duplicate pairs so that we can get an accurate measure
		down_conflict_firings.sort();
		down_conflict_firings.unique();

		cc0 = m == 0 ? up_conflict_firings.size() + down_conflict_firings.size() : cc1;
		cc1 = up_conflict_firings.size() + down_conflict_firings.size();

		if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
		{
			if (m == 0)
				cout << "Remainder:      " << cc1 << endl;
			else
			{
				cout << "Actual Delta:   " << cc0 - cc1 << endl;
				cout << "Guessed Delta:  " << best_benefit << endl;
				cout << "Remainder:      " << cc1 << endl;
				cout << "Iteration Time: " << ((double)(t1.tv_sec - t0.tv_sec) + 0.000001*(double)(t1.tv_usec - t0.tv_usec)) << " seconds" << endl;
				cout << endl << endl;
			}
		}

		if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
		{
			cout << "Up Conflicts:   ";
			for (ci = up_conflict_firings.begin(); ci != up_conflict_firings.end(); ci++)
				cout << "[" << ci->first << "," << ci->second << "]";
			cout << endl;
		}

		if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
		{
			cout << "Down Conflicts: ";
			for (ci = down_conflict_firings.begin(); ci != down_conflict_firings.end(); ci++)
				cout << "[" << ci->first << "," << ci->second << "]";
			cout << endl;
		}

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
				up_benefit = 0;
				down_benefit = 0;
				up_deficit = 0;
				down_deficit = 0;
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

					// TODO counting is off by one for up conflicts and down conflicts each. Making the total error a maximum of 2

					// Add the number of up production rule conflicts eliminated in this trace to the benefit
					for (ci = up_conflict_firings.begin(); ci != up_conflict_firings.end(); ci++)
					{
						if (rup[ci->first])
						{
							if (values[ci->second].data[0] == '0')
								up_benefit++;

							if (!((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0')) && find(space.up_conflicts[ci->first].begin(), space.up_conflicts[ci->first].end(), ci->second) == space.up_conflicts[ci->first].end())
								up_benefit--;
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								up_benefit--;

							if (rup[ci->second] && find(space.up_conflicts[ci->first].begin(), space.up_conflicts[ci->first].end(), ci->second) == space.up_conflicts[ci->first].end())
								up_benefit--;
						}
						else if (rdown[ci->first])
						{
							if (values[ci->second].data[0] == '1')
								up_benefit++;

							if (!((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0')) && find(space.down_conflicts[ci->first].begin(), space.down_conflicts[ci->first].end(), ci->second) == space.down_conflicts[ci->first].end())
								down_benefit--;
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								down_benefit--;

							if (rdown[ci->second] && find(space.down_conflicts[ci->first].begin(), space.down_conflicts[ci->first].end(), ci->second) == space.down_conflicts[ci->first].end())
								up_benefit--;
						}
						else
						{
							if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
								up_benefit++;

							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								up_benefit--;
						}
					}

					// Add the number of down production rule conflicts eliminated in this trace to the benefit
					for (ci = down_conflict_firings.begin(); ci != down_conflict_firings.end(); ci++)
					{
						if (rup[ci->first])
						{
							if (values[ci->second].data[0] == '0')
								down_benefit++;

							if (!((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0')) && find(space.up_conflicts[ci->first].begin(), space.up_conflicts[ci->first].end(), ci->second) == space.up_conflicts[ci->first].end())
								up_benefit--;
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								up_benefit--;

							if (rup[ci->second] && find(space.up_conflicts[ci->first].begin(), space.up_conflicts[ci->first].end(), ci->second) == space.up_conflicts[ci->first].end())
								down_benefit--;
						}
						else if (rdown[ci->first])
						{
							if (values[ci->second].data[0] == '1')
								down_benefit++;

							if (!((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0')) && find(space.down_conflicts[ci->first].begin(), space.down_conflicts[ci->first].end(), ci->second) == space.down_conflicts[ci->first].end())
								down_benefit--;
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								down_benefit--;

							if (rdown[ci->second] && find(space.down_conflicts[ci->first].begin(), space.down_conflicts[ci->first].end(), ci->second) == space.down_conflicts[ci->first].end())
								down_benefit--;
						}
						else
						{
							if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
								down_benefit++;

							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								down_benefit--;
						}
					}

					// Below this comment works completely, the bug is in the above code.

					// Subtract the number of up production rule conflicts added by adding this variable with these up firings from the benefit
					for (k = 0; k < (int)up.size(); k++)
					{
						for (l = 0; l < (int)space.up_conflicts[up[k]].size(); l++)
						{
							if (values[space.up_conflicts[up[k]][l]].data[0] != '1')
								up_deficit++;
							if (rdown[space.up_conflicts[up[k]][l]])
								up_deficit++;
						}

						if (values[up[k]].data[0] != '0')
							up_benefit--;
					}

					// Subtract the number of down production rule conflicts added by adding this variable with these down firings from the benefit
					for (k = 0; k < (int)down.size(); k++)
					{
						for (l = 0; l < (int)space.down_conflicts[down[k]].size(); l++)
						{
							if (values[space.down_conflicts[down[k]][l]].data[0] != '0')
								down_deficit++;
							if (rup[space.down_conflicts[down[k]][l]])
								down_deficit++;
						}
						if (values[down[k]].data[0] != '1')
							down_benefit--;
					}

					benefit = up_benefit + down_benefit - up_deficit - down_deficit;
					// Check to see if these firings yield the most benefit
					if (benefit > best_benefit)
					{
						best_up = up;
						best_down = down;
						best_rup = rup;
						best_rdown = rdown;
						best_benefit = benefit;
						best_values = values;
					}
				}
				if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
					cout << benefit << "\t";
			}
			if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
				cout << endl;
		}
		if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
			cout << endl;

		if (best_benefit != 0)
		{
			// Insert new variable
			vid = vars.insert(variable(vars.unique_name("_sv"), "int", 1, false));

			if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
				cout << "Inserting:      " << *(vars.find(vid)) << "\t\t" << best_values << endl;

			// Update the space
			space.set_trace(vid, best_values);

			// TODO inserting assignments that happen as a result of reset (uid = 0)

			sort(best_up.begin(), best_up.end());
			unique(best_up.begin(), best_up.end());
			for (j = 0; j < (int)best_up.size(); j++)
			{
				i = space.duplicate_state(best_up[j]);
				space.states[i][vid] = value("1");
				space.states[i].drive(vid);
				space.traces[vid][i] = value("1");

				prgm->insert_instr(best_up[j], i, new assignment(NULL, vars.get_name(vid) + "+", &vars, "", verbosity));
			}

			sort(best_down.begin(), best_down.end());
			unique(best_down.begin(), best_down.end());
			for (j = 0; j < (int)best_down.size(); j++)
			{
				i = space.duplicate_state(best_down[j]);
				space.states[i][vid] = value("0");
				space.states[i].drive(vid);
				space.traces[vid][i] = value("0");

				prgm->insert_instr(best_down[j], i, new assignment(NULL, vars.get_name(vid) + "-", &vars, "", verbosity));
			}
		}
	}

	if (verbosity & VERB_STATE_VAR_HSE)
	{
		prgm->print_hse("");
		cout << endl;
	}

	if (verbosity & VERB_STATEVAR_STATE)
		space.print_states(&vars);
	if (verbosity & VERB_STATEVAR_STATE_DOT)
		space.print_dot();
}

void program::generate_prs()
{
	for (int vi = 0; vi < space.width(); vi++)
		if (vars.get_name(vi).find_first_of("|&~") == string::npos && vars.get_name(vi).find("Reset") == string::npos)
			prs.push_back(rule(vi, &space, &vars, verbosity));

	if (verbosity & VERB_BASE_PRS)
		print_prs();
}

/* TODO: Factoring - production rules should be relatively short.
 * Look for common expressions between production rules and factor them
 * out into their own variable.
 */
void program::factor_prs()
{

	//Not as trivial as seemed on initial exploration.
	//Must find cost function for inserting state variable.
	//Must find benefit for inserting factor
	//Balance size of factored chunk vs how many rules factored from
	//Prioritize longer rules to reduce capacitive driving (i.e. not every 'factor removed' is equal)
	//MUST factor if over cap in series/parallel

	for(size_t prsi = 0; prsi < prs.size(); prsi++)
	{
		prs[prsi].find_var_usage_up();
		prs[prsi].find_var_usage_down();

	}

	//BAH TOO BRUTE FORCE TOO SLOW DELETE EVERYTHING HIDE THE SHAME KNOW THERE IS BETTER



	if (verbosity & VERB_FACTORED_PRS)
		print_prs();
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

