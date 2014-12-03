#pragma once

#include "OpenTracerDll.h"

namespace OpenTracer
{
	class Context
	{
	public:
		enum ContextType
		{
			CONTEXT_TYPE_CPU = 0,
			CONTEXT_TYPE_GPU
		};

		static Context& GetInstance()
		{
			static Context instance;
			return instance;
		}

		OPENTRACER_API void Initialize(const ContextType&);
		OPENTRACER_API void Release();

	private:
		Context() {}
		Context(const Context&);
		void operator=(const Context&);
	};

	class Renderer;

	class Texture
	{
	private:
		void* mData;

	public:
		OPENTRACER_API Texture(unsigned int width, unsigned int height);
		OPENTRACER_API ~Texture();
		OPENTRACER_API unsigned int GetWidth();
		OPENTRACER_API unsigned int GetHeight();
		OPENTRACER_API void* GetData();
		OPENTRACER_API void Resize(unsigned int width, unsigned int height);
		OPENTRACER_API void Clear(float red, float green, float blue, float alpha);

		friend class Renderer;
	};

	class RayGenerator
	{
	private:
		void* mData;

	public:
		OPENTRACER_API RayGenerator();
		OPENTRACER_API ~RayGenerator();
		OPENTRACER_API void GeneratePrimary(float posX, float posY, float posZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ, float aspect, float fov, int width, int height, float nearPlane, float farPlane);

		friend class Renderer;
	};

	class Renderer
	{
	public:
		OPENTRACER_API static void Render(Texture* output, RayGenerator* raygen);
	};
}