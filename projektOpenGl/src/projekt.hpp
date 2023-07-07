#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>

#include "SOIL/stb_image_aug.h"

#define STB_IMAGE_IMPLEMENTATION



namespace texture {
	GLuint blueCar;
	GLuint redCar;
	GLuint purpleCar;

	GLuint airplane;

	GLuint road;
	GLuint gasStation;
	GLuint alien;
	GLuint grid;
	GLuint lighPole;
	
}


GLuint program;
GLuint programSun;
GLuint programTex;
GLuint programCubeMap;


Core::Shader_Loader shaderLoader;

Core::RenderContext planeContext;
Core::RenderContext sphereContext;
Core::RenderContext carContext;
Core::RenderContext cubeContext;
Core::RenderContext gasStationContext;
Core::RenderContext alienContext;
Core::RenderContext lightPoleContext;


glm::vec3 cameraPos = glm::vec3(-4.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

glm::vec3 planePos = glm::vec3(-4.f, 0, 0);
glm::vec3 planeDir = glm::vec3(1.f, 0.f, 0.f);
GLuint VAO,VBO;

unsigned int textureID;
std::vector<std::string> textures_faces = { "./textures/skybox/negz.jpg","./textures/skybox/posz.jpg" ,"./textures/skybox/posy.jpg" ,"./textures/skybox/negy.jpg" ,"./textures/skybox/posx.jpg" ,"./textures/skybox/negx.jpg" };


float aspectRatio = 1.f;

float x = 0.f;
float z = 0.f;

float progMIN = 0.f;
float progMAX = 14.f;
bool czyXdodawac = TRUE;
bool czyXodejmowac = FALSE;

bool czyZdodawac = FALSE;
bool czyZodejmowac = FALSE;
float k = 180.f;

glm::vec3 transVec(float odchyl, float predkosc) {

	if (czyXdodawac) {
		x+= predkosc;
	}
	else if (czyXodejmowac) {
		x-= predkosc;
	}
	else if (czyZdodawac) {
		z+= predkosc;
	}
	else if (czyZodejmowac) {
		z-= predkosc;
	}

	if (x >= progMAX + odchyl && czyXdodawac) {
		czyXdodawac = FALSE;
		czyZdodawac = TRUE;
	}

	else if (x <= progMIN - odchyl && czyXodejmowac) {
		czyXodejmowac = FALSE;
		czyZodejmowac = TRUE;
	}

	else if (z >= progMAX + odchyl && czyZdodawac) {
		czyZdodawac = FALSE;
		czyXodejmowac = TRUE;;
	}

	else if (z <= progMIN - odchyl && czyZodejmowac) {
		czyXdodawac = TRUE;
		czyZodejmowac = FALSE;
	}

	if (k == 180.f || k == 0.f){
		return glm::vec3(x + odchyl, 0.0, z);
	}
	else {
		return glm::vec3(x, 0.0, z + odchyl);
	}
}


float kat() {

	if (czyXdodawac == TRUE) {
		k = 90.f;
	}
	else if (czyXodejmowac == TRUE) {
		k = 270.f;
	}
	else if (czyZdodawac == TRUE) {
		k = 0.f;
	}
	else if (czyZodejmowac == TRUE) {
		k = 180.f;
	}

	return k;
}



glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir,glm::vec3(0.f,1.f,0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide,cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix()
{
	
	glm::mat4 perspectiveMatrix;
	float n = 0.20;
	float f = 35.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f+n) / (n - f),2*f * n / (n - f),
		0.,0.,-1.,0.,
		});

	
	perspectiveMatrix=glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

void drawObjectSun(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {

	glUseProgram(programSun);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programSun, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programSun, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(programSun, "lightPos"), 0, 0, 0);
	Core::DrawContext(context);

}

