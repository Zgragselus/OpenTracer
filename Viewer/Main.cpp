#include "Main.h"
//#include "Model.h"
#include "Sponza.h"

#include <gl/glext.h>

#include <chrono>
#include <iostream>

PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLMAPBUFFERPROC glMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = NULL;

int main()
{
	sf::Window window = sf::Window(sf::VideoMode(640, 480), "Viewer", sf::Style::Default, sf::ContextSettings(32));
	window.setVerticalSyncEnabled(true);

	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
	glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");

	OpenTracer::Context::GetInstance().Initialize(OpenTracer::Context::CONTEXT_TYPE_GPU);
	OpenTracer::Texture* image = new OpenTracer::Texture(640, 480);
	OpenTracer::RayGenerator* raygen = new OpenTracer::RayGenerator();
	OpenTracer::Scene* scene = new OpenTracer::Scene(model, sizeof(model) / sizeof(float) / 4);
	OpenTracer::Aggregate* as = new OpenTracer::Aggregate(OpenTracer::Aggregate::AGGREGATE_KDTREE, scene, "C:\\Programming\\OpenTracer\\KDTree.conf");
	OpenTracer::Renderer* renderer = new OpenTracer::Renderer();

	unsigned int gl;
	glGenTextures(1, &gl);
	glBindTexture(GL_TEXTURE_2D, gl);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->GetWidth(), image->GetHeight(), 0, GL_RGBA, GL_FLOAT, image->GetData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned int pbo;
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, image->GetWidth() * image->GetHeight() * sizeof(float) * 4, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	float time = 0.0f;

	bool run = true;
	while (run)
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
			{
				run = false;
			}
			else if (e.type == sf::Event::Resized)
			{
				glViewport(0, 0, e.size.width, e.size.height);
				image->Resize(e.size.width, e.size.height);

				glDeleteTextures(1, &gl);
				glGenTextures(1, &gl);
				glBindTexture(GL_TEXTURE_2D, gl);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->GetWidth(), image->GetHeight(), 0, GL_RGBA, GL_FLOAT, image->GetData());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glDeleteBuffers(1, &pbo);
				glGenBuffers(1, &pbo);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
				glBufferData(GL_PIXEL_UNPACK_BUFFER, image->GetWidth() * image->GetHeight() * sizeof(float) * 4, 0, GL_DYNAMIC_DRAW);
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			}
		}

		auto start = std::chrono::high_resolution_clock::now();
		raygen->GeneratePrimary(0.0f, 1.0f, 0.0f, 2.0f * sinf(time), 1.5f, 2.0f * cosf(time), 0.0f, 1.0f, 0.0f, (float)image->GetHeight() / (float)image->GetWidth(), 45.0f, image->GetWidth(), image->GetHeight(), 0.1f, 10000.0f);
		renderer->Render(scene, as, raygen, image);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		float *ptr = (float*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, image->GetData(), sizeof(float) * 4 * image->GetWidth() * image->GetHeight());
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		glBindTexture(GL_TEXTURE_2D, gl);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->GetWidth(), image->GetHeight(), GL_RGBA, GL_FLOAT, (void*)0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);
		glBegin(GL_TRIANGLES);
		glTexCoord2f(0, 0); glVertex2f(-1, -1);
		glTexCoord2f(1, 0); glVertex2f(1, -1);
		glTexCoord2f(1, 1); glVertex2f(1, 1);
		glTexCoord2f(0, 0); glVertex2f(-1, -1);
		glTexCoord2f(1, 1); glVertex2f(1, 1);
		glTexCoord2f(0, 1); glVertex2f(-1, 1);
		glEnd();
		std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
		long long int us_i = us.count();
		if (us_i > 0)
		{
			std::cout << "FPS: " << 1000000 / us_i <<
				" Time: " << us_i / 1000 << "ms " <<
				"Rays: " << image->GetWidth() * image->GetHeight() * 1.0 / (double)us_i << "Mrays/s" << std::endl;
		}

		window.display();

		time += 0.01f;
	}

	OpenTracer::Context::GetInstance().Release();
}