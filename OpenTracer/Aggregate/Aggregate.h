#ifndef __NAIVE_AGGREGATE__H__
#define __NAIVE_AGGREGATE__H__

#include "../Scene.h"
#include "../Math/Numeric/Mat4.h"

namespace OpenTracerCore
{
	class Aggregate
	{
	protected:
		cl::Buffer* mWoop;
		size_t mWoopCount;

		void Woopify(float4* input, float4* output)
		{
			float4& v0 = input[0];
			float4& v1 = input[1];
			float4& v2 = input[2];
			float4 a = v0 - v2;
			float4 b = v1 - v2;
			float4 c = cross(a, b);

			mat4 m = inverse(mat4(a.x, b.x, c.x, v2.x,
				a.y, b.y, c.y, v2.y,
				a.z, b.z, c.z, v2.z,
				0.0f, 0.0f, 0.0f, 1.0f));

			output[0] = m[2] * float4(1.0f, 1.0f, 1.0f, -1.0f);
			output[1] = m[0];
			output[2] = m[1];
		}

	public:
		Aggregate(Context* context, Scene* scene)
		{
			mWoop = new cl::Buffer(context->GetContext(), CL_MEM_READ_ONLY, sizeof(float4) * 3 * scene->GetTriangleCount());

			float4* input = (float4*)scene->GetGeometryCPU();
			float4* output = new float4[scene->GetVertexCount()];
			for (int i = 0; i < scene->GetVertexCount(); i += 3)
			{
				Woopify(input + i, output + i);
			}
			mWoopCount = scene->GetTriangleCount();
			context->GetCommandQueue().enqueueWriteBuffer(*mWoop, CL_TRUE, 0, sizeof(float4) * scene->GetVertexCount(), output);
			delete[] output;
		}
		
		virtual ~Aggregate()
		{
			delete mWoop;
		}

		cl::Buffer* GetTriangles() { return mWoop; }

		size_t GetTriangleCount() { return mWoopCount; }
	};
}

#endif