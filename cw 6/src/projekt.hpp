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
	GLuint cat;
	GLuint alien;
	GLuint grid;
	GLuint lighPole;
	GLuint tank;
	GLuint rustedCar;
	GLuint airplane2;
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
Core::RenderContext catContext;
Core::RenderContext alienContext;
Core::RenderContext lightPoleContext;
Core::RenderContext tankContext;
Core::RenderContext rustedCar;
Core::RenderContext airplane2Context;

glm::vec3 cameraPos = glm::vec3(-4.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

//Połozenie i wektor kierunku samolotu (x,y,z)
glm::vec3 planePos = glm::vec3(-4.f, 10, 0);
glm::vec3 planeDir = glm::vec3(1.f, 0.f, 0.f);

//textury skyboxa, powiny być w trochę innej kolejności ale i tak jest ok
unsigned int textureID;
std::vector<std::string> textures_faces = { "./textures/skybox/negz.jpg","./textures/skybox/posz.jpg" ,"./textures/skybox/posy.jpg" ,"./textures/skybox/negy.jpg" ,"./textures/skybox/posx.jpg" ,"./textures/skybox/negx.jpg" };

float aspectRatio = 1.f;

float x = 0.f;
float z = 0.f;

float k = 180.f;

//to jest od kamery, zrobione identycznie jak w tym pooradniku na stronie
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

//Moj komentarz macierz perspektywy Aby dostosować zasięg widzenia, 
// możesz zmienić wartości n (bliska płaszczyzna przycinania) i f (daleka płaszczyzna przycinania). 
// Zwiększając wartość f lub zmniejszając wartość n, zasięg widzenia będzie większy, co pozwoli na oglądanie odleglejszych obiektów.
glm::mat4 createPerspectiveMatrix()
{
	glm::mat4 perspectiveMatrix;
	float n = 0.01; // Zmniejszenie wartości n
	float f = 1000.0; // Zwiększenie wartości f
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1, 0., 0., 0.,
		0., aspectRatio, 0., 0.,
		0., 0., (f + n) / (n - f), 2 * f * n / (n - f),
		0., 0., -1., 0.,
		});

	perspectiveMatrix = glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

//z tego co pamiętam to te wszystkie drawObject są do siebie bardzo podobne, różnią się chyba tylko shaderami i światłem

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

//wszystko inne
//Core::RenderContext& context to jest obiekt który renderujesz
//glm::mat4 modelMatrix to jest pozycja obiektu i jak on się porusza
//GLuint textureID textura którą nakładasz

