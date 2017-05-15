#pragma once

struct avs_t;

enum avs_result_t
{
	AVS_RESULT_OK = 0,
};

struct avs_create_info_t
{
	int initial_pool_size;
	float root_size;
	float saturation_distance;
};

avs_result_t avs_create(const avs_create_info_t* create_info, avs_t** out_avs);

void avs_destroy(avs_t* avs);
