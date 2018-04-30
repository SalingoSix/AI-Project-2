#include "cGameObject.h"

cGameObject::cGameObject()
{
	position = glm::vec3(0.0f);
	initialRotation = glm::vec3(0.0f);
	forward = glm::vec3(0.0f, 0.0f, 1.0f);
	speed = 3.0f;
	rotation = glm::vec3(0.0f);
	specialState = false;
	scale = 1.0f;
	health = 100;
	hitRadius = 1.0f;
	attentionRadius = 100.0f;
	aiType = FOLLOWER;
	modelName = "";
	iFrames = 0.0f;
	followTarget = NULL;
}

void cGameObject::Movement(Direction direction, float deltaTime, float playFieldX, float playFieldZ)
{
	float mySpeed = this->speed;
	if (this->iFrames > 0.0f)
	{
		mySpeed *= 2.5;
	}
	if (direction == FORWARD)
	{
		position += forward * deltaTime * mySpeed;

		if (position.x >= playFieldX)
		{
			position.x = playFieldX;
		}
		else if (position.x <= -playFieldX)
		{
			position.x = -playFieldX;
		}
		if (position.z >= playFieldZ)
		{
			position.z = playFieldZ;
		}
		else if (position.z <= -playFieldZ)
		{
			position.z = -playFieldZ;
		}
	}

	else if (direction == BACKWARD)
	{
		position -= forward * deltaTime * mySpeed;

		if (position.x >= playFieldX)
		{
			position.x = playFieldX;
		}
		else if (position.x <= -playFieldX)
		{
			position.x = -playFieldX;
		}
		if (position.z >= playFieldZ)
		{
			position.z = playFieldZ;
		}
		else if (position.z <= -playFieldZ)
		{
			position.z = -playFieldZ;
		}
	}

	else if (direction == LEFT)
	{
		rotation.y += speed * deltaTime;
		forward.z = cos(rotation.y);
		forward.x = sin(rotation.y);
		forward = glm::normalize(forward);
	}

	else if (direction == RIGHT)
	{
		rotation.y -= speed * deltaTime;
		forward.z = cos(rotation.y);
		forward.x = sin(rotation.y);
		forward = glm::normalize(forward);
	}
}

void cGameObject::handleAI(float deltaTime, float playFieldX, float playFieldZ, cGameObject* player)
{
	if (aiType == ANGRY)
	{
		float angleToMe = playerFacingRelativeToMe(player);
		float angleToPlayer = meFacingRelativeToPlayer(player);
		float radians = acos(angleToPlayer);
		float playerDist = glm::distance(player->position, this->position);

		if (player->health < 25)
		{	//Player's health is low, pursue
			specialState = true;
			if (angleToPlayer >= 0.95f)
			{	//Charge directly forward
				Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
			}
			else if (angleToPlayer >= 0.55f)
			{	//Move forward, but continue to orient better
				if (turnLeftOrRight(player, angleToPlayer, deltaTime))
					Movement(LEFT, deltaTime, playFieldX, playFieldZ);
				else
					Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
				Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
			}
			else
			{	//Orient to face player
				if (turnLeftOrRight(player, angleToPlayer, deltaTime))
					Movement(LEFT, deltaTime, playFieldX, playFieldZ);
				else
					Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
			}
		}

		else if (playerDist <= this->attentionRadius)
		{	//Player is within my radius of detection
			if (angleToMe > 0.70f)
			{	//Player is looking at me, evade
				specialState = false;
				if (angleToPlayer <= -0.99f)
				{	//Charge directly forward
					Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
				}
				else if (angleToPlayer <= -0.35f)
				{	//Move forward, but continue to orient better
					if (!turnLeftOrRight(player, angleToPlayer, deltaTime))
						Movement(LEFT, deltaTime, playFieldX, playFieldZ);
					else
						Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
					Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
				}
				else
				{	//Orient to face player
					if (!turnLeftOrRight(player, angleToPlayer, deltaTime))
						Movement(LEFT, deltaTime, playFieldX, playFieldZ);
					else
						Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
				}
			}

			else
			{	//Player is looking away, pursue
				specialState = true;
				if (angleToPlayer >= 0.95f)
				{	//Charge directly forward
					Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
				}
				else if (angleToPlayer >= 0.55f)
				{	//Move forward, but continue to orient better
					if (turnLeftOrRight(player, angleToPlayer, deltaTime))
						Movement(LEFT, deltaTime, playFieldX, playFieldZ);
					else
						Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
					Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
				}
				else
				{	//Orient to face player
					if (turnLeftOrRight(player, angleToPlayer, deltaTime))
						Movement(LEFT, deltaTime, playFieldX, playFieldZ);
					else
						Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
				}
			}
		}
		else
		{	//Player is not within my area of detection, do nothing
			specialState = false;
		}

		//Last, check if you've hit the player
		if (playerDist <= player->hitRadius && player->iFrames == 0.0f)
		{//You've landed a hit
			player->iFrames = 2.5f;
			player->health -= 10;
		}
		
	}
	else if (aiType == CURIOUS)
	{
		float angleToMe = playerFacingRelativeToMe(player);
		float angleToPlayer = meFacingRelativeToPlayer(player);
		float radians = acos(angleToPlayer);
		float playerDist = glm::distance(player->position, this->position);

		if(playerDist <= this->attentionRadius)
		{
			if (angleToMe > 0.70f)
			{	//Player is looking at me, evade
				specialState = false;
				if (angleToPlayer <= -0.99f)
				{	//Charge directly forward
					Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
				}
				else if (angleToPlayer <= -0.35f)
				{	//Move forward, but continue to orient better
					if (!turnLeftOrRight(player, angleToPlayer, deltaTime))
						Movement(LEFT, deltaTime, playFieldX, playFieldZ);
					else
						Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
					Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
				}
				else
				{	//Orient to face player
					if (!turnLeftOrRight(player, angleToPlayer, deltaTime))
						Movement(LEFT, deltaTime, playFieldX, playFieldZ);
					else
						Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
				}
			}

			else if (playerDist <= player->hitRadius + 1.0f)
			{	//Near, but not near enough to damage player, wait

			}

			else
			{	//Player is looking away, arrive
				specialState = true;
				if (angleToPlayer >= 0.95f)
				{	//Charge directly forward
					Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
				}
				else if (angleToPlayer >= 0.55f)
				{	//Move forward, but continue to orient better
					if (turnLeftOrRight(player, angleToPlayer, deltaTime))
						Movement(LEFT, deltaTime, playFieldX, playFieldZ);
					else
						Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
					Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
				}
				else
				{	//Orient to face player
					if (turnLeftOrRight(player, angleToPlayer, deltaTime))
						Movement(LEFT, deltaTime, playFieldX, playFieldZ);
					else
						Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
				}
			}
		}

		else
		{	//Player is not within my area of detection, do nothing
			specialState = false;
		}

		//Last, check if you've hit the player
		if (playerDist <= player->hitRadius && player->iFrames == 0.0f)
		{//You've landed a hit
			player->iFrames = 2.5f;
			player->health -= 10;
		}
	}
	else if (aiType == FOLLOWER)
	{
		float angleToTarget = meFacingRelativeToPlayer(this->followTarget);
		float radians = acos(angleToTarget);
		float playerDist = glm::distance(player->position, this->position);
		float followDist = glm::distance(followTarget->position, this->position);

		if (followDist <= 2.0f)
		{	//Player is looking at me, freeze / I have arrived at my destination
			specialState = false;
		}

		else
		{
			specialState = true;
			if (angleToTarget >= 0.95f)
			{	//Charge directly forward
				Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
			}
			else if (angleToTarget >= 0.55f)
			{	//Move forward, but continue to orient better
				if (turnLeftOrRight(player, angleToTarget, deltaTime))
					Movement(LEFT, deltaTime, playFieldX, playFieldZ);
				else
					Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
				Movement(FORWARD, deltaTime, playFieldX, playFieldZ);
			}
			else
			{	//Orient to face player
				if (turnLeftOrRight(player, angleToTarget, deltaTime))
					Movement(LEFT, deltaTime, playFieldX, playFieldZ);
				else
					Movement(RIGHT, deltaTime, playFieldX, playFieldZ);
			}
		}

		//Last, check if you've hit the player
		if (playerDist <= player->hitRadius && player->iFrames == 0.0f)
		{//You've landed a hit
			player->iFrames = 2.5f;
			player->health -= 10;
		}
	}
	else if (aiType == PLAYER)
	{
		//Check if you have iFrames, and reduce if so
		if (iFrames > 0.0f)
		{
			iFrames -= deltaTime;
		}
		if (iFrames < 0.0f)
		{
			iFrames = 0.0f;
		}

		//Check if you are dead
		if (health <= 0)
		{
			specialState = true;
			position.y += deltaTime * 3;
			Movement(LEFT, deltaTime, playFieldX, playFieldZ);
		}
	}
}

