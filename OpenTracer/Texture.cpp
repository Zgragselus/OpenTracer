#include "Texture.h"
#include <sstream>
#include <fstream>
#include <string>
#include <utility>
#include <iostream>

using namespace OpenTracerCore;

cl::Program* Texture::mProgram = NULL;
cl::Kernel* Texture::mKernel = NULL;

Texture::Texture(Context* context, size_t width, size_t height)
{
	mContext = context;
	mWidth = width;
	mHeight = height;
	mData = new float4[width * height];
	mDeviceData = new cl::Buffer(mContext->GetContext(), CL_MEM_READ_WRITE, width * height * sizeof(float4));

	if (!mProgram)
	{
		std::ifstream sf("C:\\Programming\\OpenTracer\\OpenTracer\\Texture.cl");
		std::string s(std::istreambuf_iterator<char>(sf), (std::istreambuf_iterator<char>()));
		cl::Program::Sources source = cl::Program::Sources(1, std::make_pair(s.c_str(), s.length() + 1));
		mProgram = new cl::Program(mContext->GetContext(), source);
		mProgram->build(mContext->GetDevices(), "-cl-mad-enable -cl-unsafe-math-optimizations -cl-finite-math-only -cl-fast-relaxed-math");
		std::cout << mProgram->getBuildInfo<CL_PROGRAM_BUILD_LOG>(mContext->GetDevices()[0]) << std::endl;
		mKernel = new cl::Kernel(*mProgram, "ClearColor");
	}
}

Texture::~Texture()
{
	delete mDeviceData;
	delete[] mData;
}

void Texture::ClearColor(float r, float g, float b, float a)
{
	cl_float4 color;  color.s[0] = r; color.s[1] = g; color.s[2] = b; color.s[3] = a;
	size_t items = mWidth * mHeight;

	mKernel->setArg(0, *mDeviceData);
	mKernel->setArg(1, color);
	mKernel->setArg(2, items);
		
	mContext->GetCommandQueue().enqueueNDRangeKernel(*mKernel, cl::NullRange, cl::NDRange(items));
}

void Texture::Resize(size_t width, size_t height)
{
	delete[] mData;
	delete mDeviceData;

	mWidth = width;
	mHeight = height;
	mData = new float4[width * height];
	mDeviceData = new cl::Buffer(mContext->GetContext(), CL_MEM_READ_WRITE, width * height * sizeof(float4));
}

void* Texture::GetData()
{
	cl::Event evt;
	mContext->GetCommandQueue().enqueueReadBuffer(*mDeviceData, CL_TRUE, 0, mWidth * mHeight * sizeof(float4), mData, 0, &evt);
	evt.wait();
	return mData;
}