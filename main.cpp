#include "avs.h"

#include <stdio.h>

void print_result(avs_sample_result_t& res)
{
	printf("result: %f\n", res.field_val);
}

int main(int argc, char** argv)
{
	avs_t* avs = nullptr;
	avs_create_info_t avs_create_info = {};
	avs_create_info.root_x = 0;
	avs_create_info.root_y = 0;
	avs_create_info.root_z = 0;
	avs_create_info.root_side = 1024.0f;
	avs_create(&avs_create_info, &avs);

	avs_paint_sphere(avs, 512.0f, 512.0f, 512.0f, 256.0f);

	avs_sample_result_t res = {};
	avs_sample_point(avs, 512.0f, 512.0f, 512.0f, &res);

	print_result(res);

	avs_sample_result_t res2 = {};
	avs_trace_ray(avs, 0.0f, 512.0f, 512.0f, 1.0f, 0.0f, 0.0f, 1024.0f, &res2);

	print_result(res2);

	avs_destroy(avs);

	return 0;
}
