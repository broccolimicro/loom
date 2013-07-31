/*
 * flag_space.cpp
 *
 *  Created on: Jul 24, 2013
 *      Author: nbingham
 */

#include "flag_space.h"

flag_space::flag_space()
{
	debug = 0x00000057;
	warning = 0;
	optimization = 0;
	log = 0;

	output_file = NULL;
	log_file = (ofstream*)&cout;

	tab = "";
}

flag_space::~flag_space()
{
	int i;
	for (i = 0; i < (int)input_files.size(); i++)
	{
		if (input_files[i] != NULL)
		{
			input_files[i]->close();
			delete input_files[i];
		}
		input_files[i] = NULL;
	}

	if (output_file != NULL)
	{
		output_file->close();
		delete output_file;
	}
	output_file = NULL;

	if (log_file != (ofstream*)&cout)
	{
		log_file->close();
		delete log_file;
	}
	log_file = NULL;
}

void flag_space::set_pre()
{
	debug &= 0xFFFFFF00;
	debug |= 0x00000001;
}

bool flag_space::pre()
{
	return ((debug & 0x00000081) == 0x00000001);
}

void flag_space::set_hse()
{
	debug &= 0xFFFFFF00;
	debug |= 0x00000003;
}

bool flag_space::hse()
{
	return ((debug & 0x00000082) == 0x00000002);
}

void flag_space::set_sse()
{
	debug &= 0xFFFFFF00;
	debug |= 0x00000007;
}
bool flag_space::sse()
{
	return ((debug & 0x00000084) == 0x00000004);
}

void flag_space::set_svib()
{
	debug &= 0xFFFFFF00;
	debug |= 0x0000000F;
}

bool flag_space::svib()
{
	return ((debug & 0x00000088) == 0x00000008);
}

void flag_space::set_svi()
{
	debug &= 0xFFFFFF00;
	debug |= 0x00000017;
}

bool flag_space::svi()
{
	return ((debug & 0x00000090) == 0x00000010);
}

void flag_space::set_prsb()
{
	debug &= 0xFFFFFF00;
	debug |= 0x000002F;
}

bool flag_space::prsb()
{
	return ((debug & 0x000000A0) == 0x00000020);
}

void flag_space::set_prs()
{
	debug &= 0xFFFFFF00;
	debug |= 0x0000057;
}

bool flag_space::prs()
{
	return ((debug & 0x000000C0) == 0x00000040);
}

void flag_space::set_rsse()
{
	debug &= 0xFFFFFF00;
	debug |= 0x00000082;
}

bool flag_space::rsse()
{
	return ((debug & 0x00000082) == 0x00000082);
}

void flag_space::set_log_all()
{
	log = 0xFFFFFFFF;
}

void flag_space::set_log_none()
{
	log = 0x00000000;
}

void flag_space::set_log_all_hse()
{
	log |= 0x0000007F;
}

void flag_space::set_log_all_sse()
{
	log |= 0x000000180;
}

void flag_space::set_log_all_prs()
{
	log |= 0x000000E00;
}

void flag_space::set_log_precompile()
{
	log |= 0x00000001;
}

bool flag_space::log_precompile()
{
	return ((log & 0x00000001) == 0x00000001);
}

void flag_space::set_log_base_hse()
{
	log |= 0x00000002;
}

bool flag_space::log_base_hse()
{
	return ((log & 0x00000002) == 0x00000002);
}

void flag_space::set_log_merged_hse()
{
	log |= 0x00000004;
}

bool flag_space::log_merged_hse()
{
	return ((log & 0x00000004) == 0x00000004);
}

void flag_space::set_log_projected_hse()
{
	log |= 0x00000008;
}

bool flag_space::log_projected_hse()
{
	return ((log & 0x00000008) == 0x00000008);
}

void flag_space::set_log_decomposed_hse()
{
	log |= 0x00000010;
}

bool flag_space::log_decomposed_hse()
{
	return ((log & 0x00000010) == 0x00000010);
}

void flag_space::set_log_reshuffled_hse()
{
	log |= 0x00000020;
}

bool flag_space::log_reshuffled_hse()
{
	return ((log & 0x00000020) == 0x00000020);
}

void flag_space::set_log_state_var_hse()
{
	log |= 0x00000040;
}

bool flag_space::log_state_var_hse()
{
	return ((log & 0x00000040) == 0x00000040);
}

void flag_space::set_log_base_state_space()
{
	log |= 0x00000080;
}

bool flag_space::log_base_state_space()
{
	return ((log & 0x00000080) == 0x00000080);
}

void flag_space::set_log_state_var_state_space()
{
	log |= 0x00000100;
}

bool flag_space::log_state_var_state_space()
{
	return ((log & 0x00000100) == 0x00000100);
}

void flag_space::set_log_base_prs()
{
	log |= 0x00000200;
}

bool flag_space::log_base_prs()
{
	return ((log & 0x00000200) == 0x00000200);
}

void flag_space::set_log_factored_prs()
{
	log |= 0x00000400;
}

bool flag_space::log_factored_prs()
{
	return ((log & 0x00000400) == 0x00000400);
}

void flag_space::set_log_bubble_reshuffled_prs()
{
	log |= 0x00000800;
}

bool flag_space::log_bubble_reshuffled_prs()
{
	return ((log & 0x00000800) == 0x00000800);
}

void flag_space::inc()
{
	tab += "\t";
}

void flag_space::dec()
{
	tab = tab.substr(0, tab.size()-1);
}
