#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/SOIL2.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <time.h>
#include <random>

#include "cShaderProgram.h"
#include "cCamera.h"
#include "cMesh.h"
#include "cModel.h"
#include "cSkybox.h"
#include "cGameObject.h"

//Setting up a camera GLOBAL
cCamera Camera(glm::vec3(0.0f, 25.0f, 45.0f),		//Camera Position
			glm::vec3(0.0f, 1.0f, 0.0f),		//World Up vector
			-30.0f,								//Pitch
			-90.0f);							//Yaw
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 400, lastY = 300;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int playFieldX = 40;
int playFieldZ = 40;

std::map<std::string, cModel*> mapModelsToNames;
std::vector<cGameObject*> players;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Welcome new player", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialzed GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	cShaderProgram myProgram;
	myProgram.compileProgram("assets/shaders/", "vertShader.glsl", "fragShader.glsl");

	cShaderProgram skyBoxProgram;
	skyBoxProgram.compileProgram("assets/shaders/", "skyBoxVert.glsl", "skyBoxFrag.glsl");

	glUseProgram(myProgram.ID);

	glm::vec3 playerPos(0.0f);
	float playerSpeed = 1.0f;
	int enemyCount = 0;
	float avgEnemySpeed = 1.0f;

	std::ifstream Config("assets/config.txt");
	if (Config.is_open())
	{
		std::string curLine;
		Config >> curLine;	//Map Dimensions
		Config >> curLine;	//X=
		Config >> curLine;	
		playFieldX = std::stoi(curLine);
		Config >> curLine;	//Z=
		Config >> curLine;
		playFieldZ = std::stoi(curLine);
		Config >> curLine;	//Camera Offset
		Config >> curLine;	//Y=
		Config >> curLine;
		Camera.position.y = std::stof(curLine);
		Config >> curLine;	//Z=
		Config >> curLine;
		Camera.position.z = std::stof(curLine);
		Config >> curLine;	//Player
		Config >> curLine;	//Position:
		Config >> curLine;
		playerPos.x = std::stof(curLine);
		Config >> curLine;
		playerPos.y = std::stof(curLine);
		Config >> curLine;
		playerPos.z = std::stof(curLine);
		Config >> curLine;	//Speed:
		Config >> curLine;
		playerSpeed = std::stof(curLine);
		Config >> curLine;	//Enemies:
		Config >> curLine;	//Count:
		Config >> curLine;
		enemyCount = std::stoi(curLine);
		Config >> curLine;	//Speed:
		Config >> curLine;
		avgEnemySpeed = std::stof(curLine);
	}

	std::string path = "assets/models/nanosuit/nanosuit.obj";
	mapModelsToNames["Nanosuit"] = new cModel(path);

	path = "assets/models/floor/floor.obj";
	mapModelsToNames["Floor"] = new cModel(path);

	path = "assets/models/apple/apple textured obj.obj";
	mapModelsToNames["Apple"] = new cModel(path);

	path = "assets/models/banana/banana.obj";
	mapModelsToNames["Banana"] = new cModel(path);

	path = "assets/models/pumpkin/PumpkinOBJ.obj";
	mapModelsToNames["Pumpkin"] = new cModel(path);

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

	cSkybox skybox("assets/textures/skybox/");

	cGameObject* player = new cGameObject();
	player->modelName = "Nanosuit";
	player->position = playerPos;
	player->speed = playerSpeed;
	player->scale = 0.2f;
	player->aiType = cGameObject::PLAYER;
	player->hitRadius = 1.0f;
	player->health = 50.0f;
	players.push_back(player);

	srand(time(NULL));

	for (int i = 0; i < enemyCount; i++)
	{
		int enemyType = rand() % 3;
		player = new cGameObject();

		player->attentionRadius = (playFieldX + playFieldZ) / 2 + 5.0f;
		player->speed = avgEnemySpeed;
		int randomXPos = rand() % (playFieldX * 10);
		randomXPos -= (playFieldX * 5);
		float fXPos = (float)randomXPos / 10.0f;
		int randomZPos = rand() % (playFieldZ * 10);
		randomZPos -= (playFieldZ * 5);
		float fZPos = (float)randomZPos / 10.0f;
		player->position.x = fXPos;
		player->position.z = fZPos;

		if (enemyType == 0)
		{
			player->modelName = "Apple";
			player->aiType = cGameObject::ANGRY;
			player->scale = 0.02f;
		}

		else if (enemyType == 1)
		{
			player->modelName = "Banana";
			player->scale = 0.4f;
			player->aiType = cGameObject::CURIOUS;
		}

		else
		{
			player->modelName = "Pumpkin";
			player->scale = 0.01f;
			player->position.y = 0.4f;
			player->aiType = cGameObject::FOLLOWER;
			int randFollow = rand() % players.size();
			player->followTarget = players[randFollow];
		}

		players.push_back(player);
	}

	playFieldX = playFieldX / 2;
	playFieldZ = playFieldZ / 2;

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	skyBoxProgram.setInt("skybox", skybox.textureID);
	myProgram.setInt("skybox", 0);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glEnable(GL_DEPTH_TEST);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//All the rendering commands go in here
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection;
		glm::mat4 view;

		glUseProgram(myProgram.ID);

		projection = glm::perspective(glm::radians(Camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = Camera.getViewMatrix();

		// view/projection transformations
		myProgram.setMat4("projection", projection);
		myProgram.setMat4("view", view);

		//http://devernay.free.fr/cours/opengl/materials.html
		myProgram.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		myProgram.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		myProgram.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		myProgram.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

		myProgram.setVec3("pointLights[0].position", pointLightPositions[0]);
		myProgram.setFloat("pointLights[0].constant", 1.0f);
		myProgram.setFloat("pointLights[0].linear", 0.09f);
		myProgram.setFloat("pointLights[0].quadratic", 0.032f);
		myProgram.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		myProgram.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		myProgram.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);

		myProgram.setVec3("pointLights[1].position", pointLightPositions[1]);
		myProgram.setFloat("pointLights[1].constant", 1.0f);
		myProgram.setFloat("pointLights[1].linear", 0.09f);
		myProgram.setFloat("pointLights[1].quadratic", 0.032f);
		myProgram.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		myProgram.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		myProgram.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);

		myProgram.setVec3("pointLights[2].position", pointLightPositions[2]);
		myProgram.setFloat("pointLights[2].constant", 1.0f);
		myProgram.setFloat("pointLights[2].linear", 0.09f);
		myProgram.setFloat("pointLights[2].quadratic", 0.032f);
		myProgram.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		myProgram.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		myProgram.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);

		myProgram.setVec3("pointLights[3].position", pointLightPositions[3]);
		myProgram.setFloat("pointLights[3].constant", 1.0f);
		myProgram.setFloat("pointLights[3].linear", 0.09f);
		myProgram.setFloat("pointLights[3].quadratic", 0.032f);
		myProgram.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		myProgram.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		myProgram.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);

		myProgram.setVec3("spotLight.position", Camera.position);
		myProgram.setVec3("spotLight.direction", Camera.front);
		myProgram.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		myProgram.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
		myProgram.setFloat("spotLight.constant", 1.0f);
		myProgram.setFloat("spotLight.linear", 0.09f);
		myProgram.setFloat("spotLight.quadratic", 0.032f);
		myProgram.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		myProgram.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		myProgram.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		//http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation

		glm::mat4 identity(1.0f);

		glm::mat4 model = identity;
		model = glm::scale(model, glm::vec3(playFieldX, 1.0f, playFieldZ));
		myProgram.setMat4("model", model);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		mapModelsToNames["Floor"]->Draw(myProgram);

		for (int index = 0; index < players.size(); index++)
		{
			//glm::mat4 model(1.0f);
			model = identity;
			model = glm::translate(model, players[index]->position);
			model = glm::rotate(model, players[index]->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, players[index]->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, players[index]->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, glm::vec3(players[index]->scale));
			myProgram.setMat4("model", model);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);

			//int closestOne = 0;
			//float closestDistance = 99.9f;
			//if (players[index]->aiType == cGameObject::AIType::FOLLOWER)
			//{
			//	for (int otherIndex = 0; otherIndex < players.size(); otherIndex++)
			//	{
			//		if (otherIndex == index || players[otherIndex]->aiType == cGameObject::AIType::FOLLOWER)
			//			continue;
			//		float thisDistance = glm::distance(players[index]->position, players[otherIndex]->position);
			//		if (thisDistance < closestDistance)
			//		{
			//			closestDistance = thisDistance;
			//			closestOne = otherIndex;
			//		}
			//	}
			//}
			//players[index]->followTarget = players[closestOne];

			if (players[index]->specialState)
			{
				if (players[index]->aiType == cGameObject::AIType::ANGRY)
				{
					myProgram.setVec3("enemyStateColor", glm::vec3(0.4f, 0.0f, 0.0f));
				}
				else if (players[index]->aiType == cGameObject::AIType::CURIOUS)
				{
					myProgram.setVec3("enemyStateColor", glm::vec3(0.53f, 0.81f, 0.88f));
				}
				else if (players[index]->aiType == cGameObject::AIType::FOLLOWER)
				{
					myProgram.setVec3("enemyStateColor", glm::vec3(1.0f, 1.0f, 0.0f));
				}
				else if (players[index]->aiType == cGameObject::AIType::PLAYER)
				{
					myProgram.setVec3("enemyStateColor", glm::vec3(0.9f, 0.9f, 0.9f));
				}
			}
			mapModelsToNames[players[index]->modelName]->Draw(myProgram);

			myProgram.setVec3("enemyStateColor", glm::vec3(0.0f, 0.0f, 0.0f));

			players[index]->handleAI(deltaTime, playFieldX, playFieldZ, players[0]);
		}

		glUseProgram(skyBoxProgram.ID);

		view = glm::mat4(glm::mat3(Camera.getViewMatrix()));
		projection = glm::perspective(glm::radians(Camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);

		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyBoxProgram.setMat4("projection", projection);
		skyBoxProgram.setMat4("view", view);

		glBindVertexArray(skybox.VAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS); // set depth function back to default

		std::string playerHealth = "Player Health: " + std::to_string(players[0]->health);
		glfwSetWindowTitle(window, playerHealth.c_str());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);

	if (players[0]->health > 0)
	{
		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		{
			players[0]->Movement(cGameObject::FORWARD, deltaTime, playFieldX, playFieldZ);
		}
		else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		{
			players[0]->Movement(cGameObject::BACKWARD, deltaTime, playFieldX, playFieldZ);
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		{
			players[0]->Movement(cGameObject::LEFT, deltaTime, playFieldX, playFieldZ);
		}
		else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		{
			players[0]->Movement(cGameObject::RIGHT, deltaTime, playFieldX, playFieldZ);
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // this bool variable is initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	Camera.processMouseMovement(xOffset, yOffset, true);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera.processMouseScroll(yoffset);
}