#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <string>
#include "../../Util/Config.h"
#include "../../Math/Shapes/AABB.h"
#include "../../Math/Shapes/Triangle.h"
#include "../../Scene.h"

namespace OpenTracerCore
{
	class KDTree
	{
	private:
		class BoundEdge
		{
		public:
			float mPosition;
			int mPrimitiveID;

			enum { START, END } type;

			BoundEdge(float _pos, int _prim, bool _starting)
			{
				this->mPosition = _pos;
				this->mPrimitiveID = _prim;
				this->type = _starting == true ? START : END;
			}

			bool operator<(const BoundEdge &edge) const
			{
				if (this->mPosition == edge.mPosition)
				{
					return (int)this->type < (int)edge.type;
				}
				else
				{
					return this->mPosition < edge.mPosition;
				}
			}
		};

		class KdNode
		{
		public:
			union
			{
				float mSplit;
				unsigned int mPrimitiveID;
			};

			union
			{
				unsigned int mFlags;
				unsigned int mPrimitiveCount;
				unsigned int mAboveChild;
			};

			void InitLeaf(unsigned int prim_idx, unsigned int num_prims)
			{
				this->mFlags = 3;
				this->mPrimitiveCount |= (num_prims << 2);

				if (num_prims == 0)
				{
					this->mPrimitiveID = 0;
				}
				else
				{
					this->mPrimitiveID = prim_idx;
				}
			}

			void InitInterior(unsigned int axis, unsigned int above_node, float split_pos)
			{
				this->mSplit = split_pos;
				this->mFlags = axis;
				this->mAboveChild |= (above_node << 2);
			}

			float GetSplitPosition() const
			{
				return this->mSplit;
			}

			unsigned int GetPrimitivesCount() const
			{
				return (this->mPrimitiveCount >> 2);
			}

			unsigned int GetSplitAxis() const
			{
				return (this->mFlags & 3);
			}

			bool IsLeaf() const
			{
				return (this->mFlags & 3) == 3;
			}

			unsigned int GetAboveChild() const
			{
				return (this->mAboveChild >> 2);
			}
		};

		AABB mBounds;

		KdNode* mNodes;
		size_t mNodes_alloc;
		size_t mNodes_next;

		unsigned int* mIndices;
		size_t mIndices_alloc;
		size_t mIndices_next;

		unsigned int mMaxPrimsInNode;
		unsigned int mMaxRecursionDepth;
		unsigned int mSahTraversalCost;
		unsigned int mSahIsectCost;
		float mSahEmptyBonus;

		unsigned int EstimateRecursionDepth(unsigned int prims_count)
		{
			return 8 + (int)(1.3f * log2(prims_count));
		}

		void BuildTree(Triangle* prims, unsigned int prims_count);

		void RecursiveBuild(unsigned int node,
			Triangle* prims,
			AABB* prims_bounds,
			BoundEdge* prims_bound_edges[3],
			unsigned int prims_count,
			AABB* node_bounds,
			unsigned int* current_ids,
			unsigned int current_count,
			unsigned int bad_refines,
			unsigned int recursion_depth);

	public:
		KDTree(const std::string& config, Scene* scene);

		~KDTree();

		AABB& GetAABB() { return mBounds; }

		float* GetNodes() { return (float*)mNodes; }
		size_t GetNodeCount() { return mNodes_next; }

		unsigned int* GetIndices() { return mIndices; }
		size_t GetIndexCount() { return mIndices_next; }

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
}

#endif