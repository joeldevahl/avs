#include "avs_internal.h"

static bool avs_node_has_children(avs_node_t* node)
{
	assert(node != nullptr);
	bool res = false;
	for(int i = 0; i < 8; ++i)
		res |= node->child_id[i] != AVS_INVALID_INDEX;
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

static avs_index_t avs_alloc_node(avs_t* avs)
{
	if (avs->free_nodes.size() == 0)
	{
		size_t new_size = avs->node_pool_size > 0 ? avs->node_pool_size * 2 : 1024;
		avs->nodes.resize(new_size);
		for (size_t n = 0; n < new_size; ++n)
			avs->free_nodes.push_back(n);
		avs->node_pool_size = new_size;
	}

	avs_index_t index = avs->free_nodes.back();
	avs->free_nodes.pop_back();

	return index;
}

static void avs_free_node(avs_t* avs, avs_index_t index)
{
	avs->free_nodes.push_back(index);
}

static avs_node_t* avs_get_node(avs_t* avs, avs_index_t index)
{
	return &avs->nodes[index];
}

static void avs_node_init(avs_t* avs, avs_node_t* node)
{
	for(int i = 0; i < 8; ++i)
		node->child_id[i] = AVS_INVALID_INDEX;
	node->brick_id = AVS_INVALID_INDEX;
}

static avs_index_t avs_alloc_brick(avs_t* avs)
{
	if (avs->free_bricks.size() == 0)
	{
		size_t new_size = avs->brick_pool_size > 0 ? avs->brick_pool_size * 2 : 1024;
		avs->bricks.resize(new_size);
		for (size_t n = 0; n < new_size; ++n)
			avs->free_bricks.push_back(n);
		avs->brick_pool_size = new_size;
	}
	avs_index_t index = avs->free_bricks.back();
	avs->free_bricks.pop_back();
	return index;
}

static void avs_free_brick(avs_t* avs, avs_index_t index)
{
	avs->free_bricks.push_back(index);
}

static avs_brick_t* avs_get_brick(avs_t* avs, avs_index_t index)
{
	return &avs->bricks[index];
}

static void avs_brick_init(avs_t* avs, avs_brick_t* brick)
{
	for(int i = 0; i < AVS_BRICK_SIZE; ++i)
		brick->data[i] = FLT_MAX;
}

avs_result_t avs_create(const avs_create_info_t* create_info, avs_t** out_avs)
{
	avs_t* avs = new avs_t;

	avs->root_index = avs_alloc_node(avs);
	avs_node_t* node = avs_get_node(avs, avs->root_index);
	avs_node_init(avs, node);

	node->brick_id = avs_alloc_brick(avs);
	avs_brick_t* brick = avs_get_brick(avs, node->brick_id);
	avs_brick_init(avs, brick);

	avs->root_origin.x = create_info->root_x;
	avs->root_origin.y = create_info->root_y;
	avs->root_origin.z = create_info->root_z;
	avs->root_side = create_info->root_side;

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
		// Did the octant that contains the position
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

	uint32_t ix = static_cast<uint32_t>(x / AVS_BRICK_SIDE);
	uint32_t iy = static_cast<uint32_t>(y / AVS_BRICK_SIDE);
	uint32_t iz = static_cast<uint32_t>(z / AVS_BRICK_SIDE);
	// TODO: Don't use nearest
	result.field_val = brick->data[ix + iy * AVS_BRICK_SIDE + iz * AVS_BRICK_SIDE * AVS_BRICK_SIDE];
	result.sample_step = side / AVS_BRICK_SIDE;

	*out_result = result;
	return AVS_RESULT_OK;
}

avs_result_t avs_trace_ray(avs_t* avs, float px, float py, float pz, float nx, float ny, float nz, float max_dist, avs_sample_result_t* out_result)
{
	// TODO: this is a super stupid implementation that just calls avs_sample point and traverses the whole tree every iteration!

	float nl = sqrtf(nx*nx + ny*ny + nz*nz);
	nx /= nl;
	ny /= nl;
	nz /= nl;

	float traced_dist = 0.0f;
	while(traced_dist < max_dist)
	{
		avs_sample_result_t result = {};
		if(avs_sample_point(avs, px, py, pz, &result) == AVS_RESULT_OK)
		{
			float surf_dist = fabs(result.field_val);
			if(surf_dist < result.sample_step) // TODO: stupid test here, we should iterate on a sub voxel level
			{
				*out_result = result;
				return AVS_RESULT_OK;
			}
			else
			{
				
				px += nx * surf_dist;
				py += ny * surf_dist;
				pz += nz * surf_dist;
				traced_dist += surf_dist;
			}
		}
		else
		{
			// We didn't hit anything, so just step somewhat distance
			// TODO: what distance is somewhat?
			px += nx;
			py += ny;
			pz += nz;
			traced_dist += 1.0f;
		}
	}

	return AVS_RESULT_OUTSIDE_FIELD;
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

	int64_t min_x = static_cast<int64_t>(floorf(center_x - radius));

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

					brick->data[i] = fabs(val) < fabs(brick->data[i]) ? val : brick->data[i]; // TODO: is this really right?
					++i;
				}
			}
		}
	}
}
