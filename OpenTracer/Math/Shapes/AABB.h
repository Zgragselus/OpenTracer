#ifndef __AABB_H__
#define __AABB_H__

#include "../Numeric/Float4.h"

namespace OpenTracerCore
{
	class Intersection;

	class __declspec(align(16)) AABB
	{
	public:
		float4 mMin;
		float4 mMax;

		AABB()
		{
			this->mMin = float4(1e30f, 1e30f, 1e30f, 0.0f);
			this->mMax = float4(-1e30f, -1e30f, -1e30f, 0.0f);
		}

		AABB(const float4& p)
		{
			this->mMin = p;
			this->mMax = p;
		}

		AABB(const float4& p, const float4& q)
		{
			this->mMin = f4min(p, q);
			this->mMax = f4max(p, q);
		}

		void Union(const float4& p)
		{
			this->mMin = f4min(this->mMin, p);
			this->mMax = f4max(this->mMax, p);
		}

		void Union(const AABB& b)
		{
			this->mMin = f4min(this->mMin, b.mMin);
			this->mMax = f4max(this->mMax, b.mMax);
		}

		float GetSurfaceArea() const
		{
			float4 d = this->mMax - this->mMin;
			const float4 d_s = hadd(float4(d.x, d.x, d.y, d.w) * float4(d.y, d.z, d.z, d.w));
			return d_s.x;
		}

		void Expand(float f)
		{
			this->mMin -= float4(f, f, f, 0.0f);
			this->mMax += float4(f, f, f, 0.0f);
		}

		int GetLongestAxis()
		{
			const float4 d = this->mMax - this->mMin;

			if (d.x > d.y && d.x > d.z)
			{
				return 0;
			}
			else if (d.y > d.z)
			{
				return 1;
			}
			else
			{
				return 2;
			}
		}

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

		friend class Intersection;
	};
}

#endif