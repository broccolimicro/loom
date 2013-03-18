#include "graph.h"

graph::graph()
{

}

graph::~graph()
{
	edges.clear();
	transitions.clear();
}

void graph::append_state(state s, vector<int> from, vector<string> chp)
{
	vector<int>::iterator a;
	vector<string>::iterator b;
	int to = states.size();

	if (to == 0)
	{
		s[0] = value("1");
		s[1] = value("0");
	}
	else
	{
		s[0] = value("0");
		s[1] = value("1");
	}

	states.push_back(s);
	for (a = from.begin(), b = chp.begin(); a != from.end() && b != chp.end(); a++, b++)
		insert_edge(*a, to, *b);
}

void graph::append_state(state s, int from, string chp)
{
	int to = states.size();

	if (to == 0)
	{
		s[0] = value("1");
		s[1] = value("0");
	}
	else
	{
		s[0] = value("0");
		s[1] = value("1");
	}

	states.push_back(s);

	if (from != -1)
		insert_edge(from, to, chp);
}

void graph::insert_state(state s, int from, int to)
{
	int id = states.size();
	states.push_back(s);
	for (int i = 0; i < traces.size() && i < s.size(); i++)
		traces[i].push_back(s[i]);

	replace(edges[from].begin(), edges[from].end(), to, id);
	edges.push_back(vector<int>());
	edges[id].push_back(to);
}

void graph::insert_edge(int from, int to, string chp)
{
	if (from >= (int)edges.size())
		edges.resize(from+1, vector<int>());
	edges[from].push_back(to);

	if (from >= (int)transitions.size())
		transitions.resize(from+1, vector<string>());
	transitions[from].push_back(chp);
}

path_space graph::get_paths(int from, int to, path p)
{
	if (p.from == -1 && p.to == -1)
	{
		p.from = from;
		p.to = to;
	}

	path_space result(size()), temp(size());

	if (p[from] > 0)
	{
		p.clear();
		return result;
	}

	if (from == to)
	{
		result.push_back(p);
		return result;
	}

	p.set(from);

	for (size_t i = 0; i < edges[from].size(); i++)
		result.merge(get_paths(edges[from][i], to, p));

	return result;
}

trace graph::get_trace(int from, int up, int down, trace t, value p)
{
	if (from == up)
		p = value("1");
	if (from == down)
		p = value("0");

	if (t[from].data == (t[from] || p).data)
		return t;

	t[from] = t[from] || p;

	if (edges[from].size() > 0)
	{
		trace result(value("_"), t.size());
		for (size_t i = 0; i < edges[from].size(); i++)
			result = result || get_trace(edges[from][i], up, down, t, p);

		return result;
	}
	else
		return t;
}

void graph::gen_conflicts()
{
	up_conflicts.clear();
	down_conflicts.clear();

	up_conflicts.resize(size());
	down_conflicts.resize(size());
	int j, k;

	// Compare every state in both directions
	for (j = 0; j < size(); j++)
	{
		for (k = 0; k < size(); k++)
		{
			if (k != j)
			{
				if (BUBBLELESS)
				{
					if (up_conflict(states[j], states[k]))
						up_conflicts[j].push_back(k);
					if (down_conflict(states[j], states[k]))
						down_conflicts[j].push_back(k);
				}
				else if (conflict(states[j], states[k]))
				{
					up_conflicts[j].push_back(k);
					down_conflicts[j].push_back(k);
				}
			}
		}
	}
}

void graph::gen_traces()
{
	traces.traces.clear();
	for (size_t i = 0; i < states.width(); i++)
		traces.push_back(states(i));
}

void graph::gen_deltas()
{
	size_t from, to, x;

	up.traces.clear();
	down.traces.clear();
	up_firings.clear();
	up_firings_union.clear();
	down_firings.clear();
	down_firings_union.clear();
	delta.traces.clear();

	// Delta Calculations (Up, Down, Delta)
	up.traces.resize(states.width(), trace());
	up_firings.resize(states.width(), vector<int>());
	up_firings_union.resize(states.size(), 0);
	down.traces.resize(states.width(), trace());
	down_firings.resize(states.width(), vector<int>());
	down_firings_union.resize(states.size(), 0);
	delta.traces.resize(states.width(), trace());

	for (from = 0; from < edges.size(); from++)
		for (x = 0; x < edges[from].size(); x++)
		{
			vector<value>::iterator i, j;
			string::iterator si, sj;
			string upstr, downstr;
			int k;

			state from_state;
			state to_state;

			to = edges[from][x];
			from_state = states[from];
			to_state = states[to];

			for (i = from_state.begin(), j = to_state.begin(), k = 0; i != from_state.end() && j != to_state.end(); i++, j++, k++)
			{
				upstr = "";
				downstr = "";
				for (si = i->begin(), sj = j->begin(); si != i->end() && sj != j->end(); si++, sj++)
				{
					if (*sj == '1' && *si != '1' && to_state.prs)
					{
						upstr = upstr + "1";
						up_firings[k].push_back(from);
						up_firings_union[from]++;
					}
					else if (*sj == '1' && *si == '1')
						upstr = upstr + "X";
					else
						upstr = upstr + "0";

					if (*sj == '0' && *si != '0' && to_state.prs)
					{
						downstr = downstr + "1";
						down_firings[k].push_back(from);
						down_firings_union[from]++;
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
	size_t i, j;

	cout << "Up Production Rule Space" << endl;
	cout << up << endl;

	cout << "Up Production Rule Firings" << endl;
	for (i = 0; i < up_firings.size(); i++)
	{
		for (j = 0; j < up_firings[i].size(); j++)
			cout << up_firings[i][j] << " ";
		cout << endl;
	}

	cout << "Up Production Rule Conflicts" << endl;
	for (i = 0; i < up_conflicts.size(); i++)
	{
		cout << i << ": ";
		for (j = 0; j != up_conflicts[i].size(); j++)
			cout << up_conflicts[i][j] << " ";
		cout << endl;
	}
}

void graph::print_down()
{
	size_t i, j;

	cout << "Down Production Rule Space" << endl;
	cout << down << endl;

	cout << "Down Production Rule Firings" << endl;
	for (i = 0; i < down_firings.size(); i++)
	{
		for (j = 0; j < down_firings[i].size(); j++)
			cout << down_firings[i][j] << " ";
		cout << endl;
	}

	cout << "Down Production Rule Conflicts" << endl;
	for (i = 0; i < down_conflicts.size(); i++)
	{
		cout << i << ": ";
		for (j = 0; j != down_conflicts[i].size(); j++)
			cout << down_conflicts[i][j] << " ";
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
