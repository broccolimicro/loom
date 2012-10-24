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

#include "common.h"
#include "keyword.h"
#include "variable.h"
#include "instruction.h"
#include "space.h"
#include "record.h"

struct process;
struct block;

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
	block(string chp, map<string, variable> svars)
	{
		parse(chp, svars);
	}
	~block()
	{
		raw = "";
	}

	string 					raw;							// the raw chp of this block
	map<string, variable>	vars;
	list<process*>			procs;		// a list of pointers to subprocesses
	list<block>				blocks;		// a list of sub-blocks
	list<instruction>		instrs;		// an ordered list of instructions in block
	map<string, space>		states;		// the state space of this block. format "i####" or "o####"
	list<string>			prs;

	block &operator=(block b)
	{
		raw = b.raw;
		procs = b.procs;
		blocks = b.blocks;
		instrs = b.instrs;
		states = b.states;
		return *this;
	}

	void parse(string chp, map<string, variable> svars)
	{
		vars = svars;	//The variables this block uses.

		instruction instr; 							//Lists are pass by value, right? Else this wont work

		raw = chp;
		string potential_instr;						//String of CHP code to be tested as an instruction
		string rest_of_chp = chp; 					//The segment of CHP we have not yet parsed
		string::iterator i,j;
		list<instruction>::iterator curr_instr;  	//Used later to iterate through instr lists
		map<string, variable>::iterator curr_var;
		cout << "\tblock!  -> "+chp << endl;  		// Output the raw block


		//Parse instructions!
		for(i = chp.begin(), j = chp.begin();i != chp.end(); i++){				//Iterate through the entire string
			if (*i == ';'){														//We nabbed an instruction!
				potential_instr = chp.substr(j-chp.begin(), i-j);
				instr.parse(potential_instr);
				instrs.push_back(instr);

				if((vars.find(instr.var_affected) == vars.end()) && (instr.var_affected!="Unhandled") ){
					cout<< "Error: you are trying to call an instruction that operates on a variable not in this block's scope: " + instr.var_affected << endl;
				}

				j = i+1;
			}
		}


		//Turn instructions into states!
		//Remember as we add instructions to X out the appropriate vars when we change "important" inputs
		for(curr_var = vars.begin(); curr_var != vars.end(); curr_var++){
			states[curr_var->first].states.push_back(" X");
			states[curr_var->first].var = curr_var->first;
			for(curr_instr = instrs.begin(); curr_instr != instrs.end(); curr_instr++){
				if ((curr_var->first == curr_instr->var_affected)&&(curr_instr->val_at_end != "NA")){

					states[curr_var->first].states.push_back(curr_instr->val_at_end);

				}else if(*(states[curr_var->first].states.rbegin()) != " X"){
					states[curr_var->first].states.push_back(*(states[curr_var->first].states.rbegin()));

				}else{
					states[curr_var->first].states.push_back(" X");
				}
			}
			cout << states[curr_var->first]<< endl;
		}


		cout << (states["l.a"] == states["x"]) << endl;
		cout << (states["r.a"] == states["x"]) << endl;

		map<string, space>::iterator k, l;

		for (k = states.begin(); k != states.end(); k++)
		{
			cout << " ";
		}

		cout << endl;




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
		_kind = "process";
	}
	process(string chp, map<string, keyword*> typ)
	{
		parse(chp, typ);
		_kind = "process";
	}
	~process()
	{
		name = "";
		_kind = "process";
	}

	block					def;	// the chp that defined this process
	list<string>			prs;	// the final set of generated production rules
	map<string, variable>	io;		// the input and output signals of this process

	process &operator=(process p)
	{
		def = p.def;
		prs = p.prs;
		io = p.io;
		return *this;
	}

	void parse(string chp, map<string, keyword*> typ)
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

		map<string, variable> vars;
		variable v;

		name = chp.substr(name_start, name_end - name_start);
		io_block = chp.substr(input_start, input_end - input_start);

		cout << "\tname!   -> "+name << endl;
		cout << "\tinputs! -> "+io_block << endl;

		for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
		{
			if (*(i+1) == ',' || i+1 == io_block.end())
			{
				v.parse(io_block.substr(j-io_block.begin(), i+1 - j));
				vars = expand(v, typ);
				io.insert(vars.begin(), vars.end());
				j = i+2;
			}
		}

		def.parse(chp.substr(block_start, block_end - block_start), io);
	}
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

		// Define the basic types. In this case, 'int'
		type_space.insert(pair<string, keyword*>("int", new keyword("int")));

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
						p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space);
						type_space.insert(pair<string, process*>(p->name, p));
					}
					// This isn't a process, is it a record?
					else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "struct ") == 0)
					{
						r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space);
						type_space.insert(pair<string, record*>(r->name, r));
					}
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
						{
							p = new process(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space);
							type_space.insert(pair<string, process*>(p->name, p));
						}
						else if (cleaned_chp.compare(j-cleaned_chp.begin(), 7, "struct ") == 0)
						{
							r = new record(cleaned_chp.substr(j-cleaned_chp.begin(), i-j+1), type_space);
							type_space.insert(pair<string, record*>(r->name, r));
						}
					}
				}
				j = i+1;
			}
		}

		main.parse("main m()");
	}
};


int main(int argc, char **argv)
{
	ifstream t("test.chp");
	string prgm((istreambuf_iterator<char>(t)),
	             istreambuf_iterator<char>());

	program p(prgm);
}
