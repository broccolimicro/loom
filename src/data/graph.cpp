#include "graph.h"

graph::graph()
{

}

graph::~graph()
{
	front_edges.clear();
	back_edges.clear();
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

int graph::insert_state(state s, int from)
{
	int i;
	int id = states.size();
	states.push_back(s);
	for (i = 0; i < traces.size() && i < s.size(); i++)
		traces[i].push_back(s[i]);

	int delid = 0;
	for (i = 0; i < from; i++)
		delid += front_edges[i].size();

	//vector<value>::iterator x;
	//up.traces.front().values.erase(x);

	front_edges.resize(id+1);
	front_edges[id].assign(front_edges[from].begin(), front_edges[from].end());
	front_edges[from].clear();
	front_edges[from].push_back(id);

	transitions.resize(id+1);
	transitions[id].assign(transitions[from].begin(), transitions[from].end());
	transitions[from].clear();
	transitions[from].push_back("Assign");

	return id;
}

int graph::duplicate_state(int from)
{
	return insert_state(states[from], from);
}

void graph::insert_edge(int from, int to, string chp)
{
	if (from >= (int)front_edges.size())
		front_edges.resize(from+1, vector<int>());
	front_edges[from].push_back(to);
	if (to >= (int)back_edges.size())
		back_edges.resize(to+1, vector<int>());
	back_edges[to].push_back(from);

	if (from >= (int)transitions.size())
		transitions.resize(from+1, vector<string>());
	transitions[from].push_back(chp);
}

path_space graph::get_paths(int from, int to, path p)
{
	path_space result(size()), temp(size());

	if (p.from == -1 && p.to == -1)
	{
		p.from = from;
		p.to = to;
	}

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

	while (front_edges[from].size() == 1)
	{
		from = front_edges[from].front();

		if (p.from == -1 && p.to == -1)
		{
			p.from = from;
			p.to = to;
		}

		if (p[from] > 0)
			return result;

		if (from == to)
		{
			result.push_back(p);
			return result;
		}

		p.set(from);
	}

	for (size_t i = 0; i < front_edges[from].size(); i++)
		result.merge(get_paths(front_edges[from][i], to, p));

	return result;
}

value graph::get_next(int from, int to, int up, int down, value def)
{
	if (from == up && from == down)
		return value("_");
	else if (from == up)
		return value("1");
	else if (from == down)
		return value("0");
	else
		return def;
}

trace graph::get_trace(int up, int down)
{
	vector<int> coverage(size());
	vector<int> lastfork;
	vector<vector<int> > branches(size());
	int i, j;
	trace result;

	coverage.assign(size(), 0);

	result.values.assign(states.size(), value("_"));
	result.values[0] = value("X");

	for (i = 0; i < size(); i++)
		for (j = 0; j < (int)front_edges[i].size(); j++)
			result[front_edges[i][j]] = result[front_edges[i][j]] || get_next(i, front_edges[i][j], up, down, result[i]);

	for (i = 0; i < size(); i++)
		for (j = 0; j < (int)front_edges[i].size(); j++)
			result[front_edges[i][j]] = result[front_edges[i][j]] || get_next(i, front_edges[i][j], up, down, result[i]);
	return result;
}

trace graph::get_trace(vector<int> up, vector<int> down)
{
	size_t i, j, l;
	vector<int> to;
	vector<int> loop;
	vector<int>::iterator upi, downi;
	trace result(value("X"), states.size());
	to.assign(states.size(), 0);
	loop.assign(states.size(), 0);

	result.values[0] = value("X");

	to[0] = 1;
	for (i = 0; i < front_edges.size(); i++)
	{
		if (i != l+1)
			i = l;

		l = i;
		for (j = 0; j < front_edges[i].size(); j++)
		{
			to[front_edges[i][j]]++;
			upi = find(up.begin(), up.end(), i);
			downi = find(down.begin(), down.end(), i);
			if (upi != up.end() && downi != down.end())
				result[front_edges[i][j]] = value("X");
			if (upi != up.end())
				result[front_edges[i][j]] = value("1");
			else if (downi != down.end())
				result[front_edges[i][j]] = value("0");
			else if (result[i].data != "X" && to[front_edges[i][j]] == 1)
				result[front_edges[i][j]] = result[i].data;
			else if (result[i].data == result[front_edges[i][j]].data && to[front_edges[i][j]] > 1)
				result[front_edges[i][j]] = result[i].data;
			else
				result[front_edges[i][j]] = value("X");

			if ((size_t)front_edges[i][j] < l && loop[i] <= 1)
				l = front_edges[i][j];
		}

		loop[i]++;
	}

	return result;
}

void graph::set_trace(int uid, trace t)
{
	traces.assign(uid, t);

	for (size_t i = 0; i < states.size(); i++)
		states[i].assign(uid, t[i], value("X"));
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
	size_t i;
	traces.traces.clear();
	for (i = 0; i < states.width(); i++)
		traces.push_back(states(i));
}

void graph::gen_deltas()
{
	size_t from, to, x;

	up.traces.clear();
	down.traces.clear();
	up_firings.clear();
	up_firings_transpose.clear();
	down_firings.clear();
	down_firings_transpose.clear();
	delta.traces.clear();

	// Delta Calculations (Up, Down, Delta)
	up.traces.resize(states.width(), trace());
	up_firings.resize(states.width(), vector<int>());
	up_firings_transpose.resize(states.size(), vector<int>());
	down.traces.resize(states.width(), trace());
	down_firings.resize(states.width(), vector<int>());
	down_firings_transpose.resize(states.size(), vector<int>());
	delta.traces.resize(states.width(), trace());

	for (from = 0; from < front_edges.size(); from++)
		for (x = 0; x < front_edges[from].size(); x++)
		{
			vector<value>::iterator i, j;
			string::iterator si, sj;
			string upstr, downstr;
			int k;

			state from_state;
			state to_state;

			to = front_edges[from][x];
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
						up_firings_transpose[from].push_back(k);
					}
					else if (*sj == '1' && *si == '1')
						upstr = upstr + "X";
					else
						upstr = upstr + "0";

					if (*sj == '0' && *si != '0' && to_state.prs)
					{
						downstr = downstr + "1";
						down_firings[k].push_back(from);
						down_firings_transpose[from].push_back(k);
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
		if(i >= front_edges.size())
			front_edges.resize(i+1, vector<int>());

		// "Node 1" -> "Node 2" [ label = "trans" ];
		for (j = 0; j < front_edges[i].size(); j++)
		{
			outputGraph << "\t\"" << i << ":";
			if(STATE_LONG_NAME)
				outputGraph << states[i];
			outputGraph << "\"" << " -> ";
			outputGraph << "\"" << front_edges[i][j] << ":";
			if(STATE_LONG_NAME)
				outputGraph << states[front_edges[i][j]];
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

		if (j < g.front_edges.size())
			for (m = g.front_edges[j].begin(); m != g.front_edges[j].end(); m++)
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
