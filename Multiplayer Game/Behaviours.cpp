#include "Networks.h"
#include "Behaviours.h"



void Laser::start()
{
	gameObject->networkInterpolationEnabled = false;

	App->modSound->playAudioClip(App->modResources->audioClipLaser);
}

void Laser::update()
{
	secondsSinceCreation += Time.deltaTime;

	const float pixelsPerSecond = 1000.0f;
	gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

	if (isServer)
	{
		const float neutralTimeSeconds = 0.1f;
		if (secondsSinceCreation > neutralTimeSeconds && gameObject->collider == nullptr) {
			gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		}

		const float lifetimeSeconds = 2.0f;
		if (secondsSinceCreation >= lifetimeSeconds) {
			NetworkDestroy(gameObject);
		}
	}
}


void Asteroid::start()
{
	gameObject->tag = (uint32)(Random.next() * UINT_MAX);

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Asteroid, gameObject);
	gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events
}

void Asteroid::update()
{
	
}

void Asteroid::destroy()
{
	
}

void Asteroid::onCollisionTriggered(Collider& c1, Collider& c2)
{
	
	if (c2.type == ColliderType::Laser)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser
			NetworkDestroy(gameObject);

			if (c2.gameObject->behaviour->GetIfPowerUp())
				c2.gameObject->behaviour->SetIfPowerUp(false);


			GameObject* explosion = NetworkInstantiate();
			explosion->position = gameObject->position;
			explosion->size = vec2{ gameObject->size.x, gameObject->size.y };
			explosion->angle = 365.0f * Random.next();

			explosion->sprite = App->modRender->addSprite(explosion);
			explosion->sprite->texture = App->modResources->explosion2;
			explosion->sprite->order = 100;

			explosion->animation = App->modRender->addAnimation(explosion);
			explosion->animation->clip = App->modResources->explosionClip;
			explosion->animation->type = AnimationType::Explosion;

			NetworkDestroy(explosion, 2.0f);

			App->modSound->playAudioClip(App->modResources->audioClipExplosion);

		}
	}
}

void Battery::start()
{
	gameObject->tag = (uint32)(Random.next() * UINT_MAX);

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Battery, gameObject);
}

void Battery::update()
{
}



void Spaceship::start()
{
	gameObject->tag = (uint32)(Random.next() * UINT_MAX);

	lifebar = Instantiate();
	lifebar->sprite = App->modRender->addSprite(lifebar);
	lifebar->sprite->pivot = vec2{ 0.0f, 0.5f };
	lifebar->sprite->order = 5;
}

void Spaceship::onInput(const InputController &input)
{
	if (input.horizontalAxis != 0.0f)
	{
		const float rotateSpeed = 180.0f;
		gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}

	if (input.actionDown == ButtonState::Pressed)
	{
		const float advanceSpeed = 200.0f;
		gameObject->position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}

	if (input.actionLeft == ButtonState::Press)
	{
		if (isServer)
		{
			GameObject *laser = NetworkInstantiate();

			laser->position = gameObject->position;
			laser->angle = gameObject->angle;
			laser->size = { 20, 60 };

			laser->sprite = App->modRender->addSprite(laser);
			laser->sprite->order = 3;

			Laser* laserBehaviour = App->modBehaviour->addLaser(laser);

			if (powerUp)
			{
				laser->sprite->texture = App->modResources->laser2;
				laserBehaviour->powerUp = true;
				powerUp = false;
			}
			else
			{
				laser->sprite->texture = App->modResources->laser;
				laserBehaviour->powerUp = false;
			}

			
			laserBehaviour->isServer = isServer;
			

			laser->tag = gameObject->tag;
		}
	}

	if (input.leftShoulder == ButtonState::Press)
	{
		//TODO(pol): canviar això d'aquí a un random generator d'asteroides al principi de la partida
		if (isServer)
		{
			GameObject* asteroid = NetworkInstantiate();

			asteroid->position = gameObject->position + vec2{150.0, 100.0};
			asteroid->angle = gameObject->angle;
			asteroid->size = vec2{ 120.0f,120.0f };

			asteroid->sprite = App->modRender->addSprite(asteroid);
			asteroid->sprite->order = 3;
			int test = (rand() % 2);
			if (test == 1)
				asteroid->sprite->texture = App->modResources->asteroid2;
			
			else
				asteroid->sprite->texture = App->modResources->asteroid1;
			

			Asteroid* asteroidBehaviour = App->modBehaviour->addAsteroid(asteroid);
			asteroidBehaviour->isServer = isServer;

			
		}
	}
	if (input.rightShoulder == ButtonState::Press)
	{
		//TODO(pol): canviar això d'aquí a un random generator de bateries al principi de la partida
		if (isServer)
		{
			GameObject* battery = NetworkInstantiate();

			battery->position = gameObject->position + vec2{ 150.0, -100.0 };
			battery->angle = 0.0f;
			battery->size = vec2{ 0.0f,0.0f};

			battery->sprite = App->modRender->addSprite(battery);
			battery->sprite->order = 3;
		
			battery->sprite->texture = App->modResources->battery;



			Battery* batteryBehaviour = App->modBehaviour->addBattery(battery);
			batteryBehaviour->isServer = isServer;


		}
	}
}

