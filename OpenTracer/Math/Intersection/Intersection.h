#ifndef __INTERSECTION_H__
#define __INTERSECTION_H__

#include "../Shapes/Ray.h"
#include "../Shapes/AABB.h"
#include "../Shapes/Triangle.h"

namespace OpenTracerCore
{
	class Intersection
	{
		bool Intersect(const Triangle& t, const Ray& r, float4 &b, float &d) const
		{
			const float4 e1 = t.b - t.a;
			const float4 e2 = t.c - t.a;

			const float4 pvec = cross(r.mDirection, e2);

			float det = dot(e1, pvec);
			if(det > -0.001f && det < 0.001f)
				return false;

			float inv_det = 1.0f / det;
			const float4 tvec = r.mOrigin - t.a;

			b.x = dot(tvec, pvec) * inv_det;
			if(b.x < 0.0f || b.x > 1.0f)
				return false;

			const float4 qvec = cross(tvec, e1);
			b.y = dot(r.mDirection, qvec) * inv_det;
			if(b.y < 0.0f || b.x + b.y > 1.0f)
				return false;

			d = dot(e2, qvec) * inv_det;

			b.z = 1.0f - b.x - b.y;
			b.w = 0.0f;

			return true;
		}

		bool Intersect(const AABB& b, const Ray& r, float& in, float& out) const
		{
			const float4 v1 = (b.mMin - r.mOrigin) * r.mInverse;
			const float4 v2 = (b.mMax - r.mOrigin) * r.mInverse;

			const float4 near = f4min(v1, v2);
			const float4 far = f4max(v1, v2);

			const float enter = near.x > near.y && near.x > near.z ? near.x : near.y > near.z ? near.y : near.z;
			const float exit = far.x < far.y && far.x < far.z ? far.x : far.y < far.z ? far.y : far.z;

			in = enter;
			out = exit;

			return (exit > 0.0f && enter < exit);
		}

		bool Intersect(const AABB& b, const Ray &r) const
		{
			const float4 v1 = (b.mMin - r.mOrigin) * r.mInverse;
			const float4 v2 = (b.mMax - r.mOrigin) * r.mInverse;

			const float4 near = f4min(v1, v2);
			const float4 far = f4max(v1, v2);

			const float enter = near.x > near.y && near.x > near.z ? near.x : near.y > near.z ? near.y : near.z;
			const float exit = far.x < far.y && far.x < far.z ? far.x : far.y < far.z ? far.y : far.z;

			return (exit > 0.0f && enter < exit);
		}
	};
}

#endif