﻿#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>


namespace texture {
	GLuint earth;
	GLuint clouds;
	GLuint moon;
	GLuint ship;
	GLuint road;
	GLuint grid;
	GLuint airplane;

	GLuint earthNormal;
	GLuint asteroidNormal;
	GLuint shipNormal;
}


GLuint program;
GLuint programSun;
GLuint programTex;
Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext cubeContext;
Core::RenderContext planeContext;

glm::vec3 cameraPos = glm::vec3(-4.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

glm::vec3 spaceshipPos = glm::vec3(-4.f, 0, 0);
glm::vec3 spaceshipDir = glm::vec3(1.f, 0.f, 0.f);
GLuint VAO, VBO;

float aspectRatio = 1.f;

//textury skyboxa, powiny być w trochę innej kolejności ale i tak jest ok
unsigned int textureID;
std::vector<std::string> textures_faces = { "./textures/skybox/negz.jpg","./textures/skybox/posz.jpg" ,"./textures/skybox/posy.jpg" ,"./textures/skybox/negy.jpg" ,"./textures/skybox/posx.jpg" ,"./textures/skybox/negx.jpg" };

glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
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
	float n = 0.05;
	float f = 20.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f + n) / (n - f),2 * f * n / (n - f),
		0.,0.,-1.,0.,
		});


	perspectiveMatrix = glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {

	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightPos"), 0, 0, 0);
	Core::DrawContext(context);

}

//słońce
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

//skybox
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
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();


	glUseProgram(program);

	drawObjectColor(sphereContext, glm::mat4(), glm::vec3(1.0, 1.0, 0.3));

	drawObjectColor(sphereContext, glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::scale(glm::vec3(0.3f)), glm::vec3(0.2, 0.7, 0.3));

	drawObjectColor(sphereContext,
		glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::eulerAngleY(time) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.1f)),
		glm::vec3(0.5, 0.5, 0.5));

	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});


	//drawObjectColor(shipContext,
	//	glm::translate(cameraPos + 1.5 * cameraDir + cameraUp * -0.5f) * inveseCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()),
	//	glm::vec3(0.3, 0.3, 0.5)
	//	);
	drawObjectColor(shipContext,
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()),
		glm::vec3(0.3, 0.3, 0.5)
	);

	float time = glfwGetTime();

	//slonce, w 3 parametrze mozna zmienic kolor np vec3(1.0, 0.0, 0.0) to czerwony.
	drawObjectSun(sphereContext, glm::translate(glm::vec3(6.0, 25.0, 6.0)) * glm::scale(glm::vec3(10.0)), glm::vec3(1.0, 1.0, 0.1));

	//kosmita
	drawObjectTexture(alienContext, glm::translate(glm::vec3(15.0, 0.5 + (sin(time*20)/2), 8.0)) * glm::scale(glm::vec3(0.02)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)), texture::alien);

	//zbiornik tank
	drawObjectTexture(tankContext, glm::translate(glm::vec3(18, 0, 14)) * glm::scale(glm::vec3(2)) * glm::rotate(glm::radians(-30.0f), glm::vec3(0, 1, 0)), texture::tank);

	//wrak auta
	drawObjectTexture(rustedCar, glm::translate(glm::vec3(10, 0, 0)) * glm::scale(glm::vec3(0.33f)), texture::rustedCar);

	//wark samolotu
	drawObjectTexture(airplane2Context, glm::translate(glm::vec3(16, 0, 0)) * glm::scale(glm::vec3(0.7f)) * glm::scale(glm::vec3(0.02)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(30.0f), glm::vec3(0, 0, 1)), texture::rustedCar);
	
	//road
	drawObjectTexture(cubeContext, glm::translate(glm::vec3(6.5, -0.1, 6.5)) * glm::scale(glm::vec3(2.0f, 0.02f, 1.4f)), texture::road);

	//kot
	drawObjectTexture(catContext, glm::translate(glm::vec3(15, 0, 8)) * glm::scale(glm::vec3(0.03f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)), texture::cat);

	//wark samolotu
	drawObjectTexture(airplane2Context, glm::translate(glm::vec3(16, 0, 0)) * glm::scale(glm::vec3(0.7f)) * glm::scale(glm::vec3(0.02)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(30.0f), glm::vec3(0, 0, 1)), texture::rustedCar);

	//samolot którym sterujesz
	drawObjectTexture(planeContext,
		glm::translate(planePos)
		* planeCameraRotrationMatrix //trzeba przemnożyć przez kamere bo inaczej kamera będzie sie poruszac a obiekt, nie
		* glm::eulerAngleY(glm::pi<float>())
		* glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) // Obrót o -90 stopni wokół osi X
		* glm::rotate(glm::radians(180.f), glm::vec3(0, 0, 1))
		* glm::scale(glm::vec3(0.1)),
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
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_5_1.vert", "shaders/shader_5_1.frag");

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/10593_Fighter_Jet_SG_v1_iterations-2.obj", planeContext);
	loadModelToContext("./models/cube.obj", cubeContext);
	loadModelToContext("./models/12221_Cat_v1_l3.obj", catContext);
	loadModelToContext("./models/alien.obj", alienContext);
	loadModelToContext("./models/Street Light.obj", lightPoleContext);
	loadModelToContext("./models/Water_Tank_BI.obj", tankContext);
	loadModelToContext("./models/Rusted Car.obj", rustedCar);
	loadModelToContext("./models/11804_Airplane_v2_l2.obj", airplane2Context);

	// tu są ładwowane textury
	texture::grid = Core::LoadTexture("./textures/grid.png");
	texture::airplane = Core::LoadTexture("./textures/10593_Fighter_Jet_SG_v1_diffuse.jpg");
	texture::road = Core::LoadTexture("./textures/road.jpg");
	texture::cat = Core::LoadTexture("./textures/Cat_diffuse.JPG");
	texture::alien = Core::LoadTexture("./textures/alien.jpg");
	texture::lighPole = Core::LoadTexture("./textures/Light Pole.png");
	texture::tank = Core::LoadTexture("./textures/water_tank_col.png");
	texture::rustedCar = Core::LoadTexture("./textures/Car Uv.png");
	texture::airplane2 = Core::LoadTexture("./textures/Car Uv.jpg");


}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}

//sterowanie samolotem, chyba wszystko jest tak samo jak w poradniku, tylko nazwy zmiennych zmieniłem żeby udawać że sam to robiłem 
void processInput(GLFWwindow* window)
{
	glm::vec3 planeSide = glm::normalize(glm::cross(planeDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 planeUp = glm::vec3(0.f, 1.f, 0.f);

	// Zmniejszenie prędkości obrotu
	float angleSpeed = 0.005f;

	float moveSpeed = 0.15f;

	// Obrót wokół osi X (góra/dół)
	float pitch = 0.0f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		pitch += angleSpeed;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		pitch -= angleSpeed;

	// Obrót wokół osi Y (lewo/prawo)
	float yaw = 0.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		yaw += angleSpeed;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		yaw -= angleSpeed;

	// Aktualizacja kierunku samolotu na podstawie obrótów
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), pitch, planeSide);
	rotationMatrix = glm::rotate(rotationMatrix, yaw, planeUp);
	planeDir = glm::vec3(rotationMatrix * glm::vec4(planeDir, 0.0f));

	// Przesunięcie samolotu w kierunku zgodnym z jego kierunkiem
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		planePos += planeDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		planePos -= planeDir * moveSpeed;

	// Aktualizacja pozycji kamery
	cameraPos = planePos - 1.5f * planeDir + glm::vec3(0, 1, 0) * 0.5f;
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