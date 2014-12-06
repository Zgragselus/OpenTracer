#include "Renderer.h"
#include <sstream>
#include <fstream>
#include <string>
#include <utility>
#include <iostream>

using namespace OpenTracerCore;

cl::Program* Renderer::mProgram = NULL;
cl::Kernel* Renderer::mKernel = NULL;

Renderer::Renderer(Context* context)
{
	mContext = context;

	if (!mProgram)
	{
		std::ifstream sf("C:\\Programming\\OpenTracer\\OpenTracer\\RendererNaive.cl");
		std::string s(std::istreambuf_iterator<char>(sf), (std::istreambuf_iterator<char>()));
		cl::Program::Sources source = cl::Program::Sources(1, std::make_pair(s.c_str(), s.length() + 1));
		mProgram = new cl::Program(mContext->GetContext(), source);
		mProgram->build(mContext->GetDevices());
		std::cout << mProgram->getBuildInfo<CL_PROGRAM_BUILD_LOG>(mContext->GetDevices()[0]) << std::endl;
		mKernel = new cl::Kernel(*mProgram, "Trace");
	}
}

Renderer::~Renderer()
{

}

void Renderer::Render(Scene* scene, Naive* naive, RayBuffer* rayBuffer, Texture* output)
{
	size_t raysCount = output->GetWidth() * output->GetHeight();
	size_t trisCount = naive->GetTriangleCount();

	mKernel->setArg(0, *scene->GetVertices());
	mKernel->setArg(1, *rayBuffer->GetRayBuffer());
	mKernel->setArg(2, *output->GetDeviceData());
	mKernel->setArg(3, trisCount);
	mKernel->setArg(4, raysCount);

	mContext->GetCommandQueue().enqueueNDRangeKernel(*mKernel, cl::NullRange, cl::NDRange(raysCount));
}