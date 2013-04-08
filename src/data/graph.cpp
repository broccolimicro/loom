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

/*
 * TODO: Write appropriate description
 */
int graph::append_state(state s, vector<int> from, vector<string> chp)
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

	return to;
}

/*
 * TODO: Write appropriate description
 */
int graph::append_state(state s, int from, string chp)
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

	return to;
}

/* insert_state takes in a state, s, and a uid, from. It then
 * appends s to the end of the state space and inserts appropriate edges.
 * insert_state returns the UID of the new state.
 */
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

/* duplicate_state takes the UID of a state. It then inserts a state
 * directly after the given UID's corrosponding state identical to the
 * state indexed. The function returns the UID of the duplicated state.
 */
int graph::duplicate_state(int from)
{
	return insert_state(states[from], from);
}

/* insert_edge is responsible for updating the front_edges and back_edges
 * datastructures. Given a from/to pair, insert_edge will add the appropriate
 * edge to each. It also takes a string 'chp' which is simply a message attached
 * to each edge, stored in'transitions'.
 */
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

/*
 * TODO: Write appropriate description
 */
void graph::get_trace(int from, int up, int down, trace *result)
{
	value next;
	value temp;

	while (front_edges[from].size() == 1)
	{
		if (from == up && from == down)
			next = (*result)[front_edges[from].front()] || value("_");
		else if (from == up)
			next = (*result)[front_edges[from].front()] || value("1");
		else if (from == down)
			next = (*result)[front_edges[from].front()] || value("0");
		else
			next = (*result)[front_edges[from].front()] || (*result)[from];

		if (next.data == (*result)[front_edges[from].front()].data)
			return;

		(*result)[front_edges[from].front()] = next;

		from = front_edges[from].front();
	}

	if (from == up && from == down)
		temp = value("_");
	else if (from == up)
		temp = value("1");
	else if (from == down)
		temp = value("0");
	else
		temp = (*result)[from];

	for (int i = 0; i < (int)front_edges[from].size(); i++)
	{
		next = (*result)[front_edges[from][i]] || temp;

		if (next.data != (*result)[front_edges[from][i]].data)
		{
			(*result)[front_edges[from][i]] = next;
			get_trace(front_edges[from][i], up, down, result);
		}
	}
}

/*
 * TODO: Write appropriate description
 */
void graph::get_trace(int from, vector<bool> *up, vector<bool> *down, trace *result)
{
	value next;
	value temp;
	int curr;

	while (front_edges[from].size() == 1)
	{
		curr = front_edges[from].front();
		if ((*up)[from])
			next = (*result)[curr] || value("1");
		else if ((*down)[from])
			next = (*result)[curr] || value("0");
		else
			next = (*result)[curr] || (*result)[from];

		if (next.data == (*result)[curr].data)
			return;

		(*result)[curr] = next;
		from = curr;
	}

	if ((*up)[from])
		temp = value("1");
	else if ((*down)[from])
		temp = value("0");
	else
		temp = (*result)[from];

	for (int i = 0; i < (int)front_edges[from].size(); i++)
	{
		curr = front_edges[from][i];
		next = (*result)[curr] || temp;

		if (next.data != (*result)[curr].data)
		{
			(*result)[curr] = next;
			get_trace(curr, up, down, result);
		}
	}
}

/* set_trace takes in a UID of the trace to be affected, as well as the new trace.
 * set_trace then updates graph's tracespace with the appropriate value, and
 * updates graph's state space to reflect this change.
 */
void graph::set_trace(int uid, trace t)
{
	traces.assign(uid, t);

	for (size_t i = 0; i < states.size(); i++)
		states[i].assign(uid, t[i], value("X"));
}

/*
 * TODO: Write appropriate description
 */
