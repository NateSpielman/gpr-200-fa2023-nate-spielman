#include <stdio.h>
#include <math.h>
#include <vector>
#include <filesystem>

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
#include <ew/external/stb_image.h>

#include <gjn/cubemap.h>

#include <JSLib/terrain.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void resetCamera(ew::Camera& camera, ew::CameraController& cameraController);
void resetTerrain(ew::Transform& terrainTransform, float& HBTrange1, float& HBTrange2, float& HBTrange3, float& HBTrange4);
ew::Mat4 mat3Conversion(const ew::Mat4& m);

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

const int MAX_LIGHTS = 4, LIGHT_TYPES = 4;
int numLights = 1;

float prevTime;
ew::Vec3 bgColor = ew::Vec3(0.1f);

ew::Camera camera;
ew::CameraController cameraController;

struct Light {
	ew::Vec3 position, color, direction = ew::Vec3(0, -1, 0);
	int lightType = -1;
	float radius = 5, penumbra = 10, umbra = 43;
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

	unsigned int brickTexture = ew::loadTexture("assets/brick_color.jpg", GL_REPEAT,GL_LINEAR);
	unsigned int snowTexture = ew::loadTexture("assets/textures/snow_color.jpg", GL_REPEAT, GL_LINEAR);
	unsigned int grassTexture = ew::loadTexture("assets/textures/grass_color.jpg", GL_REPEAT, GL_LINEAR);
	unsigned int rockTexture = ew::loadTexture("assets/textures/rock_color.jpg", GL_REPEAT, GL_LINEAR);

	//Create terrain mesh
	ew::Mesh terrainMesh1(JSLib::createTerrain("assets/heightmaps/heightmap01.jpg"));
	ew::Mesh terrainMesh2(JSLib::createTerrain("assets/heightmaps/heightmap02.jpg"));
	ew::Mesh terrainMesh3(JSLib::createTerrain("assets/heightmaps/heightmap03.jpg"));

	ew::Mesh sphereMesh(ew::createSphere(0.5f, 64));

	//Create skybox mesh
	ew::Mesh skyboxMesh = ew::Mesh(ew::createCube(2));

	//Initialize transforms
	ew::Transform terrainTransform;
	terrainTransform.position = ew::Vec3(0.0f, 0.0f, 0.0f);

	ew::Transform sphereTransform;
	sphereTransform.position = ew::Vec3(-1.5f, 0.0f, 0.0f);


	//Initialize UI uniforms
	int heightmapNum = 1;

	float terMinY, terMaxY;

	float HBTrange1 = 0.15f;
	float HBTrange2 = 0.3f;
	float HBTrange3 = 0.65f;
	float HBTrange4 = 0.85f;

	//Initialize Lights
	Light lights[MAX_LIGHTS];

	lights[0].position = ew::Vec3(2.0f, 65.0f, 0.0f);
	lights[0].color = ew::Vec3(1.0f, 1.0f, 1.0f);
	lights[0].lightType = 1;

	lights[1].position = ew::Vec3(0.0f, 65.0f, 2.0f);
	lights[1].color = ew::Vec3(1.0f, 0.0f, 0.0f);

	lights[2].position = ew::Vec3(-2.0f, 65.0f, 0.0f);
	lights[2].color = ew::Vec3(0.0f, 1.0f, 0.0f);

	lights[3].position = ew::Vec3(0.0f, 65.0f, -2.0f);
	lights[3].color = ew::Vec3(0.0f, 0.0f, 1.0f);

	ew::Transform lightTransforms[MAX_LIGHTS];

	for (int i = 0; i < MAX_LIGHTS; i++) {
		lightTransforms[i].position = lights[i].position;
		lightTransforms[i].scale = ew::Vec3(0.5f);
	}

	//Set Material Values
	// UPDATED TO AVOID INITALIZATION MESSAGE - JERRY KAUFMAN
	Material mat = { 0.4, 0.4, 0.2, 8.0 }; 

	//Skybox shader configuration
	skyboxShader.use();
	skyboxShader.setInt("_Skybox", 0);

	std::vector<std::string> faces {
			"assets/right.jpg",
			"assets/left.jpg",
			"assets/top.jpg",
			"assets/bottom.jpg",
			"assets/front.jpg",
			"assets/back.jpg"
	};
	unsigned int cubemapTexture = gjn::loadCubemap(faces);

	resetCamera(camera,cameraController);

	//Camera stuff to see terrain better
	camera.farPlane = 200.0f;
	camera.position.y = 75.0f;
	camera.target.y = 75.0f;

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

		//Update terrain minimum Y and max Y
		terMinY = terrainTransform.position.y;
		terMaxY = terrainTransform.position.y + (64.0f * terrainTransform.scale.y);

		shader.use();
		
		//Bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, snowTexture);
		shader.setInt("_TextureSnow", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		shader.setInt("_TextureGrass", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, rockTexture);
		shader.setInt("_TextureRock", 2);

		shader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		//Draw shapes
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

			// GPU and CPU optimization
			shader.setFloat("_Lights[" + std::to_string(i) + "].penumbra", cos(ew::Radians(lights[i].penumbra)));
			shader.setFloat("_Lights[" + std::to_string(i) + "].umbra", cos(ew::Radians(lights[i].umbra)));

		}

		//Terrain UI uniforms
		shader.setFloat("_terMinY", terMinY);
		shader.setFloat("_terMaxY", terMaxY);

