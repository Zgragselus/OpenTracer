#ifndef __SPATIAL_AGGREGATE__H__
#define __SPATIAL_AGGREGATE__H__

#include "Aggregate.h"
#include "../Graph/Trees/KDTree.h"

namespace OpenTracerCore
{
	class Spatial : public Aggregate
	{
	protected:
		KDTree* mTree;
		cl::Buffer* mNodes;
		cl::Buffer* mIndices;

	public:
		Spatial(Context* context, Scene* scene, const std::string& config) : Aggregate(context, scene)
		{
			mTree = new KDTree(config, scene);
			mNodes = new cl::Buffer(context->GetContext(), CL_MEM_READ_ONLY, sizeof(unsigned int) * 2 * mTree->GetNodeCount());
			context->GetCommandQueue().enqueueWriteBuffer(*mNodes, CL_TRUE, 0, sizeof(unsigned int) * 2 * mTree->GetNodeCount(), mTree->GetNodes());
			mIndices = new cl::Buffer(context->GetContext(), CL_MEM_READ_ONLY, sizeof(unsigned int) * mTree->GetIndexCount());
			context->GetCommandQueue().enqueueWriteBuffer(*mIndices, CL_TRUE, 0, sizeof(unsigned int) * mTree->GetIndexCount(), mTree->GetIndices());
		}

		virtual ~Spatial()
		{
			delete mNodes;
			delete mIndices;
			delete mTree;
		}

		AABB& GetBounds()
		{
			return mTree->GetAABB();
		}

		cl::Buffer* GetNodes()
		{
			return mNodes;
		}

		cl::Buffer* GetIndices()
		{
			return mIndices;
		}
	};
}

#endif