void graph::gen_conflicts()
{
	up_conflicts.clear();
	down_conflicts.clear();

	up_conflicts.resize(size());
	down_conflicts.resize(size());
	int j, k;

	map<int, int>::iterator m, n;
	bool parallel;


	// Compare every state in both directions
	for (j = 0; j < size(); j++)
	{
		for (k = 0; k < size(); k++)
		{
			if (k != j)
			{
				parallel = false;
				for (m = states[j].branch.begin(); m != states[j].branch.end() && !parallel; m++)
					for (n = states[k].branch.begin(); n != states[k].branch.end() && !parallel; n++)
						if (m->first == n->first && m->second != n->second)
							parallel = true;

				if (!parallel && BUBBLELESS)
				{
					if (up_conflict(states[j], states[k]))
						up_conflicts[j].push_back(k);
					if (down_conflict(states[j], states[k]))
						down_conflicts[j].push_back(k);
				}
				else if (!parallel && conflict(states[j], states[k]))
				{
					up_conflicts[j].push_back(k);
					down_conflicts[j].push_back(k);
				}
			}
		}
	}
}

/*
 * TODO: Write appropriate description
 */
void graph::gen_traces()
{
	size_t i;
	traces.traces.clear();
	for (i = 0; i < states.width(); i++)
		traces.push_back(states(i));
}

/*
 * TODO: Write appropriate description
 */
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
					if (*sj == '1' && *si != '1' && to_state.fire(k))
					{
						upstr = upstr + "1";
						up_firings[k].push_back(from);
						up_firings_transpose[from].push_back(k);
					}
					else if (*sj == '1' && *si == '1')
						upstr = upstr + "X";
					else
						upstr = upstr + "0";

					if (*sj == '0' && *si != '0' && to_state.fire(k))
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

//Returns the size of the state space (How long, i.e. how many states)
int graph::size()
{
	return states.size();
}

//Returns the width of the trace space (How wide, i.e. how many variables)
int graph::width()
{
	return traces.size();
}

void graph::print_states(vspace *v)
{
	size_t j, k;
	vector<state>::iterator i;
	vector<int>::iterator m;
	vector<string>::iterator q;
	map<int, int>::iterator bi;

	size_t max_length = 0;
	vector<string> names(v->global.size());
	for (j = 0; j < v->global.size(); j++)
	{
		names[j] = v->get_name(j);
		if (names[j].length() > max_length)
			max_length = names[j].length();
	}

	for (j = 0; j < max_length; j++)
	{
		for (k = 0; k < names.size(); k++)
		{
			if (max_length - j <= names[k].length())
				cout << names[k][names[k].length() - (max_length - j)] << " ";
			else
				cout << "  ";
		}
		cout << endl;
	}
	cout << endl;
	for (i = states.begin(), j = 0; i != states.end(); i++, j++)
	{
		cout << *i << " ";

		for (bi = i->branch.begin(); bi != i->branch.end(); bi++)
			cout << "[" << bi->first << "," << bi->second << "]";

		cout << "\t" << j << " -> { ";

		if (j < front_edges.size())
			for (m = front_edges[j].begin(); m != front_edges[j].end(); m++)
				cout << *m << " ";

		cout << "}\t";
		if (j < transitions.size())
			for (q = transitions[j].begin(); q != transitions[j].end(); q++)
				cout << *q << " ";

		cout << endl;
	}

	cout << endl;
}

void graph::print_traces(vspace *v)
{
	size_t j, k;
	size_t max_length = 0;
	vector<string> names(v->global.size());
	for (j = 0; j < v->global.size(); j++)
	{
		names[j] = v->get_name(j);
		if (names[j].length() > max_length)
			max_length = names[j].length();
	}

	for (j = 0; j < traces.size(); j++)
	{
		cout << names[j];
		for (k = 0; k < max_length - names[j].length() + 2; k++)
			cout << " ";
		cout << traces.traces[j] << endl;
	}

	cout << endl;
}

