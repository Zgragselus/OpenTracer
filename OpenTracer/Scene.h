#ifndef __SCENE__H__
#define __SCENE__H__

#include "Context.h"
#include "Math/Numeric/Float4.h"
#include "Math/Shapes/Triangle.h"

namespace OpenTracerCore
{
	class Scene
	{
	private:
		Triangle* mGeometryCPU;
		cl::Buffer* mGeometryGPU;
		int mTrianglesCount;
		int mVerticesCount;

	public:
		Scene(Context* context, float* vertices, int count)
		{
			mVerticesCount = count;
			mTrianglesCount = count / 3;
			mGeometryCPU = new Triangle[mTrianglesCount];
			for (size_t i = 0; i < (size_t)mTrianglesCount; i++)
			{
				size_t a = 12 * i;
				size_t b = a + 4;
				size_t c = b + 4;
				mGeometryCPU[i] = Triangle(float4(vertices[a + 0], vertices[a + 1], vertices[a + 2], 1.0f),
					float4(vertices[b + 0], vertices[b + 1], vertices[b + 2], 1.0f),
					float4(vertices[c + 0], vertices[c + 1], vertices[c + 2], 1.0f));
			}
			mGeometryGPU = new cl::Buffer(context->GetContext(), CL_MEM_READ_ONLY, sizeof(float4) * 3 * mTrianglesCount);
			context->GetCommandQueue().enqueueWriteBuffer(*mGeometryGPU, CL_TRUE, 0, sizeof(float4) * 3 * mTrianglesCount, mGeometryCPU);
		}

		~Scene()
		{
			delete[] mGeometryCPU;
			delete mGeometryGPU;
		}

		Triangle* GetGeometryCPU() { return mGeometryCPU; }
		cl::Buffer* GetGeometryGPU() { return mGeometryGPU; }
		int GetVertexCount() { return mVerticesCount; }
		int GetTriangleCount() { return mTrianglesCount; }
	};
}

#endif