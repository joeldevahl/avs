#include "avs.h"

#include <stdint.h>
#include <cassert>
#include <vector>
#include <float.h>
#include <math.h>

typedef uint32_t avs_index_t;

#define AVS_INVALID_INDEX UINT32_MAX

struct avs_node_t
{
	avs_index_t child_id[8];
	avs_index_t brick_id;
};

#define AVS_BRICK_SIDE 32
#define AVS_BRICK_SIZE (AVS_BRICK_SIDE * AVS_BRICK_SIDE * AVS_BRICK_SIDE)

struct avs_brick_t
{
	float data[AVS_BRICK_SIZE];
};

struct avs_vec_t
{
	float x, y, z;
};

struct avs_bb_t
{
	avs_vec_t min, max;
};

struct avs_t
{
	size_t node_pool_size;
	size_t brick_pool_size;

	std::vector<avs_node_t> nodes;
	std::vector<avs_index_t> free_nodes;

	std::vector<avs_brick_t> bricks;
	std::vector<avs_index_t> free_bricks;

	avs_index_t root_index;
	float root_side;
	avs_vec_t root_origin;
};

