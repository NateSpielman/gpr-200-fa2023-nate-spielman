#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ns/shader.h>

struct Vertex {
	float x, y, z;
	float u, v;
};

unsigned int createVAO(Vertex* vertexData, int numVertices, unsigned int* indicesData, int numIndices);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

Vertex vertices[4] = {
	 // x    y    z   u  v
	{-0.5, -0.5, 0.0, 0, 0 }, //Bottom Left
	{ 0.5, -0.5, 0.0, 1, 0 }, //Bottom Right
	{ 0.5,  0.5, 0.0, 1, 1 }, //Top Right
	{-0.5,  0.5, 0.0, 0, 1 }, //Top Left
};

unsigned int indices[6] = {
	0, 1, 2, //Triangle 1
	2, 3, 0  //Triangle 2
};

//Time Speed
float tSpeed = 1.0f;

//Background Colors
float dayBColor[3] = { 0.91f, 0.78f, 0.329f };     //Day Bottom Color
float dayTColor[3] = { 0.8f, 0.2f, 0.1f };         //Day Top Color
float nightBColor[3] = { 0.58f, 0.043f, 0.259f };  //Night Bottom Color
float nightTColor[3] = { 0.188f, 0.141f, 0.329f }; //Night Top Color

//Sun Values
float sunR = 0.3f;							  //Sun Radius 
float sunColor[3] = { 1.0f, 0.773f, 0.129f }; //Sun Color

//Foreground Colors
float mountainColor[3] = { 0.153f, 0.439f, 0.388f }; //Mountain Color
float hillsColor[3] = { 0.153f, 0.588f, 0.388f };    //Hills Color

bool showImGUIDemoWindow = true;

int main() {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Triangle", NULL, NULL);
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

	ns::Shader shader("assets/vertexShader.vert", "assets/fragmentShader.frag");
	shader.use();
	//Resolution
	shader.setVec2("_Resolution", SCREEN_WIDTH, SCREEN_HEIGHT);

	unsigned int vao = createVAO(vertices, 6, indices, 6);
	glBindVertexArray(vao);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.3f, 0.4f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		float time = (float)glfwGetTime();

		//Set Shader Variables 
		//Time + Time Speed
		shader.setFloat("_Time", time);
		shader.setFloat("_TimeSpeed", tSpeed);
		//Background Colors
		shader.setVec3("_DayBottom", dayBColor[0], dayBColor[1], dayBColor[2]);
		shader.setVec3("_DayTop", dayTColor[0], dayTColor[1], dayTColor[2]);
		shader.setVec3("_NightBottom", nightBColor[0], nightBColor[1], nightBColor[2]);
		shader.setVec3("_NightTop", nightTColor[0], nightTColor[1], nightTColor[2]);
		//Sun Radius + Color
		shader.setFloat("_SunR", sunR);
		shader.setVec3("_SunColor", sunColor[0], sunColor[1], sunColor[2]);
		//Mountain Color
		shader.setVec3("_MountainColor", mountainColor[0], mountainColor[1], mountainColor[2]);
		//Hills Color
		shader.setVec3("_HillsColor", hillsColor[0], hillsColor[1], hillsColor[2]);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			ImGui::Checkbox("Show Demo Window", &showImGUIDemoWindow);
			ImGui::ColorEdit3("Day Sky Color Bottom", dayBColor);
			ImGui::ColorEdit3("Day Sky Color Top", dayTColor);
			ImGui::ColorEdit3("Night Sky Color Bottom", nightBColor);
			ImGui::ColorEdit3("Night Sky Color Top", nightTColor);
			ImGui::ColorEdit3("Sun Color", sunColor);
			ImGui::SliderFloat("Sun Radius", &sunR, 0.1f, 1.0f);
			ImGui::SliderFloat("Sun Speed", &tSpeed, 0.0f, 8.0f);
			ImGui::ColorEdit3("Mountains Color", mountainColor);
			ImGui::ColorEdit3("Hills Color", hillsColor);
			ImGui::End();
			if (showImGUIDemoWindow) {
				ImGui::ShowDemoWindow(&showImGUIDemoWindow);
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

unsigned int createVAO(Vertex* vertexData, int numVertices, unsigned int* indicesData, int numIndices) {
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Define a new buffer id
	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//Allocate space for + send vertex data to GPU.
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices * 3, vertexData, GL_STATIC_DRAW);

	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);

	//UV
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, u)));
	glEnableVertexAttribArray(1);

	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, indicesData, GL_STATIC_DRAW);

	return vao;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

