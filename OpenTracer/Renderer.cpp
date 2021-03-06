#include "Renderer.h"
#include <sstream>
#include <fstream>
#include <string>
#include <utility>
#include <iostream>
#include <chrono>

using namespace OpenTracerCore;

cl::Program* Renderer::mProgram = NULL;
cl::Kernel* Renderer::mKernelNaive = NULL;
cl::Kernel* Renderer::mKernelSpatial = NULL;

Renderer::Renderer(Context* context)
{
	mContext = context;

	if (!mProgram)
	{
		std::ifstream sf("C:\\Programming\\OpenTracer\\OpenTracer\\Renderer.cl");
		std::string s(std::istreambuf_iterator<char>(sf), (std::istreambuf_iterator<char>()));
		cl::Program::Sources source = cl::Program::Sources(1, std::make_pair(s.c_str(), s.length() + 1));
		mProgram = new cl::Program(mContext->GetContext(), source);
		mProgram->build(mContext->GetDevices(), "-cl-mad-enable -cl-unsafe-math-optimizations -cl-finite-math-only -cl-fast-relaxed-math");
		std::cout << mProgram->getBuildInfo<CL_PROGRAM_BUILD_LOG>(mContext->GetDevices()[0]) << std::endl;
		mKernelNaive = new cl::Kernel(*mProgram, "TraceNaive");
		mKernelSpatial = new cl::Kernel(*mProgram, "TraceSpatial");

		size_t binarySize;
		mProgram->getInfo(CL_PROGRAM_BINARY_SIZES, &binarySize);
		unsigned char *bin = (unsigned char *)malloc(binarySize);
		mProgram->getInfo(CL_PROGRAM_BINARIES, &bin);
		FILE* fp;
		fopen_s(&fp, "renderer.cl_asm", "wb");
		fwrite(bin, sizeof(char), binarySize, fp);
		fclose(fp);
		free(bin);
	}
}

Renderer::~Renderer()
{

}

void Renderer::Render(Scene* scene, Aggregate* naive, RayBuffer* rayBuffer, Texture* output)
{
	size_t raysCount = output->GetWidth() * output->GetHeight();
	size_t trisCount = naive->GetTriangleCount();

	mKernelNaive->setArg(0, *naive->GetTriangles());
	mKernelNaive->setArg(1, *rayBuffer->GetRayBuffer());
	mKernelNaive->setArg(2, *output->GetDeviceData());
	mKernelNaive->setArg(3, trisCount);
	mKernelNaive->setArg(4, raysCount);

	mContext->GetCommandQueue().enqueueNDRangeKernel(*mKernelNaive, cl::NullRange, cl::NDRange(raysCount));
}

void Renderer::Render(Scene* scene, Spatial* spatial, RayBuffer* rayBuffer, Texture* output)
{
	size_t raysCount = output->GetWidth() * output->GetHeight();
	size_t trisCount = spatial->GetTriangleCount();
	cl_float4 pmin, pmax;
	pmin.s[0] = spatial->GetBounds().mMin.x; pmin.s[1] = spatial->GetBounds().mMin.y; pmin.s[2] = spatial->GetBounds().mMin.z; pmin.s[3] = spatial->GetBounds().mMin.w;
	pmax.s[0] = spatial->GetBounds().mMax.x; pmax.s[1] = spatial->GetBounds().mMax.y; pmax.s[2] = spatial->GetBounds().mMax.z; pmax.s[3] = spatial->GetBounds().mMax.w;

	cl_int2 dimensions;
	dimensions.s[0] = output->GetWidth();
	dimensions.s[1] = output->GetHeight();

	mKernelSpatial->setArg(0, *spatial->GetTriangles());
	mKernelSpatial->setArg(1, *rayBuffer->GetRayBuffer());
	mKernelSpatial->setArg(2, *output->GetDeviceData());
	mKernelSpatial->setArg(3, *spatial->GetNodes());
	mKernelSpatial->setArg(4, *spatial->GetIndices());
	mKernelSpatial->setArg(5, pmin);
	mKernelSpatial->setArg(6, pmax);
	mKernelSpatial->setArg(7, trisCount);
	mKernelSpatial->setArg(8, raysCount);
	mKernelSpatial->setArg(9, dimensions);

	cl::Event evt;
	//mContext->GetCommandQueue().enqueueNDRangeKernel(*mKernelSpatial, cl::NullRange, cl::NDRange(output->GetWidth(), output->GetHeight()), cl::NDRange(8, 8), 0, &evt);
	mContext->GetCommandQueue().enqueueNDRangeKernel(*mKernelSpatial, cl::NullRange, cl::NDRange(output->GetWidth(), output->GetHeight()), cl::NullRange, 0, &evt);
	evt.wait();
}