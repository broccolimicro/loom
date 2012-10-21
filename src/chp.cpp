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
 * 	 @x
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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <list>
#include <regex>

using namespace std;

struct keyword;
struct record;
struct process;
struct block;
struct variable;
struct space;

/* Is this character a character that is legal to have
 * in a type name or variable name? a-z A-Z 0-9 _
 */
bool nc(char c)
{
	return ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') ||
			(c == '_'));
}

/* Is this character an operator?
 *
 */
bool oc(char c)
{
	return (c == ':' ||
			c == '=' ||
			c == '|' ||
			c == '&' ||
			c == '~' ||
			c == '>' ||
			c == '<' ||
			c == ';' ||
			c == '*' ||
			c == '[' ||
			c == ']' ||
			c == '(' ||
			c == ')' ||
			c == '{' ||
			c == '}' ||
			c == '+' ||
			c == '-' ||
			c == '!' ||
			c == '?' ||
			c == '@' ||
			c == '#');
}

/* Is this character whitespace?
 *
 */
bool sc(char c)
{
	return (c == ' '  ||
			c == '\t' ||
			c == '\n' ||
			c == '\r');
}

/* This structure represents the basic data types. The only
 * basic data type currently defined is 'int' which represents
 * an n-bit integer. The only thing that we need to keep track
 * of these basic data types is its name.
 */
struct keyword
{
	keyword()
	{
		name = "";
	}
	keyword(string n)
	{
		name = n;
	}
	~keyword()
	{
		name = "";
	}

	string name;
};

/* This structure describes a variable in the chp program. We need
 * to keep track of a variables name, its type (record, keyword, process),
 * and its bit width.
 */
struct variable
{
	variable()
	{
		name = "";
		type = NULL;
		width = 0;
	}
	variable(string chp)
	{
		cout << "\t\tvariable! -> "+chp << endl;
	}
	~variable()
	{
		name = "";
		type = NULL;
		width = 0;
	}

	string		name;		// the name of the instantiated variable
	keyword		*type;		// a pointer to the type of the instantiated variable
	uint16_t	width;		// the bit width of the instantiated variable

	void parse(){}
};

/* This structure represents a structure or record. A record
 * contains a bunch of member variables that help you index
 * segments of bits within the multibit signal.
 */
struct record : keyword
{
	record()
	{
		name = "";
	}
	record(string chp)
	{
		parse(chp);
	}
	~record()
	{
		name = "";
	}

	list<variable> vars;	// the list of member variables that make up this record

	void parse(string chp)
	{
		cout << "record! -> " << chp << endl;
		int name_start = chp.find_first_of(" ")+1;
		int name_end = chp.find_first_of("{");
		int block_start = chp.find_first_of("{")+1;
		int block_end = chp.length()-1;
		string::iterator i, j;
		string io_block;

		name = chp.substr(name_start, name_end - name_start);
		io_block = chp.substr(block_start, block_end - block_start);

		cout << "\tname!   -> "+name << endl;
		cout << "\tblock!  -> "+io_block << endl;

		for (i = io_block.begin(), j = io_block.begin(); i < io_block.end(); i++)
		{
			if (*(i+1) == ';')
			{
				vars.push_back(variable(io_block.substr(j-io_block.begin(), i+1 - j)));
				j = i+2;
			}
		}
	}
};

/* This structure represents a block. An conditional statement
 * or loop can be considered a block. By definition, it is
 * something that we can pull out and analyze independently of
 * every other structure in the program. Within a block, there
 * can be a process instantiation, variable declarations, and sub-blocks.
 * We also include a list of spaces to keep track of the state space
 * transition affected by this block.
 */
struct block
{
	block()
	{
		raw = "";
	}
	block(string chp)
	{
		parse(chp);
	}
	~block()
	{
		raw = "";
	}

	string raw;					// the raw chp of this block
	list<process*>	procs;		// a list of pointers to subprocesses
	list<block>		blocks;		// a list of sub-blocks
	list<variable>	vars;		// the variable space of this block
	list<space>		states;		// the state space of this block

	void parse(string chp)
	{
		raw = chp;

		cout << "\tblock!  -> "+chp << endl;
	}
};

