#include "Context.h"
#include <vector>

using namespace OpenTracerCore;

Context::Context(const ContextType& type)
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	cl_context_properties props[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)(platforms[0])(),
		0
	};
	mContext = cl::Context(type == ContextType::CONTEXT_CPU ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU, props);

	mDevices = mContext.getInfo<CL_CONTEXT_DEVICES>();

	mCommandQueue = cl::CommandQueue(mContext, mDevices[0]);
}

Context::~Context()
{

}