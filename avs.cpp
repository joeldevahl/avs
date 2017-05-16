#include "avs.h"

#include <cassert>
#include <vector>
#include <stdint.h>
#include <float.h>
#include <math.h>

#define AVS_MAX_LAYERS 8

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

struct avs_t
{
	std::vector<avs_node_t> nodes;
	std::vector<avs_index_t> free_nodes;

	std::vector<avs_brick_t> bricks;
	std::vector<avs_index_t> free_bricks;

	avs_index_t root_index;
	float root_side;

	float max_dist;
	float min_dist;
};

static bool avs_node_has_children(avs_node_t* node)
{
	assert(node != nullptr);
	bool res = false;
	for(int i = 0; i < 8; ++i)
		res |= node->child_id[i] == AVS_INVALID_INDEX;
	return res;
}

static void avs_prop_down(avs_t* avs, avs_node_t* node, int cid)
{
	assert(node->child_id[cid] != AVS_INVALID_INDEX);
	avs_node_t& child_node = avs->nodes[node->child_id[cid]];

	// TODO : upsample node brick data into child_node brick data
}

static void avs_prop_down_all(avs_t* avs, avs_node_t* node)
{
	for(int i = 0; i < 8; ++i)
		avs_prop_down(avs, node, i);
}

static void avs_prop_up(avs_t* avs, avs_node_t* node, int cid)
{
	assert(node->child_id[cid] != AVS_INVALID_INDEX);
	avs_node_t& child_node = avs->nodes[node->child_id[cid]];

	// TODO : downsample child_nodes brick data into node brick data
}

static void avs_prop_up_all(avs_t* avs, avs_node_t* node)
{
	for(int i = 0; i < 8; ++i)
		avs_prop_up(avs, node, i);
}

static void avs_refine(avs_t* avs, avs_node_t* node)
{
	for(int i = 0; i < 8; ++i)
	{
		assert(node->child_id[i] == AVS_INVALID_INDEX);
		node->child_id[i] = avs->free_nodes.back();
		avs->free_nodes.pop_back();
	}

	avs_prop_down_all(avs, node);
}

static void avs_flatten(avs_t* avs, avs_node_t* node)
{
	for(int i = 0; i < 8; ++i)
	{
		avs_node_t& child_node = avs->nodes[node->child_id[i]];

		// TODO: recurively flattern
	}

	avs_prop_up_all(avs, node);

	for(int i = 0; i < 8; ++i)
	{
		assert(node->child_id[i] == AVS_INVALID_INDEX);
		node->child_id[i] = avs->free_nodes.back();
		avs->free_nodes.pop_back();
	}
}

avs_result_t avs_create(const avs_create_info_t* create_info, avs_t** out_avs)
{
	avs_t* avs = new avs_t;

	// initialize nodes and node free list
	int initial_node_count = create_info->initial_pool_size;
	avs->nodes.resize(initial_node_count);
	for(int n = initial_node_count - 1; n >= 0; --n)
		avs->free_nodes.push_back(n);

	// initialize bricks and brick free list;
	avs->bricks.resize(initial_node_count);
	for(int b = initial_node_count - 1; b >= 0; --b)
		avs->free_bricks.push_back(b);

	// get first free node from the free list and add that as the root
	avs->root_index = avs->free_nodes.back();
	avs->free_nodes.pop_back();
	avs->root_side = create_info->root_size;

	avs->max_dist = create_info->saturation_distance;
	avs->min_dist = -create_info->saturation_distance;

	// initialize root node data
	avs_node_t& node = avs->nodes[avs->root_index];
	for(int i = 0; i < 8; ++i)
		node.child_id[i] = AVS_INVALID_INDEX;
	node.brick_id = avs->free_bricks.back();
	avs->free_bricks.pop_back();
	avs_brick_t& brick = avs->bricks[node.brick_id];
	for(int i = 0; i < AVS_BRICK_SIZE; ++i)
		brick.data[i] = avs->max_dist;

	*out_avs = avs;
	return AVS_RESULT_OK;
}

void avs_destroy(avs_t* avs)
{
	delete avs;
}

