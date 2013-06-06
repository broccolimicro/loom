/*
 * process.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../syntax.h"
#include "../data.h"

#include "process.h"
#include "record.h"
#include "channel.h"

process::process()
{
	name = "";
	_kind = "process";
	verbosity = 0;
	chp = "";
	is_inline = false;
}

process::process(string raw, map<string, keyword*> *types, int verbosity)
{
	_kind = "process";
	vars.types = types;
	this->verbosity = verbosity;
	this->chp = raw;
	is_inline = false;

	parse(raw);
}

process::~process()
{
	name = "";
	_kind = "process";
	verbosity = 0;
	chp = "";

	vars.clear();
}

process &process::operator=(process p)
{
	def = p.def;
	prs = p.prs;
	vars = p.vars;
	return *this;
}

void process::parse(string raw)
{
	if (raw.compare(0, 7, "inline ") == 0)
	{
		is_inline = true;
		raw = raw.substr(7);
		chp = raw;
	}

	size_t name_start = chp.find_first_of(" ")+1;
	size_t name_end = chp.find_first_of("(");
	size_t input_start = chp.find_first_of("(")+1;
	size_t input_end = chp.find_first_of(")");
	size_t sequential_start = chp.find_first_of("{")+1;
	size_t sequential_end = chp.length()-1;
	string io_sequential;
	string::iterator i, j;

	map<string, variable> temp;
	map<string, variable>::iterator vi, vj;
	map<string, keyword*>::iterator ti;

	name = chp.substr(name_start, name_end - name_start);
	io_sequential = chp.substr(input_start, input_end - input_start);

	if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
	{
		cout << "Process:\t" << chp << endl;
		cout << "\tName:\t" << name << endl;
		cout << "\tArgs:\t" << io_sequential << endl;
	}

	for (i = io_sequential.begin(), j = io_sequential.begin(); i != io_sequential.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_sequential.end())
		{
			expand_instantiation(NULL, io_sequential.substr(j-io_sequential.begin(), i+1 - j), &vars, &args, "\t", verbosity, false);
			j = i+2;
		}
	}

	string sequential = chp.substr(sequential_start, sequential_end - sequential_start);

	if (!is_inline)
	{
		expand_instantiation(NULL, "__sync call", &vars, &args, "\t", verbosity, false);
		sequential = "[call.r];call.a+;(" + sequential + ");[~call.r];call.a-";
	}

	def.init(sequential, &vars, "\t", verbosity);

	if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
	{
		cout << "\tVariables:" << endl;
		vars.print("\t\t");
		cout << "\tHSE:" << endl;
		def.print_hse("\t\t");
		cout << endl << endl;
	}
}

void process::merge()
{
	def.merge();

	if (verbosity & VERB_MERGED_HSE)
	{
		def.print_hse("");
		cout << endl;
	}
}

// TODO Projection algorithm - when do we need to do projection? when shouldn't we do projection?
void process::project()
{
	if (verbosity & VERB_PROJECTED_HSE)
	{
		def.print_hse("");
		cout << endl;
	}
}

// TODO Process decomposition - How big should we make processes?
void process::decompose()
{
	if (verbosity & VERB_DECOMPOSED_HSE)
	{
		def.print_hse("");
		cout << endl;
	}
}

// TODO Handshaking Reshuffling
void process::reshuffle()
{
	if (verbosity & VERB_RESHUFFLED_HSE)
	{
		def.print_hse("");
		cout << endl;
	}
}

// TODO There is a problem with the interaction of scribe variables with bubbleless reshuffling because scribe variables insert bubbles
void process::generate_states()
{
	cout << "Process" << endl;

	map<string, variable>::iterator i;
	for (i = vars.global.begin(); i != vars.global.end(); i++)
	{
		i->second.state0 = pid(space.new_place(map<int, int>(), NULL), PETRI_PLACE);
		space.M0.push_back(i->second.state0.index);
		i->second.state1 = pid(space.new_place(map<int, int>(), NULL), PETRI_PLACE);
	}

	pids start;
	start.push_back(space.insert_place(start, map<int, int>(), NULL));
	space.M0.push_back(start[0].index);
	//TODO: Okay, I am going to say it. I think it is weird that we are passing a list of instructions to parallel to state space.
	//It does not feel like a syntax structure's job to do that kind of thing.
	space.connect(def.generate_states(&space, start, map<int, int>(), minterm()), start);

	for (i = vars.global.begin(); i != vars.global.end(); i++)
		if (!i->second.driven && i->second.arg)
		{
			space.connect(space.insert_transition(i->second.state0, expression(i->first, &vars).expr, map<int, int>(), NULL), i->second.state1);
			space.connect(space.insert_transition(i->second.state1, expression("~" + i->first, &vars).expr, map<int, int>(), NULL), i->second.state0);
		}

	print_dot();
	space.Wp.print();
	space.Wn.print();
	/*if (verbosity & VERB_BASE_STATE_SPACE)
		space.print_states(&vars);
	if (verbosity & VERB_BASE_STATE_SPACE_DOT)
		space.print_dot();*/
}

