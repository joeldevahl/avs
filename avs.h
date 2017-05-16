#pragma once

struct avs_t;

enum avs_result_t
{
	AVS_RESULT_OK = 0,
	AVS_RESULT_OUTSIDE_FIELD,
};

struct avs_create_info_t
{
	int initial_pool_size;
	float root_size;
	float saturation_distance;
};

avs_result_t avs_create(const avs_create_info_t* create_info, avs_t** out_avs);

void avs_destroy(avs_t* avs);

struct avs_sample_result_t
{
	float x;
	float y;
	float z;

	float field_val;
};

avs_result_t avs_sample_point(avs_t* avs, float x, float y, float z, avs_sample_result_t* out_result);

void avs_paint_sphere(avs_t* avs, float center_x, float center_y, float center_z, float radius);
