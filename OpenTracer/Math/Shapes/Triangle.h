#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "../Numeric/Float4.h"
#include "../Numeric/Mat4.h"
#include "AABB.h"

namespace OpenTracerCore
{
	class Intersection;

	class __declspec(align(16)) Triangle
	{
	private:
		float4 a, b, c;

	public:	
		Triangle()
		{

		}

		Triangle(const float4& aa, const float4& bb, const float4& cc)
		{
			this->a = aa;
			this->b = bb;
			this->c = cc;
		}

		AABB GetBounds() const
		{
			AABB r = AABB(this->a, this->b);
			r.Union(this->c);

			return r;
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