/*
 * flags.h
 *
 *  Created on: Jul 24, 2013
 *      Author: nbingham
 */

#include "common.h"

#ifndef flag_space_h
#define flag_space_h

struct flag_space
{
	flag_space();
	~flag_space();

	uint32_t debug;
	uint32_t warning;
	uint32_t optimization;
	uint32_t log;

	vector<ifstream*>	input_files;
	ofstream			*output_file;
	ofstream			*log_file;

	string tab;

	// Debug
	void set_pre();
	bool pre();
	void set_hse();
	bool hse();
	void set_sse();
	bool sse();
	void set_svib();
	bool svib();
	void set_svi();
	bool svi();
	void set_prsb();
	bool prsb();
	void set_prs();
	bool prs();
	void set_rsse();
	bool rsse();

	// Log
	void set_log_all();
	void set_log_none();
	void set_log_all_hse();
	void set_log_all_sse();
	void set_log_all_prs();

	void set_log_precompile();
	bool log_precompile();
	void set_log_base_hse();
	bool log_base_hse();
	void set_log_merged_hse();
	bool log_merged_hse();
	void set_log_projected_hse();
	bool log_projected_hse();
	void set_log_decomposed_hse();
	bool log_decomposed_hse();
	void set_log_reshuffled_hse();
	bool log_reshuffled_hse();
	void set_log_state_var_hse();
	bool log_state_var_hse();
	void set_log_base_state_space();
	bool log_base_state_space();
	void set_log_state_var_state_space();
	bool log_state_var_state_space();
	void set_log_base_prs();
	bool log_base_prs();
	void set_log_factored_prs();
	bool log_factored_prs();
	void set_log_bubble_reshuffled_prs();
	bool log_bubble_reshuffled_prs();

	void inc();
	void dec();
};

#endif
