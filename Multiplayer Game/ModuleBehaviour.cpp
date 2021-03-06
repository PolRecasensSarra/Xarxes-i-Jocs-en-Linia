#include "Networks.h"
#include "ModuleBehaviour.h"

bool ModuleBehaviour::update()
{
	for (Spaceship &behaviour : spaceships)
	{
		handleBehaviourLifeCycle(&behaviour);
	}
	
	for (Laser &behaviour : lasers)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (Asteroid& behaviour : asteroids)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (Battery& behaviour : battery)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (Shield& behaviour : shield)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (DoubleBullet& behaviour : doubleBullets)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (SuperSpeed& behaviour : superSpeeds)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	return true;
}

Behaviour *ModuleBehaviour::addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject)
{
	switch (behaviourType)
	{
	case BehaviourType::Spaceship:
		return addSpaceship(parentGameObject);
	case BehaviourType::Laser:
		return addLaser(parentGameObject);
	case BehaviourType::Asteroid:
		return addAsteroid(parentGameObject);
	case BehaviourType::Battery:
		return addBattery(parentGameObject);
	case BehaviourType::Shield:
		return addShield(parentGameObject);
	case BehaviourType::DoubleBullet:
		return addDoubleBullet(parentGameObject);
	case BehaviourType::SuperSpeed:
		return addSuperSpeed(parentGameObject);
	default:
		return nullptr;
	}
}

Spaceship *ModuleBehaviour::addSpaceship(GameObject *parentGameObject)
{
	for (Spaceship &behaviour : spaceships)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

Laser *ModuleBehaviour::addLaser(GameObject *parentGameObject)
{
	for (Laser &behaviour : lasers)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

Asteroid* ModuleBehaviour::addAsteroid(GameObject* parentGameObject)
{
	for (Asteroid& behaviour : asteroids)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

Battery* ModuleBehaviour::addBattery(GameObject* parentGameObject)
{
	for (Battery& behaviour : battery)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

Shield* ModuleBehaviour::addShield(GameObject* parentGameObject)
{
	for (Shield& behaviour : shield)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

DoubleBullet* ModuleBehaviour::addDoubleBullet(GameObject* parentGameObject)
{
	for (DoubleBullet& behaviour : doubleBullets)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

SuperSpeed* ModuleBehaviour::addSuperSpeed(GameObject* parentGameObject)
{
	for (SuperSpeed& behaviour : superSpeeds)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

void ModuleBehaviour::handleBehaviourLifeCycle(Behaviour *behaviour)
{
	GameObject *gameObject = behaviour->gameObject;

	if (gameObject != nullptr)
	{
		switch (gameObject->state)
		{
		case GameObject::STARTING:
			behaviour->start();
			break;
		case GameObject::UPDATING:
			behaviour->update();
			break;
		case GameObject::DESTROYING:
			behaviour->destroy();
			gameObject->behaviour = nullptr;
			behaviour->gameObject = nullptr;
			break;
		default:;
		}
	}
}