void process::insert_state_vars()
{
	/*srand(time(0));

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

	list<pair<int, int> >::iterator ci;

	int cc0, cc1;

	// These are strictly for debugging purposes
	timeval t0, t1;
	int best_up_benefit;
	int best_down_benefit;
	int best_up_deficit;
	int best_down_deficit;

	list<pair<int, int> > old_up_conflict_firings;
	list<pair<int, int> > old_down_conflict_firings;
	list<pair<int, int> > actual_up_benefit;
	list<pair<int, int> > actual_down_benefit;
	list<pair<int, int> > actual_up_deficit;
	list<pair<int, int> > actual_down_deficit;

	/* THIS IS A STRAIGHTFORWARD BRUTE FORCE ALGORITHM. IT IS NOT SMART. IT IS NOT FAST. BUT IT WORKS.
	 *
	 * This algorithms execution time balloons very quickly and becomes very very very fucking slow, BUT! it does indeed calculate the optimal
	 * state variable insertion points and resulting trace. So... projection + process decomposition to keep the space we are looking at very small?
	 * or find a less optimal, faster way?
	 * /

	/* Turn this knob to control how many up transitions are allowed and how many down transitions are allowed. For a state space of size 25,
	 * 1 is instant, 2 takes exactly 1 minute, and 3 takes hours. This is the glory of a brute force method.
	 * /
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
		space.print_firings(&vars);* /

		space.gen_deltas();
		space.gen_conflicts();

		/* Step 1: cache the list of conflict firings already present in the state space. This does not include conflicts between two
		 * states in which neither states are a firing for a variable.
		 * /

		if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
		{
			old_up_conflict_firings.clear();
			old_down_conflict_firings.clear();
			old_up_conflict_firings = up_conflict_firings;
			old_down_conflict_firings = down_conflict_firings;
		}

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

		if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
		{
			if (m == 0)
				cout << "Remainder:      " << cc1 << endl;
			else
			{
				actual_up_benefit.clear();
				actual_down_benefit.clear();
				actual_up_deficit.clear();
				actual_down_deficit.clear();

				for (ci = old_up_conflict_firings.begin(); ci != old_up_conflict_firings.end(); ci++)
					if (find(up_conflict_firings.begin(), up_conflict_firings.end(), *ci) == up_conflict_firings.end())
						actual_up_benefit.push_back(*ci);

				for (ci = old_down_conflict_firings.begin(); ci != old_down_conflict_firings.end(); ci++)
					if (find(down_conflict_firings.begin(), down_conflict_firings.end(), *ci) == down_conflict_firings.end())
						actual_down_benefit.push_back(*ci);

				for (ci = up_conflict_firings.begin(); ci != up_conflict_firings.end(); ci++)
					if (find(old_up_conflict_firings.begin(), old_up_conflict_firings.end(), *ci) == old_up_conflict_firings.end())
						actual_up_deficit.push_back(*ci);

				for (ci = down_conflict_firings.begin(); ci != down_conflict_firings.end(); ci++)
					if (find(old_down_conflict_firings.begin(), old_down_conflict_firings.end(), *ci) == old_down_conflict_firings.end())
						actual_down_deficit.push_back(*ci);


				cout << "Old Up Conflicts:   ";
				for (ci = old_up_conflict_firings.begin(); ci != old_up_conflict_firings.end(); ci++)
					cout << "[" << ci->first << "," << ci->second << "]";
				cout << endl;

				cout << "Old Down Conflicts: ";
				for (ci = old_down_conflict_firings.begin(); ci != old_down_conflict_firings.end(); ci++)
					cout << "[" << ci->first << "," << ci->second << "]";
				cout << endl;

				cout << "Actual Up Benefit:   ";
				for (ci = actual_up_benefit.begin(); ci != actual_up_benefit.end(); ci++)
					cout << "[" << ci->first << "," << ci->second << "]";
				cout << endl;

				cout << "Actual Down Benefit: ";
				for (ci = actual_down_benefit.begin(); ci != actual_down_benefit.end(); ci++)
					cout << "[" << ci->first << "," << ci->second << "]";
				cout << endl;

				cout << "Actual Up Deficit:   ";
				for (ci = actual_up_deficit.begin(); ci != actual_up_deficit.end(); ci++)
					cout << "[" << ci->first << "," << ci->second << "]";
				cout << endl;

				cout << "Actual Down Deficit: ";
				for (ci = actual_down_deficit.begin(); ci != actual_down_deficit.end(); ci++)
					cout << "[" << ci->first << "," << ci->second << "]";
				cout << endl;

				cout << "Actual Delta:   " << cc0 - cc1 << " = " << actual_up_benefit.size() << " + " << actual_down_benefit.size() << " - " << actual_up_deficit.size() << " - " << actual_down_deficit.size() << endl;
				cout << "Guessed Delta:  " << best_benefit << " = " << best_up_benefit << " + " << best_down_benefit << " - " << best_up_deficit << " - " << best_down_deficit << endl;
				cout << "Remainder:      " << cc1 << endl;
				cout << "Iteration Time: " << ((double)(t1.tv_sec - t0.tv_sec) + 0.000001*(double)(t1.tv_usec - t0.tv_usec)) << " seconds" << endl;
				cout << endl << endl;
			}
		}

		if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
		{
			cout << "Up Conflicts:   ";
			for (ci = up_conflict_firings.begin(); ci != up_conflict_firings.end(); ci++)
				cout << "[" << ci->first << "," << ci->second << "]";
			cout << endl;
		}

		if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
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
		 * /
		best_benefit = -9999999;
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
						// Cases 2 and 4
						if (rup[ci->first])
						{
							// F => C
							if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
								up_benefit++;

							// DF => C
							if (values[ci->second].data[0] != '0')
								up_deficit++;

							// F => DC
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								up_deficit++;

							// DF => DC
							if (rup[ci->second])
								up_deficit++;
						}
						else if (rdown[ci->first])
						{
							// F => C
							if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
								up_benefit++;

							// DF => C
							if (values[ci->second].data[0] != '1')
								up_deficit++;

							// F => DC
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								up_deficit++;

							// DF => DC
							if (rdown[ci->second])
								up_deficit++;
						}
						// Cases 1 and 3
						else
						{
							// F => C
							if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
								up_benefit++;

							// F => DC
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								up_deficit++;
						}
					}

					// Add the number of down production rule conflicts eliminated in this trace to the benefit
					for (ci = down_conflict_firings.begin(); ci != down_conflict_firings.end(); ci++)
					{
						// Cases 2 and 4
						if (rup[ci->first])
						{
							// F => C
							if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
								down_benefit++;

							// DF => C
							if (values[ci->second].data[0] != '0')
								down_deficit++;

							// F => DC
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								down_deficit++;

							// DF => DC
							if (rup[ci->second])
								down_deficit++;
						}
						else if (rdown[ci->first])
						{
							// F => C
							if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
								down_benefit++;

							// DF => C
							if (values[ci->second].data[0] != '1')
								down_deficit++;

							// F => DC
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								down_deficit++;

							// DF => DC
							if (rdown[ci->second])
								down_deficit++;
						}
						// Cases 1 and 3
						else
						{
							// F => C
							if ((values[ci->first].data[0] == '0' && values[ci->second].data[0] == '1') ||
								(values[ci->first].data[0] == '1' && values[ci->second].data[0] == '0'))
								down_benefit++;

							// F => DC
							if ((values[ci->first].data[0] != '1' && rdown[ci->second]) ||
								(values[ci->first].data[0] != '0' && rup[ci->second]))
								down_deficit++;
						}
					}

					// Subtract the number of up production rule conflicts added by adding this variable with these up firings from the benefit
					for (k = 0; k < (int)up.size(); k++)
					{
						for (l = 0; l < (int)space.up_conflicts[up[k]].size(); l++)
						{
							if (values[space.up_conflicts[up[k]][l]].data[0] != '1')
								up_deficit++;
							//if (rdown[space.up_conflicts[up[k]][l]])
							//	up_deficit++;
						}

						//if (values[up[k]].data[0] != '0')
						//	up_deficit++;
					}

					// Subtract the number of down production rule conflicts added by adding this variable with these down firings from the benefit
					for (k = 0; k < (int)down.size(); k++)
					{
						for (l = 0; l < (int)space.down_conflicts[down[k]].size(); l++)
						{
							if (values[space.down_conflicts[down[k]][l]].data[0] != '0')
								down_deficit++;
							//if (rup[space.down_conflicts[down[k]][l]])
							//	down_deficit++;
						}

						//if (values[down[k]].data[0] != '1')
						//	down_deficit++;
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
						best_up_benefit = up_benefit;
						best_down_benefit = down_benefit;
						best_up_deficit = up_deficit;
						best_down_deficit = down_deficit;
						best_values = values;
					}
				}
				if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
					cout << benefit << "\t";
			}
			if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
				cout << endl;
		}
		if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
			cout << endl;

		if (best_benefit != 0)
		{
			// Insert new variable
			vid = vars.insert(variable(vars.unique_name("_sv"), "node", 1, false));

			if ((verbosity & VERB_BASE_HSE) && (verbosity & VERB_DEBUG))
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

				def.insert_instr(best_up[j], i, new assignment(NULL, vars.get_name(vid) + "+", &vars, "", verbosity));
			}

			sort(best_down.begin(), best_down.end());
			unique(best_down.begin(), best_down.end());
			for (j = 0; j < (int)best_down.size(); j++)
			{
				i = space.duplicate_state(best_down[j]);
				space.states[i][vid] = value("0");
				space.states[i].drive(vid);
				space.traces[vid][i] = value("0");

				def.insert_instr(best_down[j], i, new assignment(NULL, vars.get_name(vid) + "-", &vars, "", verbosity));
			}
		}
	}

	if (verbosity & VERB_STATE_VAR_HSE)
	{
		def.print_hse("");
		cout << endl;
	}

	if (verbosity & VERB_STATEVAR_STATE)
		space.print_states(&vars);
	if (verbosity & VERB_STATEVAR_STATE_DOT)
		space.print_dot();*/
}

