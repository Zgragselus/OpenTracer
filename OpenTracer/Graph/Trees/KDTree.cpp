#include "KDTree.h"

using namespace OpenTracerCore;

KDTree::KDTree(const std::string& config, Scene* scene)
{
	Config* cfg = new Config(config);
	mMaxPrimsInNode = cfg->Get<int>("KDTree.MaxPrimsInNode");
	mMaxRecursionDepth = cfg->Get<int>("KDTree.MaxRecursionDepth");
	mSahTraversalCost = cfg->Get<int>("KDTree.SAH.TraversalCost");
	mSahIsectCost = cfg->Get<int>("KDTree.SAH.IntersectCost");
	mSahEmptyBonus = cfg->Get<float>("KDTree.SAH.EmptyBonus");
	delete cfg;

	std::cout << "Building KD-Tree using SAH algorithm..." << std::endl;

	BuildTree(scene->GetGeometryCPU(), scene->GetTriangleCount());

	std::cout << "Statistics:" << std::endl;
	std::cout << "\tTotal number of primitives: " << this->mIndices_next << std::endl;
	std::cout << "\tTotal number of nodes: " << this->mNodes_next << std::endl;
}

KDTree::~KDTree()
{
	if (this->mNodes)
	{
		free(this->mNodes);
		this->mNodes = NULL;
	}
	this->mNodes_alloc = 0;
	this->mNodes_next = 0;

	if (this->mIndices)
	{
		free(this->mIndices);
		this->mIndices = NULL;
	}
	this->mIndices_alloc = 0;
	this->mIndices_next = 0;

	this->mBounds = AABB();
}

void KDTree::BuildTree(Triangle* prims, unsigned int prims_count)
{
	this->mMaxRecursionDepth = this->mMaxRecursionDepth == 0 ? EstimateRecursionDepth(prims_count) : this->mMaxRecursionDepth;

	AABB* prims_bounds = (AABB*)_aligned_malloc(sizeof(AABB) * prims_count, 16);
	for (unsigned int i = 0; i < prims_count; i++)
	{
		prims_bounds[i] = prims[i].GetBounds();
		this->mBounds.Union(prims_bounds[i]);
	}

	BoundEdge* prims_bound_edges[3];
	for (unsigned int i = 0; i < 3; i++)
	{
		prims_bound_edges[i] = (BoundEdge*)malloc(sizeof(BoundEdge) * prims_count * 2);
	}

	this->mNodes_alloc = 0;
	this->mNodes_next = 0;

	this->mIndices_alloc = 0;
	this->mIndices_next = 0;

	unsigned int *prims_ids = (unsigned int*)malloc(sizeof(unsigned int) * prims_count);

	for (unsigned int i = 0; i < prims_count; i++)
	{
		prims_ids[i] = i;
	}

	this->RecursiveBuild(0, prims, prims_bounds, prims_bound_edges, prims_count, &this->mBounds, prims_ids, prims_count, 0, 0);

	free(prims_ids);
	prims_ids = NULL;

	for (unsigned int i = 0; i < 3; i++)
	{
		free(prims_bound_edges[i]);
		prims_bound_edges[i] = NULL;
	}

	_aligned_free(prims_bounds);
	prims_bounds = NULL;
}

