#ifndef _HG_cGameObject_
#define _HG_cGameObject_

#define _USE_MATH_DEFINES
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <math.h>

class cGameObject
{
public:
	enum AIType
	{
		PLAYER,
		ANGRY,
		CURIOUS,
		FOLLOWER
	};

	enum Direction
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	glm::vec3 position;
	glm::vec3 initialRotation;
	glm::vec3 rotation;
	glm::vec3 forward;
	float scale;
	float speed;
	int health;
	float hitRadius;
	float attentionRadius;
	float iFrames;
	AIType aiType;
	bool specialState;
	std::string modelName;
	cGameObject* followTarget;

	cGameObject();
	void Movement(Direction direction, float deltaTime, float playFieldX, float playFieldZ);
	void handleAI(float deltaTime, float playFieldX, float playFieldZ, cGameObject* player);

private:
	float playerFacingRelativeToMe(cGameObject* player);
	float meFacingRelativeToPlayer(cGameObject* player);
	bool turnLeftOrRight(cGameObject* player, float currentAngle, float deltaTime);
};


#endif