void process::generate_prs()
{
	for (int vi = 0; vi < vars.size(); vi++)
		if (vars.get_name(vi).find_first_of("|&~") == string::npos)
			prs.push_back(rule(vi, &space, &vars, verbosity));

	if (verbosity & VERB_BASE_PRS)
		print_prs();
}

/* TODO: Factoring - production rules should be relatively short.
 * Look for common expressions between production rules and factor them
 * out into their own variable.
 */
void process::factor_prs()
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

void process::print_hse(ostream *fout)
{
	def.print_hse("", fout);
}

void process::print_dot(ostream *fout)
{
	int i, j;
	string label;
	(*fout) << "digraph " << name << endl;
	(*fout) << "{" << endl;

	for (i = 0; i < (int)space.S.size(); i++)
		if (!space.dead(pid(i, PETRI_PLACE)))
			(*fout) << "\tS" << i << ";" << endl;

	for (i = 0; i < (int)space.T.size(); i++)
	{
		label = space.values.expr(space.T[i].delta, vars.get_names());
		if (label != "")
			(*fout) << "\tT" << i << " [shape=box] [label=\"" << label << "\"];" << endl;
		else
			(*fout) << "\tT" << i << " [shape=box];" << endl;
	}

	for (i = 0; i < (int)space.Wp.size(); i++)
		for (j = 0; j < (int)space.Wp[i].size(); j++)
			if (space.Wp[i][j] > 0)
				(*fout) << "\tT" << j << " -> " << "S" << i << ";" <<  endl;

	for (i = 0; i < (int)space.Wn.size(); i++)
		for (j = 0; j < (int)space.Wn[i].size(); j++)
			if (space.Wn[i][j] > 0)
				(*fout) << "\tS" << i << " -> " << "T" << j << ";" <<  endl;

	(*fout) << "}" << endl;
}

