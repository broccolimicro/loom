#include "graph.h"

graph::graph()
{

}

graph::~graph()
{
	edges.clear();
	transitions.clear();
}

void graph::insert(state s, vector<int> from, vector<string> chp)
{
	vector<int>::iterator a;
	vector<string>::iterator b;
	int to = states.size();

	push_back(s);
	for (a = from.begin(), b = chp.begin(); a != from.end() && b != chp.end(); a++, b++)
		insert_edge(*a, to, *b);
}

void graph::insert(state s, int from, string chp)
{
	int to = states.size();
	push_back(s);

	if (from != -1)
		insert_edge(from, to, chp);
}

void graph::insert_edge(int from, int to, string chp)
{
	state from_state = states[from];
	state to_state = states[to];

	vector<value>::iterator i, j;
	string::iterator si, sj;
	string upstr, downstr;
	int k;

	// Delta Calculations (Up, Down, Delta)
	if (to_state.size() > up.size())
		up.traces.resize(to_state.size(), trace());
	if (to_state.size() > (int)up_firings.size())
		up_firings.resize(to_state.size(), vector<int>());
	if (to_state.size() > down.size())
		down.traces.resize(to_state.size(), trace());
	if (to_state.size() > (int)down_firings.size())
		down_firings.resize(to_state.size(), vector<int>());
	if (to_state.size() > delta.size())
		delta.traces.resize(to_state.size(), trace());

	for (i = states[from].begin(), j = to_state.begin(), k = 0; i != states[from].end() && j != to_state.end(); i++, j++, k++)
	{
		upstr = "";
		downstr = "";
		for (si = i->begin(), sj = j->begin(); si != i->end() && sj != j->end(); si++, sj++)
		{
			if (*sj == '1' && *si != '1' && to_state.prs)
			{
				upstr = upstr + "1";
				up_firings[k].push_back(from);
			}
			else if (*sj == '1' && *si == '1')
				upstr = upstr + "X";
			else
				upstr = upstr + "0";

			if (*sj == '0' && *si != '0' && to_state.prs)
			{
				downstr = downstr + "1";
				down_firings[k].push_back(from);
			}
			else if (*sj == '0' && *si == '0')
				downstr = downstr + "X";
			else
				downstr = downstr + "0";
		}

		delta[k].push_back(to_state[k]);
		up[k].push_back(value(upstr));
		down[k].push_back(value(downstr));
	}

	// Edge Insertion
	if (from >= (int)edges.size())
		edges.resize(from+1, vector<int>());
	edges[from].push_back(to);

	if (from >= (int)transitions.size())
		transitions.resize(from+1, vector<string>());
	transitions[from].push_back(chp);
}

void graph::push_back(state s)
{
	states.push_back(s);

	vector<trace>::iterator i;
	vector<value>::iterator v;
	trace t;

	for (i = traces.begin(), v = s.begin(); i != traces.end() && v != s.end(); i++, v++)
		i->values.push_back(*v);

	for (; i != traces.end(); i++)
		i->push_back(value("X"));

	for (; v != s.end(); v++)
	{
		t.assign(states.size()-1, *v);
		traces.push_back(t);
	}

	if (edges.size() < states.size())
		edges.resize(states.size(), vector<int>());

	if (transitions.size() < states.size())
		transitions.resize(states.size(), vector<string>());
}

void graph::push_back(trace t)
{
	traces.push_back(t);

	vector<state>::iterator i;
	vector<value>::iterator v;
	state s;

	for (i = states.begin(), v = t.begin(); i != states.end() && v != t.end(); i++, v++)
		i->values.push_back(*v);

	for (; i != states.end(); i++)
		i->values.push_back(value("X"));

	for (; v != t.end(); v++)
	{
		s.assign(traces.size()-1, *v);
		states.push_back(s);
	}

	if (edges.size() < states.size())
		edges.resize(states.size(), vector<int>());

	if (transitions.size() < states.size())
		transitions.resize(states.size(), vector<string>());
}

void graph::close()
{
	int i, j, k;
	map<int, vector<int> >::iterator ci;

	for (i = 0; i < width(); i++)
	{
		for (j = 0; j < (int)up_firings[i].size(); j++)
		{
			ci = up_conflicts.find(up_firings[i][j]);
			if (ci == up_conflicts.end())
				ci = up_conflicts.insert(pair<int, vector<int> >(up_firings[i][j], vector<int>())).first;

			for (k = 0; k < size(); k++)
			{
				if (k != up_firings[i][j] && up[i][k].data == "0" && find(ci->second.begin(), ci->second.end(), k) == ci->second.end())
				{
					if (BUBBLELESS)
					{
						if (up_conflict(states[up_firings[i][j]], states[k]))
							ci->second.push_back(k);
					}
					else
						if (conflict(states[up_firings[i][j]], states[k]))
							ci->second.push_back(k);
				}
			}
		}

		for (j = 0; j < (int)down_firings[i].size(); j++)
		{
			ci = down_conflicts.find(down_firings[i][j]);
			if (ci == down_conflicts.end())
				ci = down_conflicts.insert(pair<int, vector<int> >(down_firings[i][j], vector<int>())).first;

			for (k = 0; k < size(); k++)
			{
				if (k != down_firings[i][j] && down[i][k].data == "0" && find(ci->second.begin(), ci->second.end(), k) == ci->second.end())
				{
					if (BUBBLELESS)
					{
						if (down_conflict(states[down_firings[i][j]], states[k]))
							ci->second.push_back(k);
					}
					else
						if (conflict(states[down_firings[i][j]], states[k]))
							ci->second.push_back(k);
				}
			}
		}
	}
}

