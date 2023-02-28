#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include <fstream>
#include "Ray.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
float offset = 0;
float SIZE = 10;
float fov = 90.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float alfa = 0;
float beta = 0;
float gamma = 0;
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view(1.0f);
glm::mat4 projection(1.0f);
float yaw = 90.0f;
float pitch = 0.0f;
float lastX = 1024.0 / 2.0;
float lastY = 768.0 / 2.0;
bool firstMouse = true;
bool disableMouse = false;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (disableMouse)return;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.2f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	const float cameraSpeed = 0.05f; //the speed of camera movement when using keys
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		cameraPos += glm::normalize(cameraUp) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		cameraPos -= glm::normalize(cameraUp) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		disableMouse = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		disableMouse = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

struct Surface
{
	float* coordinates;
	unsigned size;
	unsigned* indexBuffer;
	unsigned indexCount;

	Surface(float* vertex, unsigned n, unsigned* indices, unsigned m) :coordinates{ vertex }, size{ n }, indexBuffer{ indices }, indexCount{ m }{}
	Surface(Surface&& s) :size{ s.size }, indexCount{ s.indexCount }
	{
		coordinates = s.coordinates;
		indexBuffer = s.indexBuffer;
		s.coordinates = nullptr;
		s.indexBuffer = nullptr;
	}
	Surface(const Surface& s) :size{ s.size }, indexCount{ s.indexCount }
	{
		coordinates = new float[size];
		std::copy(s.coordinates, s.coordinates + size, coordinates);
		indexBuffer = new unsigned[indexCount];
		std::copy(s.indexBuffer, s.indexBuffer + indexCount, indexBuffer);
	}
	Surface operator=(const Surface& s)
	{
		//if (this == (&s))return *this;
		size = s.size;
		indexCount = s.indexCount;
		float* nVertex = new float[size];
		unsigned* nIndex = new unsigned[indexCount];
		std::copy(s.coordinates, s.coordinates + s.size, nVertex);
		std::copy(s.indexBuffer, s.indexBuffer + s.indexCount, nIndex);
		delete[] coordinates;
		delete[] indexBuffer;
		coordinates = nVertex;
		indexBuffer = nIndex;
		return *(this);
	}
	~Surface()
	{
		delete[] coordinates;
		delete[] indexBuffer;
	}
};

Surface GenerateIndexedTriangleStripPlane(int hVertices, int vVertices, float size)
{

	float dy = size / (hVertices - 1);
	float dx = size / (vVertices - 1);
	unsigned n = hVertices * vVertices * 5;
	float* coordinates = new float[n];
	unsigned i{};
	float u = 0;

	for (float y = size / 2; y >= -size / 2 - 0.0001; y -= dy)
	{
		float v = 0;
		for (float x = -size / 2; x <= size / 2 + 0.0001; x += dx)
		{
			coordinates[i++] = x;
			coordinates[i++] = y;
			coordinates[i++] = 0;
			coordinates[i++] = u;
			coordinates[i++] = v;
			v += dx / size;
		}
		u += dy / size;
	}
	unsigned m = 2 * hVertices * (vVertices - 1) + (vVertices - 2) * 2;
	unsigned* index = new unsigned[m];
	i = 0;
	for (unsigned k = 1; k <= hVertices * (vVertices - 1); ++k)
	{
		index[i++] = k - 1;
		index[i++] = k + hVertices - 1;
		if (k % hVertices == 0 && k != hVertices * (vVertices - 1))
		{
			index[i++] = k + hVertices - 1;
			index[i++] = k;
		}
	}

	return Surface{ coordinates, n, index, m };
}

void callback(GLFWwindow* window, int button, int action, int mods)
{
	double x;
	double y;

	glfwGetCursorPos(window, &x, &y);

	auto temp = Ray::ConstructRayFromPixel(glm::inverse(view), 1024, 768, fov, glm::vec2(x, y));
	glm::vec3 a(-SIZE / 2, SIZE / 2, 0);
	glm::vec3 b(-SIZE / 2, -SIZE / 2, 0);
	glm::vec3 c(SIZE / 2, -SIZE / 2, 0);
	glm::vec3 d(SIZE / 2, SIZE / 2, 0);
	glm::vec3 e(SIZE / 2, -SIZE / 2, 0);
	glm::vec3 f(-SIZE / 2, SIZE / 2, 0);
	glm::vec3 point;
	if (Ray::RayTriangleIntersection(a, b, c, temp, point))
	{

		alfa = point[0];
		beta = point[1];
		gamma = point[2];
		//std::cout << alfa << std::endl << beta << std::endl << gamma << std::endl;
	}
	else if (Ray::RayTriangleIntersection(d, e, f, temp, point))
	{
		alfa = point[0];
		beta = point[1];
		gamma = point[2];
		//std::cout << alfa << std::endl << beta << std::endl << gamma<<std::endl;
	}
	offset = glfwGetTime();
}

