#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "Math/Numeric/Float4.h"
#include "Context.h"
#include "OpenTracerDll.h"

namespace OpenTracerCore
{
	class Texture
	{
	private:
		Context*	mContext;
		float4*		mData;
		cl::Buffer* mDeviceData;
		size_t		mWidth;
		size_t		mHeight;

		static cl::Program* mProgram;
		static cl::Kernel* mKernel;

	public:
					Texture(Context* context, size_t width, size_t height);
					~Texture();
		void		ClearColor(float r, float g, float b, float a);
		void		Resize(size_t width, size_t height);
		size_t		GetWidth() { return mWidth; }
		size_t		GetHeight() { return mHeight; }
		void		SetData(cl::Buffer* data)
		{
			mContext->GetCommandQueue().enqueueCopyBuffer(*data, *mDeviceData, 0, 0, sizeof(float4) * mWidth * mHeight);
		}
		void*		GetData();
		cl::Buffer*	GetDeviceData()	{ return mDeviceData; }
	};
}

#endif