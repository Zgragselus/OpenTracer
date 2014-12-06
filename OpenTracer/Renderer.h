#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Texture.h"
#include "RayBuffer.h"
#include "Naive.h"

namespace OpenTracerCore
{
	class Renderer
	{
	private:
		static cl::Program* mProgram;
		static cl::Kernel* mKernel;
		Context* mContext;

	public:
		Renderer(Context* context);
		~Renderer();
		void Render(Scene* scene, Naive* naive, RayBuffer* rayBuffer, Texture* output);
	};
}

#endif