void KDTree::RecursiveBuild(unsigned int node,
	Triangle* prims,
	AABB* prims_bounds,
	BoundEdge* prims_bound_edges[3],
	unsigned int prims_count,
	AABB* node_bounds,
	unsigned int* current_ids,
	unsigned int current_count,
	unsigned int bad_refines,
	unsigned int recursion_depth)
{
	if (node != this->mNodes_next)
	{
		std::cout << "Error: Something strange happened in KdTree builder" << std::endl;
		return;
	}

	if (node == this->mNodes_alloc)
	{
		int alloc_count = this->mNodes_alloc > 256 ? this->mNodes_alloc * 2 : 512;

		KdNode* nodes_realloc = (KdNode*)malloc(sizeof(KdNode) * alloc_count);

		if (this->mNodes_alloc > 0)
		{
			memcpy(nodes_realloc, this->mNodes, sizeof(KdNode) * this->mNodes_alloc);
			free(this->mNodes);
		}

		this->mNodes = nodes_realloc;
		this->mNodes_alloc = alloc_count;
	}

	this->mNodes_next++;

	if (current_count <= this->mMaxPrimsInNode || recursion_depth > this->mMaxRecursionDepth)
	{
		unsigned int base_index = this->mIndices_next;

		int alloc_count = this->mIndices_alloc;
		while (this->mIndices_next + current_count >= (unsigned int)alloc_count)
		{
			alloc_count = alloc_count > 256 ? alloc_count * 2 : 512;
		}

		unsigned int* indexes_realloc = (unsigned int*)malloc(sizeof(unsigned int) * alloc_count);

		if (this->mIndices_alloc > 0)
		{
			memcpy(indexes_realloc, this->mIndices, sizeof(unsigned int) * this->mIndices_alloc);
			free(this->mIndices);
		}

		this->mIndices = indexes_realloc;
		this->mIndices_alloc = alloc_count;

		for (unsigned int i = 0; i < current_count; i++)
		{
			this->mIndices[base_index + i] = current_ids[i];
			this->mIndices_next++;
		}

		this->mNodes[node].InitLeaf(base_index, current_count);

		return;
	}

	int best_axis = -1;
	int best_offset = -1;
	float best_cost = 1e30f;
	float old_cost = (float)(this->mSahIsectCost * current_count);
	float4 d = node_bounds->mMax - node_bounds->mMin;
	float total_sa = (2.0f * (d.x * d.y + d.x * d.z + d.y * d.z));
	float inv_total_sa = 1.0f / total_sa;

	int axis = node_bounds->GetLongestAxis();
	int retries = 0;

RetrySplit:
	for (unsigned int i = 0; i < current_count; i++)
	{
		int prim_number = current_ids[i];
		const AABB& bbox = prims_bounds[prim_number];
		prims_bound_edges[axis][2 * i] = BoundEdge(bbox.mMin[axis], prim_number, true);
		prims_bound_edges[axis][2 * i + 1] = BoundEdge(bbox.mMax[axis], prim_number, false);
	}
	std::sort(&prims_bound_edges[axis][0], &prims_bound_edges[axis][2 * current_count]);

	int below = 0, above = current_count;
	for (unsigned int i = 0; i < 2 * current_count; i++)
	{
		if (prims_bound_edges[axis][i].type == BoundEdge::END)
		{
			above--;
		}

		float edge_t = prims_bound_edges[axis][i].mPosition;

		if (edge_t > node_bounds->mMin[axis] && edge_t < node_bounds->mMax[axis])
		{
			int other_axis[3][2] = { { 1, 2 }, { 0, 2 }, { 0, 1 } };
			int other_axis0 = other_axis[axis][0];
			int other_axis1 = other_axis[axis][1];

			float below_sa = 2 * (d[other_axis0] * d[other_axis1] + (edge_t - node_bounds->mMin[axis]) * (d[other_axis0] + d[other_axis1]));
			float above_sa = 2 * (d[other_axis0] * d[other_axis1] + (node_bounds->mMax[axis] - edge_t) * (d[other_axis0] + d[other_axis1]));

			float below_p = below_sa * inv_total_sa;
			float above_p = above_sa * inv_total_sa;

			float eb = (above == 0 || below == 0) ? this->mSahEmptyBonus : 0.0f;
			float cost = this->mSahTraversalCost + this->mSahIsectCost * (1.0f - eb) * (below_p * below + above_p * above);

			if (cost < best_cost)
			{
				best_cost = cost;
				best_axis = axis;
				best_offset = i;
			}
		}

		if (prims_bound_edges[axis][i].type == BoundEdge::START)
		{
			below++;
		}
	}

	if (best_axis == -1 && retries < 2)
	{
		retries++;
		axis = (axis + 1) % 3;
		goto RetrySplit;
	}

	if (best_cost > old_cost)
	{
		bad_refines++;
	}

	if ((best_cost > 4.0f * old_cost && current_count < this->mMaxPrimsInNode) || best_axis == -1 || bad_refines == 3 || best_offset == -1)
	{
		unsigned int base_index = this->mIndices_next;

		int alloc_count = this->mIndices_alloc;
		while (this->mIndices_next + current_count >= (unsigned int)alloc_count)
		{
			alloc_count = alloc_count > 256 ? alloc_count * 2 : 512;
		}

		unsigned int* indexes_realloc = (unsigned int*)malloc(sizeof(unsigned int) * alloc_count);

		if (this->mIndices_alloc > 0)
		{
			memcpy(indexes_realloc, this->mIndices, sizeof(unsigned int) * this->mIndices_alloc);
			free(this->mIndices);
		}

		this->mIndices = indexes_realloc;
		this->mIndices_alloc = alloc_count;

		for (unsigned int i = 0; i < current_count; i++)
		{
			this->mIndices[base_index + i] = current_ids[i];
			this->mIndices_next++;
		}

		this->mNodes[node].InitLeaf(base_index, current_count);

		return;
	}

	axis = best_axis;
	float split = prims_bound_edges[axis][best_offset].mPosition;

	unsigned int* left = (unsigned int*)malloc(sizeof(unsigned int) * current_count);
	unsigned int left_count = 0;

	unsigned int* right = (unsigned int*)malloc(sizeof(unsigned int) * current_count);
	unsigned int right_count = 0;

	for (unsigned int i = 0; i < (unsigned int)best_offset; i++)
	{
		if (prims_bound_edges[axis][i].type == BoundEdge::START)
		{
			left[left_count] = prims_bound_edges[axis][i].mPrimitiveID;
			left_count++;
		}
	}

	for (unsigned int i = (unsigned int)best_offset + 1; i < 2 * current_count; i++)
	{
		if (prims_bound_edges[axis][i].type == BoundEdge::END)
		{
			right[right_count] = prims_bound_edges[axis][i].mPrimitiveID;
			right_count++;
		}
	}

	AABB left_bounds = *node_bounds;
	AABB right_bounds = *node_bounds;

	left_bounds.mMax[axis] = split;
	right_bounds.mMin[axis] = split;

	this->RecursiveBuild(node + 1,
		prims,
		prims_bounds,
		prims_bound_edges,
		prims_count,
		&left_bounds,
		left,
		left_count,
		bad_refines,
		recursion_depth + 1);

	unsigned int above_child = this->mNodes_next;
	this->mNodes[node].InitInterior(axis, above_child, split);

	this->RecursiveBuild(above_child,
		prims,
		prims_bounds,
		prims_bound_edges,
		prims_count,
		&right_bounds,
		right,
		right_count,
		bad_refines,
		recursion_depth + 1);

	free(left);
	left = NULL;

	free(right);
	right = NULL;
}