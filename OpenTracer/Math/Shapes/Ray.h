#ifndef _RAY_H
#define _RAY_H

#include "../Numeric/Float4.h"

namespace OpenTracerCore
{
	class Intersection;

	class __declspec(align(16)) Ray
	{
	private:
		float4 mOrigin;
		float4 mDirection;
		float4 mInverse;

	public:
		Ray(const float4& new_orig, const float4& new_dir)
		{
			this->mOrigin = new_orig;
			this->mDirection = normalize(new_dir);
			this->mInverse = rcp(this->mDirection);
		}

		friend class Intersection;
	};
}

#endif