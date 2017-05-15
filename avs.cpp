#include "avs.h"

#include <cassert>
#include <vector>
#include <stdint.h>
#include <float.h>

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
