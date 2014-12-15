#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <cl/cl.hpp>

namespace OpenTracerCore
{
	class Context
	{
	public:
		enum ContextType
		{
			CONTEXT_CPU = 0,
			CONTEXT_GPU
		};

	private:
		ContextType mType;
		cl::Context mContext;
		cl::CommandQueue mCommandQueue;
		std::vector<cl::Device> mDevices;

	public:
		Context(const ContextType& type);
		~Context();
		cl::Context& GetContext() { return mContext; }
		cl::CommandQueue& GetCommandQueue() { return mCommandQueue; }
		std::vector<cl::Device>& GetDevices() { return mDevices; }
	};
}

#endif