void Spaceship::update()
{
	static const vec4 colorAlive = vec4{ 0.2f, 1.0f, 0.1f, 0.5f };
	static const vec4 colorDead = vec4{ 1.0f, 0.2f, 0.1f, 0.5f };
	const float lifeRatio = max(0.01f, (float)(hitPoints) / (MAX_HIT_POINTS));
	lifebar->position = gameObject->position + vec2{ -50.0f, -50.0f };
	lifebar->size = vec2{ lifeRatio * 80.0f, 5.0f };
	lifebar->sprite->color = lerp(colorDead, colorAlive, lifeRatio);
}

void Spaceship::destroy()
{
	Destroy(lifebar);
}

void Spaceship::onCollisionTriggered(Collider &c1, Collider &c2)
{
	if (c2.type == ColliderType::Laser && c2.gameObject->tag != gameObject->tag)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser
		
			if (hitPoints > 0)
			{
				if (!c2.gameObject->behaviour->GetIfPowerUp())
				{
					hitPoints--;
				}
				else
				{
					hitPoints = hitPoints - 3;
					gameObject->behaviour->SetIfPowerUp(false);
				}
				
				NetworkUpdate(gameObject);
			}

			float size = 30 + 50.0f * Random.next();
			vec2 position = gameObject->position + 50.0f * vec2{Random.next() - 0.5f, Random.next() - 0.5f};

			if (hitPoints <= 0)
			{
				// Centered big explosion
				size = 250.0f + 100.0f * Random.next();
				position = gameObject->position;

				NetworkDestroy(gameObject);
			}

			GameObject *explosion = NetworkInstantiate();
			explosion->position = position;
			explosion->size = vec2{ size, size };
			explosion->angle = 365.0f * Random.next();

			explosion->sprite = App->modRender->addSprite(explosion);
			explosion->sprite->texture = App->modResources->explosion1;
			explosion->sprite->order = 100;

			explosion->animation = App->modRender->addAnimation(explosion);
			explosion->animation->clip = App->modResources->explosionClip;
			explosion->animation->type = AnimationType::Explosion;

			NetworkDestroy(explosion, 2.0f);

			// NOTE(jesus): Only played in the server right now...
			// You need to somehow make this happen in clients
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);
		}
	}
	else if (c2.type == ColliderType::Asteroid)
	{
		// Centered big explosion
		float size = 250.0f + 100.0f * Random.next();
		vec2 position = gameObject->position;

		NetworkDestroy(gameObject);

		GameObject* explosion = NetworkInstantiate();
		explosion->position = position;
		explosion->size = vec2{ size, size };
		explosion->angle = 365.0f * Random.next();

		explosion->sprite = App->modRender->addSprite(explosion);
		explosion->sprite->texture = App->modResources->explosion1;
		explosion->sprite->order = 100;

		explosion->animation = App->modRender->addAnimation(explosion);
		explosion->animation->clip = App->modResources->explosionClip;
		explosion->animation->type = AnimationType::Explosion;

		NetworkDestroy(explosion, 2.0f);

		// NOTE(jesus): Only played in the server right now...
		// You need to somehow make this happen in clients
		App->modSound->playAudioClip(App->modResources->audioClipExplosion);
	}
	else if (c2.type == ColliderType::Battery)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the battery
			powerUp = true;
			
			App->modSound->playAudioClip(App->modResources->audioPowerUp);
		}
	}
}

void Spaceship::write(OutputMemoryStream & packet)
{
	packet << hitPoints;
	packet << powerUp;
}

void Spaceship::read(const InputMemoryStream & packet)
{
	packet >> hitPoints;
	packet >> powerUp;
}

bool Laser::GetIfPowerUp()
{
	return powerUp;
}

void Laser::SetIfPowerUp(bool powerup)
{
	powerUp = powerup;
}

void Laser::write(OutputMemoryStream& packet)
{
	packet << powerUp;
}

void Laser::read(const InputMemoryStream& packet)
{
	packet >> powerUp;
}