void process::print_petrify()
{
	int i, j;
	vector<string> labels;
	map<string, int> labelmap;
	map<string, int>::iterator li;
	string label;
	FILE *file;
	map<string, variable>::iterator vi;
	bool first;

	file = fopen((name + ".g").c_str(), "wb");

	fprintf(file, ".model %s\n", name.c_str());

	first = true;
	for (vi = vars.global.begin(); vi != vars.global.end(); vi++)
		if (vi->second.arg && !vi->second.driven)
		{
			if (first)
			{
				fprintf(file, ".inputs");
				first = false;
			}
			fprintf(file, " %s", vi->second.name.c_str());
		}
	if (!first)
		fprintf(file, "\n");

	first = true;
	for (vi = vars.global.begin(); vi != vars.global.end(); vi++)
		if (vi->second.arg && vi->second.driven)
		{
			if (first)
			{
				fprintf(file, ".outputs");
				first = false;
			}
			fprintf(file, " %s", vi->second.name.c_str());
		}
	if (!first)
		fprintf(file, "\n");

	first = true;
	for (vi = vars.global.begin(); vi != vars.global.end(); vi++)
		if (!vi->second.arg)
		{
			if (first)
			{
				fprintf(file, ".internal");
				first = false;
			}
			fprintf(file, " %s", vi->second.name.c_str());
		}
	if (!first)
		fprintf(file, "\n");

	first = true;
	for (i = 0; i < (int)space.T.size(); i++)
	{
		label = space.values.expr(space.T[i].delta, vars.get_names());
		if (label == "")
		{
			if (first)
			{
				fprintf(file, ".dummy");
				first = false;
			}
			label = string("T") + to_string(i);
			fprintf(file, " %s", label.c_str());
		}
		li = labelmap.find(label);
		if (li == labelmap.end())
			labelmap.insert(pair<string, int>(label, 1));
		else
		{
			label += string("/") + to_string(li->second);
			li->second++;
		}

		labels.push_back(label);
	}
	if (!first)
		fprintf(file, "\n");

	string from;
	vector<string> to;

	fprintf(file, ".graph\n");
	for (i = 0; i < (int)space.Wp.size(); i++)
		for (j = 0; j < (int)space.Wp[i].size(); j++)
			if (space.Wp[i][j] > 0)
				fprintf(file, "%s S%d\n", labels[j].c_str(), i);

	for (i = 0; i < (int)space.Wn.size(); i++)
		for (j = 0; j < (int)space.Wn[i].size(); j++)
			if (space.Wn[i][j] > 0)
				fprintf(file, "S%d %s\n", i, labels[j].c_str());

	first = true;
	fprintf(file, ".marking {");
	for (i = 0; i < (int)space.M0.size(); i++)
	{
		if (!space.dead(pid(space.M0[i], PETRI_PLACE)))
		{
			if (first)
				first = false;
			else
				fprintf(file, " ");
			fprintf(file, "S%d", space.M0[i]);
		}
	}
	fprintf(file, "}\n");
	fprintf(file, ".end\n");

	fclose(file);
}

void process::print_prs()
{
	cout << "Production Rules: " << endl;

	for (size_t i = 0; i < prs.size(); i++)
		cout << prs[i];
}
