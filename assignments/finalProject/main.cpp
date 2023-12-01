#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/texture.h>
#include <ew/procGen.h>
#include <ew/transform.h>
#include <ew/camera.h>
#include <ew/cameraController.h>

using namespace std;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void resetCamera(ew::Camera& camera, ew::CameraController& cameraController);

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

const int MAX_LIGHTS = 4, LIGHT_TYPES = 4;
int numLights = 4;

float prevTime;
ew::Vec3 bgColor = ew::Vec3(0.1f);

ew::Camera camera;
ew::CameraController cameraController;

struct Light {
	ew::Vec3 position; //World space
	ew::Vec3 color; //RGB
	int lightType = -1;
	float radius = 0, penumbra = 0, umbra = 0;
};

struct Material {
	float ambientK; //Ambient coefficient (0-1)
	float diffuseK; //Diffuse coefficient (0-1)
	float specular; //Specular coefficient (0-1)
	float shininess; //Shininess
};

int main() {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Camera", NULL, NULL);
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

	//Global settings
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	ew::Shader shader("assets/defaultLit.vert", "assets/defaultLit.frag");
	ew::Shader unlitShader("assets/unlit.vert", "assets/unlit.frag");
	unsigned int brickTexture = ew::loadTexture("assets/brick_color.jpg",GL_REPEAT,GL_LINEAR);

	//Create cube
	ew::Mesh cubeMesh(ew::createCube(1.0f));
	ew::Mesh planeMesh(ew::createPlane(5.0f, 5.0f, 10));
	ew::Mesh sphereMesh(ew::createSphere(0.5f, 64));
	ew::Mesh cylinderMesh(ew::createCylinder(0.5f, 1.0f, 32));

	//Initialize transforms
	ew::Transform cubeTransform;
	ew::Transform planeTransform;
	ew::Transform sphereTransform;
	ew::Transform cylinderTransform;
	planeTransform.position = ew::Vec3(0, -1.0, 0);
	sphereTransform.position = ew::Vec3(-1.5f, 0.0f, 0.0f);
	cylinderTransform.position = ew::Vec3(1.5f, 0.0f, 0.0f);

	//Initialize Lights
	Light lights[MAX_LIGHTS];

	lights[0].position = ew::Vec3(2.0f, 1.0f, 0.0f);
	lights[0].color = ew::Vec3(1.0f, 0.0f, 0.0f);

	lights[1].position = ew::Vec3(0.0f, 1.0f, 2.0f);
	lights[1].color = ew::Vec3(0.0f, 1.0f, 0.0f);

	lights[2].position = ew::Vec3(-2.0f, 1.0f, 0.0f);
	lights[2].color = ew::Vec3(0.0f, 0.0f, 1.0f);

	lights[3].position = ew::Vec3(0.0f, 1.0f, -2.0f);
	lights[3].color = ew::Vec3(1.0f, 1.0f, 0.0f);

	ew::Transform lightTransforms[MAX_LIGHTS];

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		lightTransforms[i].position = lights[i].position;
		lightTransforms[i].scale = ew::Vec3(0.5f);
	}

	//Set Material Values
	Material mat;
	mat.diffuseK = 0.4;
	mat.specular = 0.4;
	mat.ambientK = 0.2;
	mat.shininess = 8.0;

	resetCamera(camera,cameraController);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		float deltaTime = time - prevTime;
		prevTime = time;

		//Update camera
		camera.aspectRatio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
		cameraController.Move(window, &camera, deltaTime);

		//RENDER
		glClearColor(bgColor.x, bgColor.y,bgColor.z,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		shader.setInt("_Texture", 0);
		shader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		//Draw shapes
		shader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		shader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		shader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		shader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		//TODO: Render point lights
		shader.setVec3("_CamPos", camera.position);
		shader.setInt("_NumLights", numLights);

		shader.setFloat("_Material.diffuseK", mat.diffuseK);
		shader.setFloat("_Material.specular", mat.specular);
		shader.setFloat("_Material.ambientK", mat.ambientK);
		shader.setFloat("_Material.shininess", mat.shininess);

		// UPDATED LIGHT COLOR AND POSITION CODE - JERRY KAUFMAN
		for (int i = 0; i < numLights; i++) {
			shader.setVec3("_Lights[" + to_string(i) + "].position", lightTransforms[i].position);
			shader.setVec3("_Lights[" + to_string(i) + "].color", lights[i].color);
			shader.setInt("_Lights[" + to_string(i) + "].lightType", -1);
		}

		unlitShader.use();
		unlitShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		for (int i = 0; i < numLights; i++)
		{
			unlitShader.setMat4("_Model", lightTransforms[i].getModelMatrix());
			unlitShader.setVec3("_Color", lights[i].color);
			sphereMesh.draw();
		}

		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			if (ImGui::CollapsingHeader("Camera")) {
				ImGui::DragFloat3("Position", &camera.position.x, 0.1f);
				ImGui::DragFloat3("Target", &camera.target.x, 0.1f);
				ImGui::Checkbox("Orthographic", &camera.orthographic);
				if (camera.orthographic) {
					ImGui::DragFloat("Ortho Height", &camera.orthoHeight, 0.1f);
				}
				else {
					ImGui::SliderFloat("FOV", &camera.fov, 0.0f, 180.0f);
				}
				ImGui::DragFloat("Near Plane", &camera.nearPlane, 0.1f, 0.0f);
				ImGui::DragFloat("Far Plane", &camera.farPlane, 0.1f, 0.0f);
				ImGui::DragFloat("Move Speed", &cameraController.moveSpeed, 0.1f);
				ImGui::DragFloat("Sprint Speed", &cameraController.sprintMoveSpeed, 0.1f);
				if (ImGui::Button("Reset")) {
					resetCamera(camera, cameraController);
				}
			}

			ImGui::ColorEdit3("BG color", &bgColor.x);

			if (ImGui::CollapsingHeader("Material"))
			{
				ImGui::DragFloat("AmbientK", &mat.ambientK, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("DiffuseK", &mat.diffuseK, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("SpecularK", &mat.specular, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("Shininess", &mat.shininess, 0.1f, 2.0f);
			}

			ImGui::DragInt("Number of Lights", &numLights, 1.0f, 1, MAX_LIGHTS);

			// UPDATED GUI WITH A FOR LOOP - JERRY KAUFMAN
			for (int i = 0; i < numLights; i++) {
				ImGui::PushID(i);
				if (ImGui::CollapsingHeader("Lights")) {
					ImGui::DragFloat3("Position", &lightTransforms[i].position.x, 0.1f);
					ImGui::ColorEdit3("Color", &lights[i].color.x, 0.1f);
					ImGui::DragInt("Light Type", &lights[i].lightType, 1.0f, -1, 3);
				}
				ImGui::PopID();
			}

			ImGui::End();
			
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void resetCamera(ew::Camera& camera, ew::CameraController& cameraController) {
	camera.position = ew::Vec3(0, 0, 5);
	camera.target = ew::Vec3(0);
	camera.fov = 60.0f;
	camera.orthoHeight = 6.0f;
	camera.nearPlane = 0.1f;
	camera.farPlane = 100.0f;
	camera.orthographic = false;

	cameraController.yaw = 0.0f;
	cameraController.pitch = 0.0f;
}