avs_result_t avs_sample_point(avs_t* avs, float x, float y, float z, avs_sample_result_t* out_result)
{
	// Early out if we are outside root boundry
	if(x < 0 || y < 0 || z < 0 || x >= avs->root_side || y >= avs->root_side || z >= avs->root_side)
		return AVS_RESULT_OUTSIDE_FIELD;

	// Set up result struct
	avs_sample_result_t result = {};
	result.x = x;
	result.y = y;
	result.z = z;

	// Set up first iteration from root data
	avs_node_t* node = &avs->nodes[avs->root_index];
	float side = avs->root_side;
	float half_side = side * 0.5f;

	// Loop down until we find the leaf that contains the position
	while(avs_node_has_children(node))
	{
		// Fid the octant that contains the position
		int id = 0;
		id |= (x >= half_side) ? 1 : 0;
		id |= (y >= half_side) ? 2 : 0;
		id |= (z >= half_side) ? 4 : 0;

		// And make that out current node
		node = &avs->nodes[node->child_id[id]];
		side = half_side;
		half_side = half_side * 0.5f;

		// Modify position to be in local space of the new node
		x = (id & 1) ? x - half_side : x;
		y = (id & 2) ? y - half_side : y;
		z = (id & 4) ? z - half_side : z;
	}

	// Sample the field
	avs_brick_t* brick = &avs->bricks[node->brick_id];
	uint32_t ix = static_cast<uint32_t>(x);
	uint32_t iy = static_cast<uint32_t>(y);
	uint32_t iz = static_cast<uint32_t>(z);
	// TODO: Don't use nearest
	result.field_val = brick->data[ix + iy * AVS_BRICK_SIDE + iz * AVS_BRICK_SIDE * AVS_BRICK_SIDE];

	*out_result = result;
	return AVS_RESULT_OK;
}

struct avs_work_pair_t
{
	avs_index_t index;
	float x, y, z;
	float side;
};

void avs_paint_sphere(avs_t* avs, float center_x, float center_y, float center_z, float radius)
{
	std::vector<avs_work_pair_t> work_stack; // TODO: we could have an array of all used nodes, their size and position and just churn through it. For now though let's do it this way

	avs_work_pair_t root =
	{
		avs->root_index,
		0.0f, 0.0f, 0.0f,
		avs->root_side,
	};
	work_stack.push_back(root);

	while(work_stack.size() != 0)
	{
		avs_work_pair_t work = work_stack.back();
		work_stack.pop_back();

		avs_node_t* node = &avs->nodes[work.index];
		float side = work.side;
		float half_side = side * 0.5f;

		for(int i = 0; i < 8; ++i)
		{
			if(node->child_id[i] != AVS_INVALID_INDEX)
			{
				avs_work_pair_t child_work =
				{
					node->child_id[i],
					(i & 1) ? work.x - half_side : work.x,
					(i & 2) ? work.y - half_side : work.y,
					(i & 4) ? work.z - half_side : work.z,
					half_side,
				};
				work_stack.push_back(child_work);
			}
		}

		// Go over each sample in the brick and paint in the sphere
		avs_brick_t* brick = &avs->bricks[node->brick_id];
		uint32_t i = 0;
		for(uint32_t iz = 0; iz < AVS_BRICK_SIDE; ++iz)
		{
			float z = work.z + static_cast<float>(iz) * side / AVS_BRICK_SIDE; // TODO: do iteratively
			float dz = z - center_z;
			for(uint32_t iy = 0; iy < AVS_BRICK_SIDE; ++iy)
			{
				float y = work.y + static_cast<float>(iy) * side / AVS_BRICK_SIDE; // TODO: do iteratively
				float dy = y - center_y;
				for(uint32_t ix = 0; ix < AVS_BRICK_SIDE; ++ix)
				{
					float x = work.x + static_cast<float>(ix) * side / AVS_BRICK_SIDE; // TODO: do iteratively
					float dx = x - center_x;

					float val = sqrtf(dx*dx + dy*dy + dz*dz) - radius;
					float sat_val = fmax(avs->min_dist, fmin(avs->max_dist, val));

					brick->data[i] = fabs(sat_val) < fabs(brick->data[i]) ? sat_val : brick->data[i]; // TODO: is this really right?
					++i;
				}
			}
		}
	}
}
