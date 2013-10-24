/*
 * chp.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"
#include "syntax.h"
#include "data.h"
#include "type.h"
#include "program.h"
#include "utility.h"

int main(int argc, char **argv)
{
	program prgm;
	prgm.flags.working_directory = string(argv[0]).substr(0, string(argv[0]).find_last_of("\\/"));

	for (int i = 1; i < argc; i++)
	{
		if (strncmp(argv[i], "--help", 6) == 0)			// Help
		{
			cout << "Usage: haystack [options] file" << endl;
			cout << "Options:" << endl;
			cout << " --help\t\t\tDisplay this information" << endl;
			cout << " --version\t\tDisplay compiler version information" << endl;
			cout << " -h <file>\t\tPlace all resulting HSE into <file>" << endl;
			cout << " -s <file>\t\tPlace all resulting state space graph into <file>" << endl;
			cout << " -o <file>\t\tPlace all resulting production rules into <file>" << endl;
			cout << " -l <file>\t\tPlace log into <file>" << endl;
			cout << " -D[option]\t\tDebugging options" << endl;
			cout << " \tThe usual pipeline is as follows: preprocessor, handshaking expansions, " << endl;
			cout << " \telaborate the state space, insert bubbleless state variables, generate " << endl;
			cout << " \tbubbleless production rules. The following options modify the pipeline " << endl;
			cout << " \tto only execute certain parts." << endl;
			cout << " \t-Dpre\t\t\tPreprocessor only" << endl;
			cout << " \t-Dhse\t\t\tDown to handshaking expansions" << endl;
			cout << " \t-Dsse\t\t\tDown to state space elaboration" << endl;
			cout << " \t-Dsvib\t\t\tDown to state variable insertion (bubbles allowed)" << endl;
			cout << " \t-Dsvi\t\t\tDown to state variable insertion (bubbles not allowed)" << endl;
			cout << " \t-Dprsb\t\t\tDown to production rule generation (bubbles allowed)" << endl;
			cout << " \t-Dprs\t\t\tDown to production rule generation (default, bubbles not allowed)" << endl;
			cout << endl;
			cout << " \tAside from the usual pipeline, you can also go backwards. The following " << endl;
			cout << " \toptions modify the pipeline to go in the reverse direction." << endl;
			// TODO cout << " \t-Drprs\t\t\tBubble reshuffle the given production rules" << endl;
			cout << " \t-Drsse\t\t\tUp to state space elaboration" << endl;
			cout << " -W[option]\t\tWarning options" << endl;
			cout << " -O[option]\t\tOptimization options" << endl;
			cout << " -L[option]\t\tLog options" << endl;
			cout << " \t-Lall\t\t\tLog everything" << endl;
			cout << " \t-Lnone\t\t\tLog nothing" << endl;
			cout << " \t-Lall_hse\t\tLog all handshaking expansion functionality" << endl;
			cout << " \t-Lall_sse\t\tLog all state space elaboration functionality" << endl;
			cout << " \t-Lall_prs\t\tLog all production rule functionality" << endl;
			cout << " \t-Lpreprocess\t\tLog the preprocessing steps" << endl;
			cout << " \t-Lbase_hse\t\tLog the steps taken to generate the base hse from chp" << endl;
			cout << " \t-Lmerged_hse\t\tLog the conditional and assignment merges" << endl;
			cout << " \t-Lprojected_hse\t\tLog the projections applied to the hse" << endl;
			cout << " \t-Ldecomposed_hse\tLog the steps taken during process decomposition" << endl;
			cout << " \t-Lreshuffled_hse\tLog the steps taken during handshaking reshuffling" << endl;
			cout << " \t-Lsvi_hse\t\tLog hse that results from state variable insertion" << endl;
			cout << " \t-Lbase_sse\t\tLog the steps taken during state space elaboration" << endl;
			cout << " \t-Lsvi_sse\t\tLog the steps taken during state variable insertion" << endl;
			cout << " \t-Lbase_prs\t\tLog the steps taken during production rule generation" << endl;
			cout << " \t-Lfactored_prs\t\tLog the steps taken during prs factoring" << endl;
			cout << " \t-Lreshuffled_prs\tLog the steps taken during bubble reshuffling" << endl;
			cout << endl;
			//cout << "For bug reporting instruction, please see:" << endl;
			//cout << "<https://www.sol-union.com/bugs.php>" << endl;
			//cout << endl;
		}
		else if (strncmp(argv[i], "--version", 9) == 0)	// Version Information
		{
			cout << "haystack 1.0.1" << endl;
			cout << "Copyright (C) 2013 Sol Union." << endl;
			cout << "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
			cout << endl;
		}
		else if (strncmp(argv[i], "-h", 2) == 0)		// Output File
		{
			i++;
			if (i < argc)
				prgm.flags.hse_file = new ofstream(argv[i]);
			else
				cout << "error: missing filename after '-o'" << endl;
		}
		else if (strncmp(argv[i], "-s", 2) == 0)		// Output File
		{
			i++;
			if (i < argc)
				prgm.flags.state_file = new ofstream(argv[i]);
			else
				cout << "error: missing filename after '-o'" << endl;
		}
		else if (strncmp(argv[i], "-o", 2) == 0)		// Output File
		{
			i++;
			if (i < argc)
				prgm.flags.output_file = new ofstream(argv[i]);
			else
				cout << "error: missing filename after '-o'" << endl;
		}
		else if (strncmp(argv[i], "-l", 2) == 0)
		{
			i++;
			if (i < argc)
				prgm.flags.log_file = new ofstream(argv[i]);
			else
				cout << "error: missing filename after '-l'" << endl;
		}
		else if (strncmp(argv[i], "-W", 2) == 0)		// Warning Flags
		{

		}
		else if (strncmp(argv[i], "-O", 2) == 0)		// Optimization Flags
		{

		}
		else if (strncmp(argv[i], "-L", 2) == 0)		// Log Flags
		{
			if (strncmp(argv[i], "-Lall", 5) == 0)
				prgm.flags.set_log_all();
			else if (strncmp(argv[i], "-Lnone", 6) == 0)
				prgm.flags.set_log_none();
			else if (strncmp(argv[i], "-Lall_hse", 9) == 0)
				prgm.flags.set_log_all_hse();
			else if (strncmp(argv[i], "-Lall_sse", 9) == 0)
				prgm.flags.set_log_all_sse();
			else if (strncmp(argv[i], "-Lall_prs", 9) == 0)
				prgm.flags.set_log_all_prs();
			else if (strncmp(argv[i], "-Lpreprocess", 12) == 0)
				prgm.flags.set_log_precompile();
			else if (strncmp(argv[i], "-Lbase_hse", 10) == 0)
				prgm.flags.set_log_base_hse();
			else if (strncmp(argv[i], "-Lmerged_hse", 12) == 0)
				prgm.flags.set_log_merged_hse();
			else if (strncmp(argv[i], "-Lprojected_hse", 15) == 0)
				prgm.flags.set_log_projected_hse();
			else if (strncmp(argv[i], "-Ldecomposed_hse", 16) == 0)
				prgm.flags.set_log_decomposed_hse();
			else if (strncmp(argv[i], "-Lreshuffled_hse", 16) == 0)
				prgm.flags.set_log_reshuffled_hse();
			else if (strncmp(argv[i], "-Lsvi_hse", 9) == 0)
				prgm.flags.set_log_state_var_hse();
			else if (strncmp(argv[i], "-Lbase_sse", 10) == 0)
				prgm.flags.set_log_base_state_space();
			else if (strncmp(argv[i], "-Lsvi_sse", 9) == 0)
				prgm.flags.set_log_state_var_state_space();
			else if (strncmp(argv[i], "-Lbase_prs", 10) == 0)
				prgm.flags.set_log_base_prs();
			else if (strncmp(argv[i], "-Lfactored_prs", 13) == 0)
				prgm.flags.set_log_factored_prs();
			else if (strncmp(argv[i], "-Lreshuffled_prs", 16) == 0)
				prgm.flags.set_log_bubble_reshuffled_prs();
		}
		else if (strncmp(argv[i], "-D", 2) == 0)		// Debug Flags
		{
			if (strncmp(argv[i], "-Dpre", 5) == 0)
				prgm.flags.set_pre();
			else if (strncmp(argv[i], "-Dhse", 5) == 0)
				prgm.flags.set_hse();
			else if (strncmp(argv[i], "-Dsse", 5) == 0)
				prgm.flags.set_sse();
			else if (strncmp(argv[i], "-Dsvib", 5) == 0)
				prgm.flags.set_svib();
			else if (strncmp(argv[i], "-Dsvi", 5) == 0)
				prgm.flags.set_svi();
			else if (strncmp(argv[i], "-Dprsb", 5) == 0)
				prgm.flags.set_prsb();
			else if (strncmp(argv[i], "-Dprs", 5) == 0)
				prgm.flags.set_prs();
			else if (strncmp(argv[i], "-Drsse", 5) == 0)
				prgm.flags.set_rsse();
		}
		else
		{
			prgm.flags.input_files.push_back(pair<sstring, ifstream*>(sstring(argv[i]).substr(0, sstring(argv[i]).find_last_of("\\/")), new ifstream(argv[i])));
			if (!prgm.flags.input_files.back().second->is_open())
				cerr << "Error: File not found " << argv[i] << endl;
		}
	}
	//FILE *log = fopen("log.txt", "w");
	//*stdout = *log;

	prgm.compile();

	//fclose(log);

	/*ifstream t("fifo1b.prs");
	sstring prgm((istreambuf_iterator<char>(t)),
				 istreambuf_iterator<char>());

	process p;

	p.parse_prs(prgm);
	p.elaborate_prs();

	ofstream f("verify_fifo1b.dot");
	p.net.print_dot(&f, "verify_fifo1b");
	f.close();*/

}
