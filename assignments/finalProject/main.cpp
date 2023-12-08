#include <stdio.h>
#include <math.h>
#include <vector>

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

#include <gjn/cubemap.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void resetCamera(ew::Camera& camera, ew::CameraController& cameraController);

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

const int MAX_LIGHTS = 4, LIGHT_TYPES = 4;
int numLights = 1;

float prevTime;
ew::Vec3 bgColor = ew::Vec3(0.1f);

ew::Camera camera;
ew::CameraController cameraController;

std::vector<std::string> cubemapFaces {
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"back.jpg",
	"front.jpg"
};

float skyboxVertices[] {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

struct Light {
	ew::Vec3 position, color, direction;
	int lightType = -1;
	float radius = 5, penumbra = 5, umbra = 30;
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
	ew::Shader skyboxShader("assets/skybox.vert", "assets/skybox.frag");
	unsigned int cubemapTexture = gjn::loadCubemap(cubemapFaces);
	unsigned int brickTexture = ew::loadTexture("assets/brick_color.jpg",GL_REPEAT,GL_LINEAR);

	//Create meshes
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
		lightTransforms[i].direction = lights[i].direction;
		lightTransforms[i].scale = ew::Vec3(0.5f);
	}

	//Set Material Values
	Material mat;
	mat.diffuseK = 0.4;
	mat.specular = 0.4;
	mat.ambientK = 0.2;
	mat.shininess = 8.0;

	glEnable(GL_DEPTH_TEST);

	//Skybox VAO + VBO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//Skybox shader config
	skyboxShader.use();
	skyboxShader.setInt("_Skybox", 0);

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
			shader.setVec3("_Lights[" + std::to_string(i) + "].position", lightTransforms[i].position);
			shader.setVec3("_Lights[" + std::to_string(i) + "].color", lights[i].color);
			shader.setVec3("_Lights[" + std::to_string(i) + "].direction", lights[i].direction);
			shader.setInt("_Lights[" + std::to_string(i) + "].lightType", lights[i].lightType);
			shader.setFloat("_Lights[" + std::to_string(i) + "].radius", lights[i].radius);
			shader.setFloat("_Lights[" + std::to_string(i) + "].penumbra", lights[i].penumbra);
			shader.setFloat("_Lights[" + std::to_string(i) + "].umbra", lights[i].umbra);

		}

		unlitShader.use();
		unlitShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		for (int i = 0; i < numLights; i++)
		{
			unlitShader.setMat4("_Model", lightTransforms[i].getModelMatrix());
			unlitShader.setVec3("_Color", lights[i].color);
			sphereMesh.draw();
		}

		//Skybox
		glDepthMask(GL_LEQUAL);
		skyboxShader.use();
		skyboxShader.setMat4("_Projection", camera.ProjectionMatrix());
		skyboxShader.setMat4("_View", camera.ViewMatrix());
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_LESS);

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
					ImGui::DragInt("Light Type", &lights[i].lightType, 1.0f, -1, 2);
					if (lights[i].lightType == 0) {
						ImGui::DragFloat("Radius", &lights[i].radius, 1.0f, 0, 100);
					}
					else if (lights[i].lightType == 1) {
						ImGui::DragFloat3("Direction", &lightTransforms[i].direction.x, 0.1f);
					}
					else if (lights[i].lightType == 2) {
						ImGui::DragFloat("Penumbra", &lights[i].penumbra, 1.0f, 1, 100);
						ImGui::DragFloat("Umbra", &lights[i].umbra, 1.0f, 0, 100);
					}
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


