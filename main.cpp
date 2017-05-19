#include "avs.h"

#include <float.h>

int main(int argc, char** argv)
{
	avs_t* avs = nullptr;
	avs_create_info_t avs_create_info = {};
	avs_create_info.saturation_distance = 1.0f;
	avs_create(&avs_create_info, &avs);

	avs_paint_sphere(avs, 1.0f, 1.0f, 1.0f, 0.5f);

	avs_sample_result_t res = {};
	avs_sample_point(avs, 1.0f, 1.0f, 1.0f, &res);

	avs_sample_result_t res2 = {};
	avs_trace_ray(avs, 0.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1024.0f, &res2);

	avs_destroy(avs);

	return 0;
}