/* This structure represents a process. Processes act in parallel
 * with each other and can only communicate with other processes using
 * channels. We need to keep track of the block that defines this process and
 * the input and output signals. The final element in this structure is
 * a list of production rules that are the result of the compilation.
 */
struct process : keyword
{
	process()
	{
		name = "";
	}
	process(string chp)
	{
		parse(chp);
	}
	~process()
	{
		name = "";
	}

	block			def;	// the chp that defined this process
	list<string>	prs;	// the final set of generated production rules
	list<variable>	io;		// the input and output signals of this process

	void parse(string chp)
	{
		cout << "process! -> " << chp << endl;
		int name_start = chp.find_first_of(" ")+1;
		int name_end = chp.find_first_of("(");
		int input_start = chp.find_first_of("(")+1;
		int input_end = chp.find_first_of(")");
		int block_start = chp.find_first_of("{")+1;
		int block_end = chp.length()-1;
		string io_block;
		string::iterator i, j;

		name = chp.substr(name_start, name_end - name_start);
		io_block = chp.substr(input_start, input_end - input_start);

		cout << "\tname!   -> "+name << endl;
		cout << "\tinputs! -> "+io_block << endl;

		for (i = io_block.begin(), j = io_block.begin(); i < io_block.end(); i++)
		{
			if (*(i+1) == ',' || i+1 == io_block.end())
			{
				io.push_back(variable(io_block.substr(j-io_block.begin(), i+1 - j)));
				j = i+2;
			}
		}

		def.parse(chp.substr(block_start, block_end - block_start));
	}
};

/* This structure represents the state space of a single variable as affected by
 * a block of chp. We need to keep track of the variable to with this state space
 * belongs, the number of states in the space, and the raw state data.
 */
struct space
{
	space()
	{
		var = NULL;
		size = 0;
		data = NULL;
	}
	~space()
	{
		if (data != NULL)
			delete [] data;

		var = NULL;
		size = 0;
		data = NULL;
	}

	variable	*var;		// a pointer to the variable that this state space describes
	uint8_t		size;		// the number of states in this state space
	string		*data;		// the raw data of the state space

	void parse(){}
};

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
		type_space.push_back(keyword("int"));
	}
	program(string chp)
	{
		parse(chp);
	}
	~program()
	{

	}

	list<keyword>	type_space;
	list<variable>	globals;
	list<string>	prs;
	list<string>	errors;

	void parse(string chp)
	{
		string::iterator i, j;
		string cleaned_chp = "";
		string word;
		string error;
		int error_start, error_len;

		// Define the basic types. In this case, 'int'
		type_space.push_back(keyword("int"));

		// remove extraneous whitespace
		for (i = chp.begin(); i < chp.end(); i++)
		{
			if (!sc(*i))
				cleaned_chp += *i;
			else if (nc(*(i-1)) && (i == chp.end()-1 || nc(*(i+1))))
				cleaned_chp += ' ';
		}

		// split the program into records and processes
		int depth[3] = {0};
		for (i = cleaned_chp.begin(), j = cleaned_chp.begin(); i < cleaned_chp.end(); i++)
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
						type_space.push_back(process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1)));
					// This isn't a process, is it a record?
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "struct ") == 0)
						type_space.push_back(record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1)));
					// This isn't either a process or a record, this is an error.
					else
					{
						error = "Error: CHP block outside of process.\nIgnoring block:\t";
						error_start = j-cleaned_chp.begin();
						error_len = min(cleaned_chp.find("proc ", error_start), cleaned_chp.find("struct ", error_start)) - error_start;
						error += cleaned_chp.substr(error_start, error_len);
						cout << error << endl;
						j += error_len;

						// Make sure we don't miss the next record or process though.
						if (cleaned_chp.compare(j-cleaned_chp.begin(), 5, "proc ") == 0)
							type_space.push_back(process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1)));
						else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "struct ") == 0)
							type_space.push_back(record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1)));
					}
				}
				j = i+1;
			}
		}
	}
};


int main(int argc, char **argv)
{
	ifstream t("test.chp");
	string prgm((istreambuf_iterator<char>(t)),
	             istreambuf_iterator<char>());

	program p(prgm);
}
