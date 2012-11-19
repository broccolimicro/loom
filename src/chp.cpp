/*
 * 	Variable Definition
 * 	x1,x2,...,xn:=E1,E2,...,En
 *
 *  x+
 *  x-
 *
 *	Expression Operators
 *  | & ~ < > == ~=
 *
 *  Composition Operators
 *  ; || *
 *
 *	Order of Operations
 *	()
 *
 *	Loops
 * 	*[g1->s1[]g2->s2[]...[]gn->sn]
 *  *[g1->s1|g2->s2|...|gn->sn]
 *  *[s1]
 *
 *	Conditionals
 *   [g1->s1[]g2->s2[]...[]gn->sn]
 *   [g1->s1|g2->s2|...|gn->sn]
 *   [g1]
 *
 *  Replication
 *   <op i : n..m : s(i)>
 *
 *	Communication
 * 	 x!y
 * 	 x?y
 * 	 x@
 *
 *	Miscellaneous
 * 	 skip
 *
 *	Assertion
 * 	 {...}
 *
 *	Process Definition and Function Block Definitions
 * 	proc ...(...){...}
 *
 * 	Record Definition
 * 	struct ...{...}
 *
 *	Data Types
 * 	int<...> ...
 *
 *	Preprocessor
 * 	 #
 *
 */

#include "common.h"
#include "keyword.h"
#include "variable.h"
#include "instruction.h"
#include "space.h"
#include "record.h"
#include "block.h"
#include "process.h"
#include "channel.h"


/* This structure describes a whole program. It contains a record of all
 * of the types described in this program and all of the global variables
 * defined in this program. It also contains a list of all of the errors
 * produced during the compilation and a list of all of the production rules
 * that result from this compilation.
 */
struct program
{
	program()
	{
		type_space.insert(pair<string, keyword*>("int", new keyword("int")));
	}
	program(string chp)
	{
		parse(chp);
	}
	~program()
	{
		map<string, keyword*>::iterator i;
		for (i = type_space.begin(); i != type_space.end(); i++)
			delete i->second;

		type_space.clear();
	}

	map<string, keyword*>	type_space;
	variable				main;
	list<string>			prs;
	list<string>			errors;

	program &operator=(program p)
	{
		type_space = p.type_space;
		main = p.main;
		prs = p.prs;
		errors = p.errors;
		return *this;
	}

	void parse(string chp)
	{
		string::iterator i, j;
		string cleaned_chp = "";
		string word;
		string error;
		int error_start, error_len;

		process *p;
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
					if (cleaned_chp.compare(j-cleaned_chp.begin(), 5, "proc ") == 0)
					{
						p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, map<string, variable*>());
						type_space.insert(pair<string, process*>(p->name, p));
					}
					// This isn't a process, is it a record?
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "record ") == 0)
					{
						r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, "");
						type_space.insert(pair<string, record*>(r->name, r));
					}
					// Is it a channel definition?
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "channel ") == 0)
					{
						c = new channel(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, "");
						type_space.insert(pair<string, channel*>(c->name, c));
					}
					// This isn't either a process or a record, this is an error.
					else
					{
						error = "Error: CHP block outside of process.\nIgnoring block:\t";
						error_start = j-cleaned_chp.begin();
						error_len = min(min(cleaned_chp.find("proc ", error_start), cleaned_chp.find("record ", error_start)), cleaned_chp.find("channel ", error_start)) - error_start;
						error += cleaned_chp.substr(error_start, error_len);
						cout << error << endl;
						j += error_len;

						// Make sure we don't miss the next record or process though.
						if (cleaned_chp.compare(j-cleaned_chp.begin(), 5, "proc ") == 0)
						{
							p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, map<string, variable*>());
							type_space.insert(pair<string, process*>(p->name, p));
						}
						else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "record ") == 0)
						{
							r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, "");
							type_space.insert(pair<string, record*>(r->name, r));
						}
						else if (cleaned_chp.compare(j-cleaned_chp.begin(), 8, "channel ") == 0)
						{
							c = new channel(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space, "");
							type_space.insert(pair<string, channel*>(c->name, c));
						}
					}
				}
				j = i+1;
			}
		}

		main.parse("main m()", "");
	}
};


int main(int argc, char **argv)
{
	//Open the top level file
	ifstream t("main.chp");
	string prgm((istreambuf_iterator<char>(t)),
	             istreambuf_iterator<char>());

	size_t i;
	size_t open, close;

	//While there are and #includes (of the form #include "foo.chp") in the program
	while ((i = prgm.find("#include")) != prgm.npos)
	{
		//Find the file name
		open = prgm.find_first_of("\"", i+1);
		close = prgm.find_first_of("\"", open+1);

		ifstream s(prgm.substr(open+1, close-open-1).c_str());
		string f((istreambuf_iterator<char>(s)),
	             istreambuf_iterator<char>());

		//Place the contents of the file where the #include statement was
		prgm = prgm.substr(0, i) + f + prgm.substr(close+1);

	}

	program p(prgm);
}
