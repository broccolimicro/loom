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
	vector<int>::iterator i;
	vector<string>::iterator j;
	int uid = states.size();
	push_back(s);

	for (i = from.begin(), j = chp.begin(); i != from.end() && j != chp.end(); i++, j++)
	{
		if (CHP_EDGE)
			insert_edge(*i, uid, *j);
		else
			insert_edge(*i, uid, "Merge");
	}
}

void graph::insert(state s, int from, string chp)
{
	int to = states.size();
	push_back(s);

	if (from != -1)
	{
		if (CHP_EDGE)
			insert_edge(from, to, chp);
		else
			insert_edge(from, to, "");
	}
}

void graph::insert_edge(int from, int to, string chp)
{
	state from_state = states[from];
	state to_state = states[to];
	vector<value>::iterator i, j;
	string::iterator si, sj;
	string str;
	int k;

	// Delta State Insertion
	state result_state = diff(from_state,to_state);
	if(to_state.prs)
		result_state.tag = from;
	else
		result_state.tag = -1;

	if(to_state.prs || SHOW_ALL_DIFF_STATES)
		delta.states.push_back(result_state);

	// Up State Insertion
	if (from_state.size() > up.size())
		up.traces.resize(from_state.size(), trace());

	if (from_state.size() > (int)up_firing.size())
		up_firing.resize(from_state.size(), vector<int>());

	for (i = from_state.begin(), j = to_state.begin(), k = 0; i != from_state.end() && j != to_state.end(); i++, j++, k++)
	{
		str = "";
		for (si = i->begin(), sj = j->begin(); si != i->end() && sj != j->end(); si++, sj++)
		{
			if (*sj == '1' && (*si == '0' || (*si != '1' && from == 0)))
				str = str + "1";
			else if (*sj == '1' && *si == '1')
				str = str + "X";
			else
				str = str + "0";
		}

		up[k].push_back(value(str));
	}

	// Down State Insertion
	if (from_state.size() > down.size())
		down.traces.resize(from_state.size(), trace());

	if (from_state.size() > (int)down_firing.size())
		down_firing.resize(from_state.size(), vector<int>());

	for (i = from_state.begin(), j = to_state.begin(), k = 0; i != from_state.end() && j != to_state.end(); i++, j++, k++)
	{
		str = "";
		for (si = i->begin(), sj = j->begin(); si != i->end() && sj != j->end(); si++, sj++)
		{
			if (*sj == '0' && (*si == '1' || (*si != '0' && from == 0)))
				str = str + "1";
			else if (*sj == '0' && *si == '0')
				str = str + "X";
			else
				str = str + "0";
		}

		down[k].push_back(value(str));
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
	cout << "Up Production Rule Firing" << endl;
	cout << up << endl;
}

void graph::print_down()
{
	cout << "Down Production Rule Firing" << endl;
	cout << down << endl;
}

void graph::print_dot()
{
	size_t i, j;

	//Print space (for debugging purposes)
	cout << "\n\n\t.dot formatted graph:" << endl;
	cout << "digraph finite_state_machine {\n\tgraph [ label = \"\\n\\nState space graph!\" ];" << endl;
	if (!GRAPH_VERT)
		cout << "\trankdir=LR;" << endl;
	cout << "\tnode [shape = ellipse];" << endl;
	cout << "\tgraph [ dpi =" << GRAPH_DPI << " ];" << endl;
	for(i = 0; i < states.size(); i++)
	{
		if(i >= edges.size())
			edges.resize(i+1, vector<int>());

		// "Node 1" -> "Node 2" [ label = "trans" ];
		for (j = 0; j < edges[i].size(); j++)
		{
			cout << "\t\"" << i << ":" << states[i] << "\"" << " -> ";
			cout << "\"" << edges[i][j] << ":" << states[edges[i][j]];
			cout << "\" [ label = \"" << (transitions[i])[j] << "\" ];" << endl;
		}
	}
	cout << "}\n\n";
}

void graph::print_delta()
{
	size_t j;
	vector<state>::iterator i;
	vector<int>::iterator m;
	vector<string>::iterator q;

	cout << "Delta Space:" << endl;
	for (i = delta.begin(), j = 0; i != delta.end(); i++, j++)
		cout << *i << "\t" << i->tag << endl;
	cout << endl;
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
