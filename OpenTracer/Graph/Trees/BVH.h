#ifndef __BVH_H__
#define __BVH_H__

#include <string>
#include "../../Math/Numeric/Float4.h"
#include "../../Math/Shapes/AABB.h"
#include "../../Util/Config.h"
#include "../../Math/Shapes/Triangle.h"
#include "../../Scene.h"

namespace OpenTracerCore
{
	class BVH
	{
	protected:
		class __declspec(align(16)) BVHPrimInfo
		{
		public:
			float4 mCentroid;
			AABB mBounds;
			int mPrimitiveID;

			BVHPrimInfo(int id, const AABB& b)
			{
				mPrimitiveID = id;
				mBounds = b;
				mCentroid = mBounds.mMin * 0.5f + mBounds.mMax * 0.5f;
			}
		};

		class __declspec(align(16)) BVHNode
		{
		public:
			AABB mBounds;
			BVHNode* mChildren[2];
			unsigned int mSplitAxis, mPrimitiveOffset, mPrimitiveCount;

			void InitLeaf(unsigned int first, unsigned int n, const AABB &b)
			{
				mPrimitiveOffset = first;
				mPrimitiveCount = n;
				mBounds = b;
			}

			void InitInterior(unsigned int axis, BVHNode* child0, BVHNode* child1)
			{
				mChildren[0] = child0;
				mChildren[1] = child1;
				mBounds = AABB();
				mBounds.Union(child0->mBounds);
				mBounds.Union(child1->mBounds);
				mSplitAxis = axis;
				mPrimitiveCount = 0;
			}
		};

		class __declspec(align(16)) LBVHNode
		{
		public:
			unsigned int mPrimitiveOffset;
			unsigned int mPrimitiveCount;
			unsigned int mAxis;
			unsigned int mLeaf;
			float4 mLXY;		// Left child float4(min0.x, max0.x, min0.y, max0.y)
			float4 mRXY;		// Right child float4(min1.x, max1.x, min1.y, max1.y)
			float4 mLRZ;		// Both children float4(min0.z, max0.z, min1.z, max1.z)
		};

		unsigned int mMaxPrimsInNode;
		unsigned int mSahBuckets;

		LBVHNode* mNodes;
		unsigned int mNodeCount;

		unsigned int* mIndices;
		unsigned int mIndexCount;

	public:
		BVH(const std::string& config, Scene* scene);
		~BVH();

		float* GetNodes() { return (float*)mNodes; }
		size_t GetNodeCount() { return mNodeCount; }

		unsigned int* GetIndices() { return mIndices; }
		size_t GetIndexCount() { return mIndexCount; }

		void* operator new(size_t size)
		{
			return _aligned_malloc(size, 16);
		}

			void* operator new[](size_t size)
		{
			return _aligned_malloc(size, 16);
		}

			void operator delete(void* ptr)
		{
			_aligned_free(ptr);
		}

		void operator delete[](void* ptr)
		{
			_aligned_free(ptr);
		}
	};
	/*
	class __attribute__((aligned(16))) CBVH
	{
	public:
		unsigned int max_prims_in_node;
		unsigned int sah_buckets;

		CLBVHNode* nodes;
		unsigned int nodes_count;

		unsigned int* indexes;
		unsigned int indexes_count;

		CBVH(unsigned int max_prims = 8, unsigned int buckets = 16);
		~CBVH();

		void BuildTree(CTriangle* prims, unsigned int prims_count);
		CBVHNode* RecursiveBuild(vector<CBVHPrimInfo> &build_data, unsigned int start, unsigned int end, unsigned int* total_nodes, vector<unsigned int> &ordered_prim_ids);
		unsigned int Linearize(CBVHNode* n, unsigned int* offset);

		int Intersect(CTriangle* prims, const CRay &ray, float4 &bary, float &depth, int &id);
		float4 IntersectPacket(CTriangle* prims, const CRayPacket &ray, mat4 &bary, float4 &depth, int &id);
	};*/
}

#endif