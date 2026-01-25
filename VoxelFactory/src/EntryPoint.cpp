#include "Core/Core.h"
#include "Core/Utils.h"
#include "Core/Window.h"
#include "Renderer/Shader.h"
#include "Renderer/Buffers.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <glm/glm.hpp>

const glm::vec3 c_Vertices[] = {
	{ -0.5f, -0.5f, 0.0f },
	{  0.5f, -0.5f, 0.0f },
	{  0.0f,  0.5f, 0.0f }
};
const uint32_t c_Indices[] = {
	0, 1, 2
};

int main(int argc, char** argv) {

	Ref<Window> window = CreateRef<Window>(800, 600, "Voxel Factory");

	Ref<VertexArray> vertex_array = CreateRef<VertexArray>();
	vertex_array->Bind();

	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(c_Vertices, sizeof(c_Vertices));
	vertex_buffer->Bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(c_Indices, sizeof(c_Indices));

	Ref<Shader> shader = CreateRef<Shader>("assets/shaders/Main.vert", "assets/shaders/Main.frag");
	shader->Bind();

	glViewport(0, 0, 800, 600);
	glClearColor(0.125, 0.13, 0.2, 1);

	// Main loop
	while (!window->ShouldClose()) {
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawElements(GL_TRIANGLES, sizeof(c_Indices) / sizeof(uint32_t), GL_UNSIGNED_INT, nullptr);

		window->Update();
	}

	return 0;
}