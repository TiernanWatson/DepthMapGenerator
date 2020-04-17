// DepthMapper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <FreeImage.h>
#include "Mesh.h"
#include "Model.h"

// Width and height needed for our project
constexpr int WIDTH = 423;
constexpr int HEIGHT = 563;
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
"uniform float xMax;\n"
"uniform float camDist;\n"
"void main()\n"
"{\n"
"   float xMaxToCam = camDist - xMax;\n"
"   float headHalfWidth = xMax;\n"
"	FragColor = vec4(vec3(1.0 - ((-FragPos.z - xMaxToCam) / headHalfWidth)), 1.0);\n"
//"	FragColor = vec4(vec3(FragPos.z / 200.0), 1.0);\n"
"}\n\0";

// True if viewing left, false otherwise
bool rightCaptured = false;

float xMax = 0;
float xMaxToCam = 0;

// File details of head being viewed
std::string headFileDirectory = "";
std::string headFileName = "";

// Camera view details
glm::vec3 camPos(180.0f, 0.0f, 0.0f);
glm::vec3 camDir(-1.0f, 0.0f, 0.0f);

int MakeProgram();
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void OutputImage(const char* filePath, GLubyte* pixels);
void OutputCSV(const char* filePath, GLubyte* pixels);
GLubyte* GetPixels();

int main(int argc, char* argv[])
{
	// -------------------------
	// Check Arguments + Init OpenGL
	// -------------------------

	if (argc < 2)
	{
		std::cout << "Please supply a head obj filename!" << std::endl;
		return -1;
	}

	std::cout << "Initializing OpenGL..." << std::endl;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Group Research Depth Mapper", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, KeyCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	int shader = MakeProgram();
	glUseProgram(shader);
	glEnable(GL_DEPTH_TEST);

	// -------------------------
	// Load Model + Calc Max Distance
	// -------------------------

	std::cout << "Loading Model..." << std::endl;

	Model head(argv[1]);
	headFileName = head.GetFileName();
	headFileDirectory = head.GetDirectory();

	std::cout << "Finding maximum x vertex..." << std::endl;

	float xGreatest = 0.0f;
	for (Mesh m : head.GetMeshes())
	{
		for (Vertex v : m.GetVertices())
		{
			if (v.pos.x > xGreatest)
				xGreatest = v.pos.x;
		}
	}

	glUniform1f(glGetUniformLocation(shader, "xMax"), xGreatest);
	xMax = xGreatest;

	// -------------------------
	// Do Drawing + Printing
	// -------------------------

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get camera movement input
		float horizontal = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? 1.0f
			: glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? -1.0f
			: 0.0f;
		float vertical = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? 1.0f
			: glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? -1.0f
			: 0.0f;
		float forward = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? 1.0f
			: glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? -1.0f
			: 0.0f;

		glm::vec3 movement(-forward, vertical, -horizontal);
		camPos += movement * 0.1f;

		glUniform1f(glGetUniformLocation(shader, "camDist"), glm::abs(camPos.x));
		xMaxToCam = glm::abs(camPos.x) - xMax;

		// Set up model, view, proj matrices
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, -120.0f, -120.0f));
		glm::mat4 view = glm::lookAt(camPos, camPos + camDir, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj = glm::perspective(glm::radians(FOV), (float)WIDTH / HEIGHT, 0.1f, 400.0f);

		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &proj[0][0]);

		head.Draw();

		// Display head
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// Compiles the vertex and fragment shader, returns program number
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

// Handles GLFW key events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Output image and CSV on Enter
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		GLubyte* pixels = GetPixels();

		std::string fileName = headFileName;
		std::string outputFileName = fileName.substr(0, fileName.find_last_of('.'));
		outputFileName += rightCaptured ? "L" : "R";

		std::string imageOutputPath = headFileDirectory + "/" + outputFileName + ".bmp";
		OutputImage(imageOutputPath.c_str(), pixels);

		std::string csvOutputPath = headFileDirectory + "/" + outputFileName + ".csv";
		OutputCSV(csvOutputPath.c_str(), pixels);

		rightCaptured = !rightCaptured;
		camPos = glm::vec3(rightCaptured ? -180.0f : 180.0f, 0.0f, 0.0f);
		camDir = glm::vec3(rightCaptured ? 1.0f : -1.0f, 0.0f, 0.0f);

		delete[] pixels;
	}
}

// Outputs an image of the current frame
void OutputImage(const char* filePath, GLubyte* pixels)
{
	FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, WIDTH, HEIGHT, 3 * WIDTH, 24, 0x0000FF, 0xFF0000, 0x00FF00, false);
	FreeImage_Save(FIF_BMP, image, filePath, 0);
	FreeImage_Unload(image);
}

// Output a CSV file with pixel values
void OutputCSV(const char* filePath, GLubyte* pixels)
{
	std::ofstream outfile;
	outfile.open(filePath);

	float min = 255.0f;

	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			float current = (float)pixels[i * WIDTH + 3 * j] / 255.0f;
			float result = -1.0f * ((float)xMax * (current - 1.0f) - xMaxToCam);
			outfile << result << ", ";

			if (result < min) min = result;
		}
		outfile << "\n";
	}

	outfile.close();
}

// Retrieve pixel data from GPU
GLubyte* GetPixels()
{
	GLfloat* fpixels = new GLfloat[3 * WIDTH * HEIGHT];

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, fpixels);

	GLubyte* pixels = new GLubyte[3 * WIDTH * HEIGHT];

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	return pixels;
}