void graph::print_up(vspace *v)
{
	size_t j, k;
	size_t max_length = 0;
	vector<string> names(v->global.size());
	for (j = 0; j < v->global.size(); j++)
	{
		names[j] = v->get_name(j);
		if (names[j].length() > max_length)
			max_length = names[j].length();
	}

	for (j = 0; j < traces.size(); j++)
	{
		cout << names[j];
		for (k = 0; k < max_length - names[j].length() + 2; k++)
			cout << " ";
		cout << up.traces[j] << endl;
	}

	cout << endl;
}

void graph::print_down(vspace *v)
{
	size_t j, k;
	size_t max_length = 0;
	vector<string> names(v->global.size());
	for (j = 0; j < v->global.size(); j++)
	{
		names[j] = v->get_name(j);
		if (names[j].length() > max_length)
			max_length = names[j].length();
	}

	for (j = 0; j < traces.size(); j++)
	{
		cout << names[j];
		for (k = 0; k < max_length - names[j].length() + 2; k++)
			cout << " ";
		cout << down.traces[j] << endl;
	}

	cout << endl;
}

void graph::print_delta(vspace *v)
{
	size_t j, k;
	size_t max_length = 0;
	vector<string> names(v->global.size());
	for (j = 0; j < v->global.size(); j++)
	{
		names[j] = v->get_name(j);
		if (names[j].length() > max_length)
			max_length = names[j].length();
	}

	for (j = 0; j < traces.size(); j++)
	{
		cout << names[j];
		for (k = 0; k < max_length - names[j].length() + 2; k++)
			cout << " ";
		cout << delta.traces[j] << endl;
	}

	cout << endl;
}

void graph::print_conflicts()
{
	size_t i, j;

	cout << "Up Production Rule Conflicts" << endl;
	for (i = 0; i < up_conflicts.size(); i++)
	{
		cout << i << ": ";
		for (j = 0; j != up_conflicts[i].size(); j++)
			cout << up_conflicts[i][j] << " ";
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

	cout << endl;
}

void graph::print_firings(vspace *v)
{
	size_t i, j, k;
	size_t max_length = 0;
	vector<string> names(v->global.size());
	for (j = 0; j < v->global.size(); j++)
	{
		names[j] = v->get_name(j);
		if (names[j].length() > max_length)
			max_length = names[j].length();
	}

	cout << "Up Production Rule Firings" << endl;
	for (i = 0; i < up_firings.size(); i++)
	{
		cout << names[i];
		for (k = 0; k < max_length - names[i].length() + 2; k++)
			cout << " ";
		for (j = 0; j < up_firings[i].size(); j++)
			cout << up_firings[i][j] << " ";
		cout << endl;
	}

	cout << "Down Production Rule Firings" << endl;
	for (i = 0; i < down_firings.size(); i++)
	{
		cout << names[i];
		for (k = 0; k < max_length - names[i].length() + 2; k++)
			cout << " ";
		for (j = 0; j < down_firings[i].size(); j++)
			cout << down_firings[i][j] << " ";
		cout << endl;
	}

	cout << endl;
}

/* print_dot generates and outputs to a file called "graph.dot". The function
 * prints graph's state space and transitions in a .dot format, recognized by
 * the graph generation program, graphviz. Visit http://www.graphviz.org for
 * appropriate plugins to render .dot files and more information on file
 * structure.
 */
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

//TODO: Is it proper that we are printing a statespace as the default << of a graph,
//given all the info it contains?
ostream &operator<<(ostream &os, graph g)
{
	size_t j;
	vector<state>::iterator i;
	vector<int>::iterator m;
	vector<string>::iterator q;
	map<int, int>::iterator bi;

	os << "State Space:" << endl;
	for (i = g.states.begin(), j = 0; i != g.states.end(); i++, j++)
	{
		os << *i << "\t";

		for (bi = i->branch.begin(); bi != i->branch.end(); bi++)
			cout << "[" << bi->first << "," << bi->second << "]";

		cout << "\t" << j << " -> { ";

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

//Does this answer the above TODO sufficiently?
ostream &operator>>(ostream &os, graph g)
{
	os << "Trace Space:" << endl;
	os << g.traces;

	return os;
}
