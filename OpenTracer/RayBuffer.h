#ifndef __RAY_BUFFER__H__
#define __RAY_BUFFER__H__

#include "Context.h"
#include "Float4.h"

namespace OpenTracerCore
{
	class RayBuffer
	{
	private:
		float4 mOrigin;
		float4 mForward;
		float4 mRight;
		float4 mUp;
		float mFov;
		float mAspect;
		float mWidth;
		float mHeight;
		float mNear;
		float mFar;

		Context* mContext;
		
		cl::Buffer* mDeviceData;
		static cl::Program* mProgram;
		static cl::Kernel* mKernel;

	public:
		RayBuffer(Context* context);
		~RayBuffer();
		void SetCamera(const float4& position, const float4& target, const float4& up, float aspect, float fov, int width, int height, float nearPlane, float farPlane);
		void GeneratePrimary();
		cl::Buffer* GetRayBuffer() { return mDeviceData; }

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