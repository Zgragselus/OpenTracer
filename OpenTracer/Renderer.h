#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Texture.h"
#include "RayBuffer.h"
#include "Aggregate/Aggregate.h"
#include "Aggregate/Spatial.h"

namespace OpenTracerCore
{
	class Renderer
	{
	private:
		static cl::Program* mProgram;
		static cl::Kernel* mKernelNaive;
		static cl::Kernel* mKernelSpatial;
		Context* mContext;

	public:
		Renderer(Context* context);
		~Renderer();
		void Render(Scene* scene, Aggregate* naive, RayBuffer* rayBuffer, Texture* output);
		void Render(Scene* scene, Spatial* naive, RayBuffer* rayBuffer, Texture* output);
	};
}

#endif