		shader.setFloat("_HBTrange1", HBTrange1);
		shader.setFloat("_HBTrange2", HBTrange2);
		shader.setFloat("_HBTrange3", HBTrange3);
		shader.setFloat("_HBTrange4", HBTrange4);

		//Draw terrain
		shader.setMat4("_Model", terrainTransform.getModelMatrix());
		switch (heightmapNum)
		{
		case 1:
			terrainMesh1.draw();
			break;
		case 2:
			terrainMesh2.draw();
			break;
		case 3:
			terrainMesh3.draw();
			break;
		default:
			terrainMesh1.draw();
			break;
		}

		unlitShader.use();
		unlitShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		for (int i = 0; i < numLights; i++) {
			unlitShader.setMat4("_Model", lightTransforms[i].getModelMatrix());
			unlitShader.setVec3("_Color", lights[i].color);
			sphereMesh.draw();
		}

		//Skybox
		glDepthFunc(GL_LEQUAL);
		glCullFace(GL_FRONT);
		skyboxShader.use();
		ew::Mat4 view = ew::Mat4(mat3Conversion(camera.ViewMatrix()));
		skyboxShader.setMat4("_View", view);
		skyboxShader.setMat4("_Projection", camera.ProjectionMatrix());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		skyboxMesh.draw();
		glCullFace(GL_BACK);
		glDepthFunc(GL_LESS);

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

			if (ImGui::CollapsingHeader("Material")) {
				ImGui::DragFloat("AmbientK", &mat.ambientK, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("DiffuseK", &mat.diffuseK, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("SpecularK", &mat.specular, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("Shininess", &mat.shininess, 0.1f, 2.0f);
			}

			ImGui::SliderInt("# of Lights", &numLights, 1, MAX_LIGHTS);

			// UPDATED GUI WITH A FOR LOOP - JERRY KAUFMAN
			for (int i = 0; i < numLights; i++) {
				ImGui::PushID(i);

				if (ImGui::CollapsingHeader(("Light: " + std::to_string(i + 1)).c_str())) {
					ImGui::DragFloat3("Position", &lightTransforms[i].position.x, 0.1f);
					ImGui::ColorEdit3("Color", &lights[i].color.x, 0.1f);
					ImGui::SliderInt("Light Type", &lights[i].lightType, -1, 2);

					// Specific controls for point light type
					if (lights[i].lightType == 0) {
						ImGui::Text("\nType - Point Light: ");
						ImGui::DragFloat("Radius", &lights[i].radius, 1.0f, 0, 100);
					}
					// Specific controls for directional light type
					else if (lights[i].lightType == 1) {
						ImGui::Text("\nType - Directional: ");
						ImGui::DragFloat3("Direction", &lights[i].direction.x, 0.1f);
					}
					// Specific controls for spotlight light type
					else if (lights[i].lightType == 2) {
						ImGui::Text("\nType - Spotlight: ");
						ImGui::DragFloat("Penumbra", &lights[i].penumbra, 1.0f, 1, 90);
						ImGui::DragFloat("Umbra", &lights[i].umbra, 1.0f, 0, 90);

						// Clamps the penumbra and umbra.
						// - Penumbra can never be larger than umbra
						// - Umbra can never be smaller than penumbra
						if (lights[i].penumbra > lights[i].umbra) {
							lights[i].penumbra = lights[i].umbra;
						} else if (lights[i].umbra < lights[i].penumbra) {
							lights[i].umbra = lights[i].penumbra;
						} else {}

						ImGui::DragFloat3("Direction", &lights[i].direction.x, 0.1f);
						ImGui::DragFloat("Radius", &lights[i].radius, 1.0f, 0, 100);
					}
					else {
						ImGui::Text("\nType - NONE ");
					}
				}
				ImGui::PopID();
			}

			//Terrain UI
			if (ImGui::CollapsingHeader("Terrain"))
			{
				ImGui::SliderInt("Heightmap", &heightmapNum, 1, 3);
				ImGui::DragFloat3("Position", &terrainTransform.position.x, 0.1f);
				ImGui::DragFloat3("Scale", &terrainTransform.scale.x, 0.1f);

				if (ImGui::CollapsingHeader("Height Based Texturing Ranges"))
				{
					ImGui::DragFloat("Range 1", &HBTrange1, 0.05f, 0.0f, 1.0f);
					ImGui::DragFloat("Range 2", &HBTrange2, 0.05f, 0.0f, 1.0f);
					ImGui::DragFloat("Range 3", &HBTrange3, 0.05f, 0.0f, 1.0f);
					ImGui::DragFloat("Range 4", &HBTrange4, 0.05f, 0.0f, 1.0f);
				}

				if (ImGui::Button("Reset Terrain")) {
					resetTerrain(terrainTransform, HBTrange1, HBTrange2, HBTrange3, HBTrange4);
				}
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

//Terrain reset
void resetTerrain(ew::Transform& terrainTransform, float& HBTrange1, float& HBTrange2, float& HBTrange3, float& HBTrange4)
{
	terrainTransform.position = ew::Vec3(0.0f);
	terrainTransform.scale = ew::Vec3(1.0f);

	HBTrange1 = 0.15f;
	HBTrange2 = 0.3f;
	HBTrange3 = 0.65f;
	HBTrange4 = 0.85f;
}

ew::Mat4 mat3Conversion(const ew::Mat4& m) {
	return ew::Mat4(m[0][0], m[1][0], m[2][0], 0.0f,
					m[0][1], m[1][1], m[2][1], 0.0f,
					m[0][2], m[1][2], m[2][2], 0.0f,
					m[0][3], m[1][3], m[2][3], 1.0f);
}
