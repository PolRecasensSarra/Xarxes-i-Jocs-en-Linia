#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	size_t size;
	packet >> size;

	for (int i = 0; i < size; ++i)
	{
		uint32 id;
		packet >> id;

		ReplicationAction action;
		packet >> action;

		if (action == ReplicationAction::Destroy)
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(id);
			App->modLinkingContext->unregisterNetworkGameObject(gameObject);
			App->modGameObject->Destroy(gameObject);
			
		}
		else if (action == ReplicationAction::PlayAudio)
		{
			switch (AudioType(id))
			{
			case AudioType::None: {
				break; }
			case AudioType::Laser: {
				App->modSound->playAudioClip(App->modResources->audioClipLaser);
				break; }
			case AudioType::Explosion: {
				App->modSound->playAudioClip(App->modResources->audioClipExplosion);
				break; }
			case AudioType::PowerUp: {
				App->modSound->playAudioClip(App->modResources->audioPowerUp);
				break; }
			case AudioType::Shield: {
				App->modSound->playAudioClip(App->modResources->audioShield);
				break; }
			case AudioType::Damage: {
				App->modSound->playAudioClip(App->modResources->audioDamage);
				break; }
			case AudioType::ShieldBreak: {
				App->modSound->playAudioClip(App->modResources->audioShieldBreak);
				break; }
			}
		}
		else if (action != ReplicationAction::None && action != ReplicationAction::PlayAudio)
		{
			GameObject* gameObject = nullptr;

			if (action == ReplicationAction::Update)
			{
				gameObject = App->modLinkingContext->getNetworkGameObject(id);
				if (gameObject == nullptr)
				{
					gameObject = App->modGameObject->Instantiate();
					App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, id);
				}
			}

			else
			{
				gameObject = App->modGameObject->Instantiate();
				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, id);

			}

			if (gameObject)
			{
				packet >> gameObject->position.x;
				packet >> gameObject->position.y;
				packet >> gameObject->size.x;
				packet >> gameObject->size.y;
				packet >> gameObject->angle;
				packet >> gameObject->tag;
				packet >> gameObject->networkInterpolationEnabled;
				

				if (action == ReplicationAction::Update && gameObject->behaviour != nullptr)
				{
					if (gameObject->behaviour->type() == BehaviourType::Spaceship)
					{
						gameObject->behaviour->read(packet);
					}
					if (gameObject->behaviour->type() == BehaviourType::Laser)
					{
						gameObject->behaviour->read(packet);
					}
				}

				
				if (action == ReplicationAction::Create)
				{

					//Behaviour
					int type = 0;
					packet >> type;

					switch (type)
					{
					case 0: {
						//There's no behaviour xD
						break; }

					case 1: { //SpaceShip
						gameObject->behaviour = App->modBehaviour->addBehaviour(BehaviourType::Spaceship, gameObject);

						if (gameObject->behaviour != nullptr)
							gameObject->behaviour->read(packet);

						break; }

					case 2: { //Laser
						gameObject->behaviour = App->modBehaviour->addBehaviour(BehaviourType::Laser, gameObject);
						if (gameObject->behaviour != nullptr)
							gameObject->behaviour->read(packet);
						break; }

					case 3: { //Asteroid
						gameObject->behaviour = App->modBehaviour->addBehaviour(BehaviourType::Asteroid, gameObject);
						break; }
					case 4: { //Battery
						gameObject->behaviour = App->modBehaviour->addBehaviour(BehaviourType::Battery, gameObject);
						break; }
					case 5: { //Shield
						gameObject->behaviour = App->modBehaviour->addBehaviour(BehaviourType::Shield, gameObject);
						break; }
					case 6: { //Double Bullet
						gameObject->behaviour = App->modBehaviour->addBehaviour(BehaviourType::DoubleBullet, gameObject);
						break; }
					}

					

					//Textures

					int tex_id;
					packet >> tex_id;

					if (tex_id != -1)
					{
						if (gameObject->sprite == nullptr)
							gameObject->sprite = App->modRender->addSprite(gameObject);

						if (gameObject->sprite != nullptr)
						{
							gameObject->sprite->texture = App->modTextures->getTextureByID(tex_id);

							packet.Read(gameObject->sprite->color);
							packet >> gameObject->sprite->order;
							packet.Read(gameObject->sprite->pivot);
							
						}
					}

					//Animations
					AnimationType anim;
					packet >> anim;

					switch (anim)
					{
					case AnimationType::None: {
						break; }
					case AnimationType::Explosion: {

						gameObject->animation = App->modRender->addAnimation(gameObject);
						if (gameObject->animation != nullptr)
							gameObject->animation->clip = App->modResources->explosionClip;

						break; }

					}
				}
			}
		}
	}
}