void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {


	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);


	glm::vec3 lightColor = glm::vec3(0.8, 0.8, 0.0);
	glm::normalize(lightColor);
	glUniform3f(glGetUniformLocation(program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glm::vec3 lightPos = glm::vec3(6.0, 12.0, 6.0);
	glUniform3f(glGetUniformLocation(program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	glUniform1f(glGetUniformLocation(program, "lightIntensity"), 1.9f);

	glUniform3f(glGetUniformLocation(program, "spotPos"), 19.5, 4.5, 8.0); 
	glUniform3f(glGetUniformLocation(program, "spotDir"), 0.0, -1.0, 0.0);
	glUniform1f(glGetUniformLocation(program, "phi"), 180.0f);
	glUniform1f(glGetUniformLocation(program, "spotIntensity"), 12.0f);

	Core::DrawContext(context);

}






void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID) {

	glUseProgram(program);
	glUseProgram(programTex);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programTex, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform1f(glGetUniformLocation(program, "lightIntensity"), 0.9f);


	glm::vec3 lightColor = glm::vec3(0.8, 0.8, 0.0);
	glm::normalize(lightColor);
	glUniform3f(glGetUniformLocation(program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glm::vec3 lightPos = glm::vec3(6.0, 2.0, 6.0);
	glUniform3f(glGetUniformLocation(program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	Core::SetActiveTexture(textureID, "colorTexture", programTex, 0);



	glUniform3f(glGetUniformLocation(program, "spotPos"), 19.5, 4.5, 8.0);
	glUniform3f(glGetUniformLocation(program, "spotDir"), 0.0, -1.0, 0.0);

	glUniform3f(glGetUniformLocation(program, "spotPos2"), -4.3, -0.5, 8.0);

	glUniform1f(glGetUniformLocation(program, "phi"), 180.0f);
	glUniform1f(glGetUniformLocation(program, "spotIntensity"), 12.0f);

	Core::SetActiveTexture(textureID, "colorTexture", programTex, 0);

	
	Core::DrawContext(context);
}



void drawObjectCube(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID) {
	glUseProgram(programCubeMap);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programCubeMap, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	Core::DrawContext(context);
}




void renderScene(GLFWwindow* window)
{

	glDisable(GL_DEPTH_TEST);
	
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();
	//float timer = 1.f;
	
	

	drawObjectCube(cubeContext, glm::translate(glm::vec3(6.0, 3.0, 6.5)) * glm::scale(glm::vec3(1.4)), textureID);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(program);

	drawObjectSun(sphereContext, glm::translate(glm::vec3(6.0, 12.0, 6.0)), glm::vec3(1.0,1.0,0.1));
	drawObjectTexture(gasStationContext, glm::translate(glm::vec3(17.0, 0.0, -3.0)) * glm::scale(glm::vec3(0.009)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(-90.f), glm::vec3(0, 0, 1)), texture::gasStation);

	//drawObjectTexture(lightPoleContext, glm::translate(glm::vec3(-4.3, -0.5, 8.0)) * glm::scale(glm::vec3(0.5)), texture::lighPole);
	drawObjectTexture(lightPoleContext, glm::translate(glm::vec3(19.5, -0.5, 8.0)) * glm::scale(glm::vec3(0.5)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)), texture::lighPole);


	drawObjectTexture(alienContext, glm::translate(glm::vec3(5.0, 0.5 + (sin(time*20)/2), -3.0)) * glm::scale(glm::vec3(0.02)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)), texture::alien);

	drawObjectTexture(carContext, glm::translate(transVec(0, 0.1)) * glm::translate(glm::vec3(-2.5, 0.0, 2.0)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(kat()), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.08f)), texture::blueCar);

	drawObjectTexture(carContext, glm::translate(transVec(3.0, 0.029)) * glm::translate(glm::vec3(-2.5, 0.0, 2.0)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(kat()), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.08f)), texture::redCar);

	drawObjectTexture(carContext, glm::translate(transVec(6, 0.025)) * glm::translate(glm::vec3(-2.5, 0.0, 2.0)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(kat()), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.08f)), texture::purpleCar);
	drawObjectTexture(cubeContext, glm::translate(glm::vec3(6.5, -0.1, 6.5)) * glm::scale(glm::vec3(1.4f, 0.02f, 1.4f)), texture::road);



	glm::vec3 planeSide = glm::normalize(glm::cross(planeDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 planeUp = glm::normalize(glm::cross(planeSide, planeDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		planeSide.x,planeSide.y,planeSide.z,0,
		planeUp.x,planeUp.y,planeUp.z ,0,
		-planeDir.x,-planeDir.y,-planeDir.z,0,
		0.,0.,0.,1.,
		});


	drawObjectTexture(planeContext,
		glm::translate(planePos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.1f)),
		texture::airplane
	);

	

	glUseProgram(0);
	glfwSwapBuffers(window);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void init(GLFWwindow* window)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	unsigned char* data;
	for (unsigned int i = 0; i < textures_faces.size(); i++)
	{
		data = stbi_load(textures_faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	program = shaderLoader.CreateProgram("shaders/shader_main.vert", "shaders/shader_main.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");
	programCubeMap = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");


	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/RC_ Car_.obj", carContext);
	loadModelToContext("./models/11665_Airplane_v1_l3.obj", planeContext); 
	loadModelToContext("./models/cube.obj", cubeContext);
	loadModelToContext("./models/Gas Station_V4_L3.obj", gasStationContext);
	loadModelToContext("./models/alien.obj", alienContext);
	loadModelToContext("./models/Street Light.obj", lightPoleContext);



	texture::grid = Core::LoadTexture("./textures/grid.png");
	texture::blueCar = Core::LoadTexture("./textures/RC_ Car_Blue.jpg");
	texture::redCar = Core::LoadTexture("./textures/RC_ Car_Red.png");
	texture::airplane = Core::LoadTexture("./textures/11665_Airplane_diff.jpg");
	texture::purpleCar = Core::LoadTexture("./textures/RC_ Car_Purple.png");
	texture::road = Core::LoadTexture("./textures/road_road_0021_03_tiled.jpg");
	texture::gasStation = Core::LoadTexture("./textures/gasStation.JPG");
	texture::alien = Core::LoadTexture("./textures/alien.jpg");
	texture::lighPole = Core::LoadTexture("./textures/Light Pole.png");
}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}


void processInput(GLFWwindow* window)
{
	glm::vec3 planeSide = glm::normalize(glm::cross(planeDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 planeUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.05f;
	float moveSpeed = 0.15f;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		planePos += planeDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		planePos -= planeDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		planePos += planeSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		planePos -= planeSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		planePos += planeUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		planePos -= planeUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		planeDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(planeDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		planeDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(planeDir, 0));

	cameraPos = planePos - 1.5 * planeDir + glm::vec3(0, 1, 0) * 0.5f;
	cameraDir = planeDir;

}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}
//}