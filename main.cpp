#include "avs.h"

#include <float.h>

int main(int argc, char** argv)
{
	avs_t* avs = nullptr;
	avs_create_info_t avs_create_info = {};
	avs_create_info.initial_pool_size = 128;
	avs_create_info.root_size = 1024.0f;
	avs_create_info.saturation_distance = FLT_MAX;
	avs_create(&avs_create_info, &avs);
	avs_destroy(avs);

	avs_paint_sphere(avs, 512.0f, 512.0f, 512.0f, 256.0f);

	avs_sample_result_t res = {};
	avs_sample_point(avs, 512.0f, 512.0f, 512.0f, &res);

	return 0;
}