void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID) {

	//te dwie linijki na dole to wybranie shadera którego chcesz użyć w tej funckji, shadery ładujesz na dole w init() do zmiennych GLuint
	glUseProgram(program);
	glUseProgram(programTex);

	//te dwie na dole to obliczają pozycję obiektu 
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	//wszyastkie te które zaczynają się od glUniform przekazują zmienne do shaderów, na przykład ta pierwsza przekazuje wartości do shadera zapisanego w zmiennej programTex, do zmiennej tranformation
	glUniformMatrix4fv(glGetUniformLocation(programTex, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	//Kolor świecenie słońca, glm::vec3(1.0, 0, 0) - to bedzie czerwony.
	glm::vec3 lightColor = glm::vec3(0.8, 0.8, 0.0);
	glUniform3f(glGetUniformLocation(program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	//Pozycja światła ze słońca
	glm::vec3 lightPos = glm::vec3(6.0, 12.0, 6.0);
	glUniform3f(glGetUniformLocation(program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	//Swiecenie latarni
	glUniform3f(glGetUniformLocation(program, "spotPos"), 16.0, 4.5, 8.0);
	glUniform3f(glGetUniformLocation(program, "spotDir"), 0.0, -1.0, 0.0);
	glUniform1f(glGetUniformLocation(program, "phi"), 180.0f);
	glUniform1f(glGetUniformLocation(program, "spotIntensity"), 12.0f);

	//tu odpalasz texturę i potem obiekt
	Core::SetActiveTexture(textureID, "colorTexture", programTex, 0);
	Core::DrawContext(context);
}

void renderScene(GLFWwindow* window)
{
	//kolejność następnych 5 linijek kodu jest nieprzypadkowa, nie pamiętam dokładnie dlaczego ale tak ma być bo w innej kolejności to nie działało dobrze
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//to jest skyBox, on powinien zmieniać swoją pozycję razem z samolotem
	drawObjectCube(cubeContext, glm::translate(planePos), textureID);
	glEnable(GL_DEPTH_TEST);

	float time = glfwGetTime();

	//slonce, w 3 parametrze mozna zmienic kolor np vec3(1.0, 0.0, 0.0) to czerwony.
	drawObjectSun(sphereContext, glm::translate(glm::vec3(6.0, 25.0, 6.0)) * glm::scale(glm::vec3(10.0)), glm::vec3(1.0, 1.0, 0.1));

	//kot
	drawObjectTexture(catContext, glm::translate(glm::vec3(15, 0, 8)) * glm::scale(glm::vec3(0.03f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)), texture::cat);

	//latarnia
	drawObjectTexture(lightPoleContext, glm::translate(glm::vec3(16.0, -0.5, 8.0)) * glm::scale(glm::vec3(0.8)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)), texture::lighPole);

	//kosmita
	drawObjectTexture(alienContext, glm::translate(glm::vec3(15.0, 0.5 + (sin(time * 20) / 2), 8.0)) * glm::scale(glm::vec3(0.02)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)), texture::alien);

	//podłoga
	drawObjectTexture(cubeContext, glm::translate(glm::vec3(6.5, -0.1, 6.5)) * glm::scale(glm::vec3(2.0f, 0.02f, 1.4f)), texture::road);

	//zbiornik tank
	drawObjectTexture(tankContext, glm::translate(glm::vec3(18, 0, 14)) * glm::scale(glm::vec3(2)) * glm::rotate(glm::radians(-30.0f), glm::vec3(0, 1, 0)), texture::tank);

	//wrak auta
	drawObjectTexture(rustedCar, glm::translate(glm::vec3(10, 0, 0)) * glm::scale(glm::vec3(0.33f)), texture::rustedCar);

	//wark samolotu
	drawObjectTexture(airplane2Context, glm::translate(glm::vec3(16, 0, 0)) * glm::scale(glm::vec3(0.7f)) * glm::scale(glm::vec3(0.02)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(30.0f), glm::vec3(0, 0, 1)), texture::rustedCar);

	// to jest od kamery i sterowania, tak samo jak w poradniku zrobione
	glm::vec3 planeSide = glm::normalize(glm::cross(planeDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 planeUp = glm::normalize(glm::cross(planeSide, planeDir));
	glm::mat4 planeCameraRotrationMatrix = glm::mat4({
		planeSide.x,planeSide.y,planeSide.z,0,
		planeUp.x,planeUp.y,planeUp.z ,0,
		-planeDir.x,-planeDir.y,-planeDir.z,0,
		0.,0.,0.,1.,
		});

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

	//tu są ładowane te shadery 
	program = shaderLoader.CreateProgram("shaders/shader_main.vert", "shaders/shader_main.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");
	programCubeMap = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");

	//tu są ładowane modele, zajebane z internetu darmowe pierwsze lepsze co działały i umiałem je dodać, 
	//jeśli będziesz je zmieniał to szukaj takich które mają dodane textury jako jeden plik jpg lub png, często są dodane textury jako kilka osobnych plików i ja nie wiem jak to dobrze zrobić żeby to ładnie wyglądało, najłatwiej mieć takie obiekty które mają jeden prosty obrazek
	//pamiętaj też że różne obiekty są ustawione domyślnie pod różnym kątem i mają różne wielkości, jeśli załadujesz nowy obiekt to pewnie będziesz musiał go odpowiednio obrócić i zmienić jego rozmiar
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

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}