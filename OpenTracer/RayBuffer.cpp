#include "RayBuffer.h"
#include <sstream>
#include <fstream>
#include <string>
#include <utility>
#include <iostream>

using namespace OpenTracerCore;

cl::Program* RayBuffer::mProgram = NULL;
cl::Kernel* RayBuffer::mKernel = NULL;

RayBuffer::RayBuffer(Context* context)
{
	mContext = context;

	if (!mProgram)
	{
		std::ifstream sf("C:\\Programming\\OpenTracer\\OpenTracer\\RayBuffer.cl");
		std::string s(std::istreambuf_iterator<char>(sf), (std::istreambuf_iterator<char>()));
		cl::Program::Sources source = cl::Program::Sources(1, std::make_pair(s.c_str(), s.length() + 1));
		mProgram = new cl::Program(mContext->GetContext(), source);
		mProgram->build(mContext->GetDevices(), "-cl-mad-enable -cl-unsafe-math-optimizations -cl-finite-math-only -cl-fast-relaxed-math");
		std::cout << mProgram->getBuildInfo<CL_PROGRAM_BUILD_LOG>(mContext->GetDevices()[0]) << std::endl;
		mKernel = new cl::Kernel(*mProgram, "Primary");
	}
}

RayBuffer::~RayBuffer()
{
	delete mDeviceData;
}

void RayBuffer::SetCamera(const float4& position, const float4& target, const float4& up, float aspect, float fov, int width, int height, float nearPlane, float farPlane)
{
	if (width != (int)mWidth || height != (int)mHeight)
	{
		if (mDeviceData)
		{
			delete mDeviceData;
		}
		mDeviceData = new cl::Buffer(mContext->GetContext(), CL_MEM_READ_WRITE, width * height * sizeof(float4) * 2);
	}

	mOrigin = position;
	mForward = normalize(target - position);
	mRight = normalize(cross(mForward, up));
	mUp = normalize(cross(mRight, mForward));
	mFov = fov;
	mAspect = aspect;
	mWidth = (float)width;
	mHeight = (float)height;
	mNear = nearPlane;
	mFar = farPlane;
}

void RayBuffer::GeneratePrimary()
{
	size_t items = (int)mWidth * (int)mHeight;
	cl_float2 halfDim = { mWidth * 0.5f, mHeight * 0.5f };
	cl_float2 invHalfDim = { 1.0f / halfDim.s[0], 1.0f / halfDim.s[1] };
	cl_int2 dim = { (int)mWidth, (int)mHeight };
	cl_float4 right = { mRight.x, mRight.y, mRight.z, 0.0f };
	cl_float4 up = { mUp.x, mUp.y, mUp.z, 0.0f };
	float planePos = (float)halfDim.s[1] / tanf(mFov * 0.5f * 3.141592654f / 180.0f);
	cl_float4 forward = { mForward.x * planePos, mForward.y * planePos, mForward.z * planePos, 0.0f };
	cl_float4 origin = { mOrigin.x, mOrigin.y, mOrigin.z, mOrigin.w };

	mKernel->setArg(0, *mDeviceData);
	mKernel->setArg(1, origin);
	mKernel->setArg(2, forward);
	mKernel->setArg(3, right);
	mKernel->setArg(4, up);
	mKernel->setArg(5, halfDim);
	mKernel->setArg(6, invHalfDim);
	mKernel->setArg(7, mNear);
	mKernel->setArg(8, mFar);
	mKernel->setArg(9, mAspect);
	mKernel->setArg(10, dim);
	
	mContext->GetCommandQueue().enqueueNDRangeKernel(*mKernel, cl::NullRange, cl::NDRange(dim.s[0], dim.s[1]));
}