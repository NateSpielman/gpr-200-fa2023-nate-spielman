#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ns/shader.h>
#include <ns/texture.h>

struct Vertex {
	float x, y, z;
	float u, v;
};

unsigned int createVAO(Vertex* vertexData, int numVertices, unsigned short* indicesData, int numIndices);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

Vertex vertices[4] = {
	{-1.0, -1.0, 0.0, 0.0, 0.0},
	{1.0, -1.0, 0.0, 1.0, 0.0},
	{1.0, 1.0, 0.0, 1.0, 1.0},
	{-1.0, 1.0, 0.0, 0.0, 1.0}
};
unsigned short indices[6] = {
	0, 1, 2,
	2, 3, 0
};

//Time Speed
float tSpeed = 1.0f;

//Distortion Intensity 
float distortion = 0.05;

int main() {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Textures", NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return 1;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	
	//Create shaders for background and character
	ns::Shader backgroundShader("assets/background.vert", "assets/background.frag");
	ns::Shader characterShader("assets/character.vert", "assets/character.frag");

	//Create textures for background, character, and noise
	unsigned int noiseTexture = ns::loadTexture("assets/noiseTexture.png", GL_REPEAT, GL_LINEAR);
	unsigned int backgroundTexture = ns::loadTexture("assets/spaceBackground.png", GL_REPEAT, GL_LINEAR);
	unsigned int characterTexture = ns::loadTexture("assets/penguin.png", GL_CLAMP_TO_BORDER, GL_NEAREST);

	//Place noise texture in unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	//Place background texture in unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	//Place character texture in unit 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, characterTexture);

	unsigned int quadVAO = createVAO(vertices, 4, indices, 6);

	glBindVertexArray(quadVAO);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.3f, 0.4f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Get time
		float time = (float)glfwGetTime();

		//Background
		backgroundShader.use();
		//Sample from unit 0 and 1
		backgroundShader.setInt("_NoiseTexture", 0);
		backgroundShader.setInt("_Texture", 1);
		//Set Time + Time speed
		backgroundShader.setFloat("_Time", time);
		backgroundShader.setFloat("_TimeSpeed", tSpeed);
		//Set distortion intensity 
		backgroundShader.setFloat("_Distortion", distortion);

		//Draw
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

		//Character
		characterShader.use();
		//Sample from unit 2
		characterShader.setInt("_Texture", 2);
		//Set Time + Time speed
		characterShader.setFloat("_Time", time);
		characterShader.setFloat("_TimeSpeed", tSpeed);

		//Draw
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			ImGui::SliderFloat("Distortion Intensity", &distortion, 0.0f, 0.5f);
			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

unsigned int createVAO(Vertex* vertexData, int numVertices, unsigned short* indicesData, int numIndices) {
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Vertex Buffer Object 
	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*numVertices, vertexData, GL_STATIC_DRAW);

	//Element Buffer Object
	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * numIndices, indicesData, GL_STATIC_DRAW);

	//Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex,x));
	glEnableVertexAttribArray(0);

	//UV attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, u)));
	glEnableVertexAttribArray(1);

	return vao;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

