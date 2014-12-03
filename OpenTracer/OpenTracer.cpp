#include "OpenTracer.h"

#include "Context.h"
#include "Texture.h"
#include "RayBuffer.h"
#include "Float4.h"

using namespace OpenTracer;

OpenTracerCore::Context* g_mContext;

void Context::Initialize(const ContextType& type)
{
	g_mContext = new OpenTracerCore::Context((OpenTracerCore::Context::ContextType)type);
}

void Context::Release()
{
	delete g_mContext;
}

Texture::Texture(unsigned int width, unsigned int height)
{
	OpenTracerCore::Texture* texture = new OpenTracerCore::Texture(g_mContext, width, height);
	mData = (void*)texture;
}

Texture::~Texture()
{
	delete ((OpenTracerCore::Texture*)mData);
}

unsigned int Texture::GetWidth()
{
	return ((OpenTracerCore::Texture*)mData)->GetWidth();
}

unsigned int Texture::GetHeight()
{
	return ((OpenTracerCore::Texture*)mData)->GetHeight();
}

void* Texture::GetData()
{
	return ((OpenTracerCore::Texture*)mData)->GetData();
}

void Texture::Resize(unsigned int width, unsigned int height)
{
	((OpenTracerCore::Texture*)mData)->Resize(width, height);
}

void Texture::Clear(float red, float green, float blue, float alpha)
{
	((OpenTracerCore::Texture*)mData)->ClearColor(red, green, blue, alpha);
}

RayGenerator::RayGenerator()
{
	OpenTracerCore::RayBuffer* rb = new OpenTracerCore::RayBuffer(g_mContext);
	mData = (void*)rb;
}

RayGenerator::~RayGenerator()
{
	delete ((OpenTracerCore::RayBuffer*)mData);
}

void RayGenerator::GeneratePrimary(float posX, float posY, float posZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ, float aspect, float fov, int width, int height, float nearPlane, float farPlane)
{
	((OpenTracerCore::RayBuffer*)mData)->SetCamera(OpenTracerCore::float4(posX, posY, posZ, 1.0f), 
		OpenTracerCore::float4(targetX, targetY, targetZ, 1.0f),
		OpenTracerCore::float4(upX, upY, upZ, 0.0f), aspect, fov, width, height, nearPlane, farPlane);
	((OpenTracerCore::RayBuffer*)mData)->GeneratePrimary();
}

void Renderer::Render(Texture* output, RayGenerator* raygen)
{
	((OpenTracerCore::Texture*)output->mData)->SetData(((OpenTracerCore::RayBuffer*)raygen->mData)->GetRayBuffer());
}