int main()
{
	std::ofstream fout{ "file.txt" };
	//initializing glfw, glew
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Creating the window
	GLFWwindow* window;
	window = glfwCreateWindow(1024, 768, "CG", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to create window");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW");
		return 1;
	}
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);//setting mouse callback to update camera
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glViewport(0, 0, 1024, 768);//specifying the screen size for the viewport transform

	glfwSetMouseButtonCallback(window, callback);


	//WATER
	Surface s = GenerateIndexedTriangleStripPlane(64, 64, SIZE);
	float* vertices = s.coordinates;
	unsigned* indices = s.indexBuffer;
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * s.size, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * s.indexCount, indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
	//END WATER

	//T1
	Surface s1 = GenerateIndexedTriangleStripPlane(64, 64, SIZE);
	vertices = s1.coordinates;
	indices = s1.indexBuffer;
	for (auto i = vertices; i < vertices + s1.size; i += 5)
	{
		//(*i) =(*i)-1;
		*(i + 2) = -0.15f;
	}
	unsigned int VBO1;
	glGenBuffers(1, &VBO1);

	unsigned int VAO1;
	glGenVertexArrays(1, &VAO1);
	unsigned int EBO1;
	glGenBuffers(1, &EBO1);

	glBindVertexArray(VAO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * s1.size, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * s1.indexCount, indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
	//END T1





	//the (commented)call below is very useful when debugging(to use the line strips)
	//wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);//enable z buffer test
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader sd{ "VertexShader.txt", "GeometryShader.txt","FragmentShader.txt" };
	Shader sd1{ "VertexShader1.txt", "GeometryShader1.txt","FragmentShader1.txt" };
	Shader sd2{ "VertexShader1.txt", "GeometryShader2.txt","FragmentShader2.txt" };
	//model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::translate(model, glm::vec3(0.0f, 0.25f, 0.0f));



	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	int width, height, nrChannels;
	unsigned char* data = stbi_load("WaterDiffuse.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);



	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	data = stbi_load("TerrainDiffuse.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);



	unsigned int heightmap;
	glGenTextures(1, &heightmap);

	//glActiveTexture(GL_TEXTURE0);
	//glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, heightmap); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	data = stbi_load("TerrainHeightMap.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);



	unsigned int grass;
	glGenTextures(1, &grass);
	glBindTexture(GL_TEXTURE_2D, grass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("GrassDiffuse.PNG", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);




	unsigned int distribution;
	glGenTextures(1, &distribution);

	//glActiveTexture(GL_TEXTURE0);
	//glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, distribution); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	data = stbi_load("GrassDistribution.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);




	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);



	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		int modelLoc;

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glm::vec3 pos(0,0,0);
		glm::vec3 pos(cameraPos[0], cameraPos[1], -cameraPos[2]);
		glm::vec3 front(cameraFront[0], cameraFront[1], -cameraFront[2]);
		view = glm::lookAt(pos, pos + front, cameraUp);
		projection = glm::perspective(glm::radians(fov), (float)1024 / (float)768, 0.1f, 100.0f);






		sd2.bind();
		sd2.SetInteger("elevation", 2);
		sd2.SetInteger("grass", 3);
		sd2.SetInteger("distribution", 4);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, grass);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, distribution);

		modelLoc = glGetUniformLocation(sd2.GetProgramId(), "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modelLoc = glGetUniformLocation(sd2.GetProgramId(), "view");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(view));
		modelLoc = glGetUniformLocation(sd2.GetProgramId(), "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO1);
		glDrawElements(GL_TRIANGLE_STRIP, s1.indexCount, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);


		glDisable(GL_CULL_FACE);



		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(glm::radians(fov), (float)1024 / (float)768, 0.1f, 100.0f);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);//background color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//WATER
		sd.bind();
		sd.SetInteger("ourTexture", 0);
		sd.SetInteger("reflection", 6);
		sd.SetFloat("offset", offset);
		modelLoc = glGetUniformLocation(sd.GetProgramId(), "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modelLoc = glGetUniformLocation(sd.GetProgramId(), "view");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(view));
		modelLoc = glGetUniformLocation(sd.GetProgramId(), "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projection));
		modelLoc = glGetUniformLocation(sd.GetProgramId(), "lightColor");
		glUniform3f(modelLoc, 1.0f, 1.0f, 1.0f);
		sd.SetVec3("center", alfa, beta, gamma);
		modelLoc = glGetUniformLocation(sd.GetProgramId(), "lightD");
		glUniform3f(modelLoc, 0.0f, 0.0f, -1.0f);
		auto temp = glfwGetTime();
		sd.SetFloat("shift", temp);
		//std::cout << temp<<std::endl;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLE_STRIP, s.indexCount, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		//sd.unbind();
		//END WATER

		//T1

		sd1.bind();
		sd1.SetInteger("elevation", 2);
		sd1.SetInteger("ourTexture", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, heightmap);

		modelLoc = glGetUniformLocation(sd1.GetProgramId(), "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modelLoc = glGetUniformLocation(sd1.GetProgramId(), "view");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(view));
		modelLoc = glGetUniformLocation(sd1.GetProgramId(), "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projection));
		modelLoc = glGetUniformLocation(sd1.GetProgramId(), "lightColor");
		glUniform3f(modelLoc, 1.0f, 1.0f, 1.0f);
		modelLoc = glGetUniformLocation(sd1.GetProgramId(), "lightD");
		glUniform3f(modelLoc, 0.0f, 0.0f, -1.0f);

		glBindVertexArray(VAO1);
		glDrawElements(GL_TRIANGLE_STRIP, s1.indexCount, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		//END T1



		sd2.bind();
		sd2.SetInteger("elevation", 2);
		sd2.SetInteger("grass", 3);
		sd2.SetInteger("distribution", 4);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, grass);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, distribution);

		modelLoc = glGetUniformLocation(sd2.GetProgramId(), "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		modelLoc = glGetUniformLocation(sd2.GetProgramId(), "view");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(view));
		modelLoc = glGetUniformLocation(sd2.GetProgramId(), "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO1);
		glDrawElements(GL_TRIANGLE_STRIP, s1.indexCount, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);



		glfwSwapBuffers(window);
		glfwPollEvents();

	}



	glfwTerminate();
	return 77;
}