int graph::size()
{
	return states.size();
}

int graph::width()
{
	return traces.size();
}

void graph::print_up()
{
	vector<vector<int> >::iterator i;
	vector<int>::iterator j;
	map<int, vector<int> >::iterator k;

	cout << "Up Production Rule Space" << endl;
	cout << up << endl;

	cout << "Up Production Rule Firings" << endl;
	for (i = up_firings.begin(); i != up_firings.end(); i++)
	{
		for (j = i->begin(); j != i->end(); j++)
			cout << *j << " ";
		cout << endl;
	}

	cout << "Up Production Rule Conflicts" << endl;
	for (k = up_conflicts.begin(); k != up_conflicts.end(); k++)
	{
		cout << k->first << ": ";
		for (j = k->second.begin(); j != k->second.end(); j++)
			cout << *j << " ";
		cout << endl;
	}
}

void graph::print_down()
{
	vector<vector<int> >::iterator i;
	vector<int>::iterator j;
	vector<int>::iterator l;
	map<int, vector<int> >::iterator k;

	cout << "Down Production Rule Space" << endl;
	cout << down << endl;

	cout << "Down Production Rule Firings" << endl;
	for (i = down_firings.begin(); i != down_firings.end(); i++)
	{
		for (j = i->begin(); j != i->end(); j++)
			cout << *j << " ";
		cout << endl;
	}

	cout << "Down Production Rule Conflicts" << endl;
	for (k = down_conflicts.begin(); k != down_conflicts.end(); k++)
	{
		cout << k->first << ": ";
		for (l = k->second.begin(); l != k->second.end(); l++)
			cout << *l << " ";
		cout << endl;
	}
}

void graph::print_dot()
{
	size_t i, j;
	ofstream outputGraph;
	outputGraph.open("graph.dot");
	//Print space (for debugging purposes)
	//outputGraph << "\n\n\t.dot formatted graph:" << endl;
	outputGraph << "digraph finite_state_machine {\n\tgraph [ label = \"\\n\\nState space graph!\" ];" << endl;
	if (!GRAPH_VERT)
		cout << "\trankdir=LR;" << endl;
	outputGraph << "\tnode [shape = ellipse];" << endl;
	outputGraph << "\tgraph [ dpi =" << GRAPH_DPI << " ];" << endl;
	for(i = 0; i < states.size(); i++)
	{
		if(i >= edges.size())
			edges.resize(i+1, vector<int>());

		// "Node 1" -> "Node 2" [ label = "trans" ];
		for (j = 0; j < edges[i].size(); j++)
		{
			outputGraph << "\t\"" << i << ":";
			if(STATE_LONG_NAME)
				outputGraph << states[i];
			outputGraph << "\"" << " -> ";
			outputGraph << "\"" << edges[i][j] << ":";
			if(STATE_LONG_NAME)
				outputGraph << states[edges[i][j]];
			outputGraph << "\" [ label = \"" << (transitions[i])[j] << "\" ];" << endl;
		}
	}
	outputGraph << "}\n\n";
	outputGraph.close();
}

void graph::print_delta()
{
	vector<vector<int> >::iterator i;
	vector<int>::iterator j;
	map<int, vector<int> >::iterator k;

	cout << "Delta Space" << endl;
	cout << delta << endl;

	/*size_t j;
	vector<state>::iterator i;
	vector<int>::iterator m;
	vector<string>::iterator q;

	cout << "Delta Space:" << endl;
	for (i = delta.begin(), j = 0; i != delta.end(); i++, j++)
		cout << *i << "\t" << i->tag << endl;
	cout << endl;*/
}

ostream &operator<<(ostream &os, graph g)
{
	size_t j;
	vector<state>::iterator i;
	vector<int>::iterator m;
	vector<string>::iterator q;

	os << "State Space:" << endl;
	for (i = g.states.begin(), j = 0; i != g.states.end(); i++, j++)
	{
		os << *i << "\t" << j << " -> { ";

		if (j < g.edges.size())
			for (m = g.edges[j].begin(); m != g.edges[j].end(); m++)
				os << *m << " ";

		os << "}\t";
		if (j < g.transitions.size())
			for (q = g.transitions[j].begin(); q != g.transitions[j].end(); q++)
				os << *q << " ";

		os << endl;
	}

    return os;
}

ostream &operator>>(ostream &os, graph g)
{
	os << "Trace Space:" << endl;
	os << g.traces;

	return os;
}