float cGameObject::playerFacingRelativeToMe(cGameObject* player)
{
	//Get the vector between me and the player
	glm::vec3 vecPlayerDistance = this->position - player->position;
	vecPlayerDistance = glm::normalize(vecPlayerDistance);

	//Get the angle between the player's forward, and the distance between us.
	//ie. if it's 0 degrees, he's facing right at me
	//If it's 180, he's facing the opposite way
	float dotProduct = player->forward.x * vecPlayerDistance.x +
						player->forward.y * vecPlayerDistance.y +
						player->forward.z * vecPlayerDistance.z;
	//Cosine of the angle works just as well, only it's between 1 and -1
	float cosTheta = dotProduct / (glm::length(player->forward) * glm::length(vecPlayerDistance));

	return cosTheta;
}

float cGameObject::meFacingRelativeToPlayer(cGameObject* player)
{
	glm::vec3 vecPlayerDistance = player->position - this->position;
	vecPlayerDistance = glm::normalize(vecPlayerDistance);

	float dotProduct = this->forward.x * vecPlayerDistance.x +
		this->forward.y * vecPlayerDistance.y +
		this->forward.z * vecPlayerDistance.z;

	float cosTheta = dotProduct / (glm::length(this->forward) * glm::length(vecPlayerDistance));
	return cosTheta;
}

bool cGameObject::turnLeftOrRight(cGameObject* player, float currentAngle, float deltaTime)
{
	//Find my forward vector if I were to turn left
	float leftRotation = rotation.y + speed * deltaTime;
	glm::vec3 leftForward = glm::vec3(sin(leftRotation), 0.0f, cos(leftRotation));
	leftForward = glm::normalize(leftForward);

	//Find the angle based on the theoretical left turn
	glm::vec3 vecPlayerDistance = player->position - this->position;
	vecPlayerDistance = glm::normalize(vecPlayerDistance);

	float dotProduct = leftForward.x * vecPlayerDistance.x +
		leftForward.y * vecPlayerDistance.y +
		leftForward.z * vecPlayerDistance.z;

	float cosTheta = dotProduct / (glm::length(leftForward) * glm::length(vecPlayerDistance));

	//Return true if a left turn makes the angle tighter, or false if we should make a right turn
	return (cosTheta > currentAngle);
}