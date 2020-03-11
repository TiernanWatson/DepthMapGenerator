#pragma once
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>

struct Vertex
{
	glm::vec3 pos;
};

class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

	void Init();
	void Draw();

private:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

