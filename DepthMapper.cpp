// DepthMapper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Mesh.h"
#include "Model.h"

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;
constexpr float FOV = 45.0f;

const char* vertexSource =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"out vec3 FragPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"	FragPos = vec3(view * model * vec4(aPos, 1.0));\n"
"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\n\0";

const char* fragSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 FragPos;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(vec3(1.0 - ((-FragPos.z - 100.0) / 100.0)), 1.0);\n"
//"	FragColor = vec4(vec3(FragPos.z / 200.0), 1.0);\n"
"}\n\0";

int MakeProgram();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	int shader = MakeProgram();
	glUseProgram(shader);
	glEnable(GL_DEPTH_TEST);

	Model head("D:\\tiern\\Documents\\UCL Year 3\\ResearchGroup\\Database-Master_V1-3\\H3\\H3_Scans\\H3_3DScan.obj");

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 camPos(200.0f, 0.0f, 0.0f);
		glm::vec3 camDir(-1.0f, 0.0f, 0.0f);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, -120.0f, -120.0f));
		glm::mat4 view = glm::lookAt(camPos, camPos + camDir, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj = glm::perspective(glm::radians(FOV), (float)WIDTH / HEIGHT, 0.1f, 400.0f);

		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &proj[0][0]);

		head.Draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

int MakeProgram()
{
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Vertex Shader compilation failed\n" << infoLog << std::endl;
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "Fragment Shader compilation failed\n" << infoLog << std::endl;
	}

	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "Shader linking failed\n" << infoLog << std::endl;
	}

	return shaderProgram;
}