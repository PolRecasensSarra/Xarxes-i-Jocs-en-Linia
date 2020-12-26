#pragma once

#include "Behaviours.h"

class ModuleBehaviour : public Module
{
public:

	bool update() override;

	Behaviour * addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject);
	Spaceship * addSpaceship(GameObject *parentGameObject);
	Laser     * addLaser(GameObject *parentGameObject);
	Asteroid     * addAsteroid(GameObject *parentGameObject);
	Battery     * addBattery(GameObject *parentGameObject);
	Shield     * addShield(GameObject *parentGameObject);
	DoubleBullet     * addDoubleBullet(GameObject *parentGameObject);

	Spaceship spaceships[MAX_CLIENTS];

private:

	void handleBehaviourLifeCycle(Behaviour * behaviour);

	Laser lasers[MAX_GAME_OBJECTS];
	Asteroid asteroids[MAX_GAME_OBJECTS];
	Battery battery[MAX_GAME_OBJECTS];
	Shield shield[MAX_GAME_OBJECTS];
	DoubleBullet doubleBullets[MAX_GAME_OBJECTS];
};

