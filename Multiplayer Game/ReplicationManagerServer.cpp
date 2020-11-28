#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	actions.emplace(networkId, ReplicationAction::Create);
}

void ReplicationManagerServer::update(uint32 networkId)
{
	actions[networkId] = ReplicationAction::Update;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	actions[networkId] = ReplicationAction::Destroy;
}

void ReplicationManagerServer::write(OutputMemoryStream& packet)
{
	packet << PROTOCOL_ID;
	packet << ClientMessage::Input;
	packet << actions.size();

	for (auto item = actions.begin(); item != actions.end();)
	{
		packet << (*item).first;
		packet << (*item).second;

		if ((*item).second == ReplicationAction::Destroy)
		{
			item = actions.erase(item);
			continue;
		}
		else if ((*item).second != ReplicationAction::None)
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject((*item).first);


			packet << gameObject->position.x;
			packet << gameObject->position.y;
			packet << gameObject->size.x;
			packet << gameObject->size.y;

			packet << gameObject->angle;
			packet << gameObject->tag;
			packet << gameObject->networkInterpolationEnabled;
			packet << gameObject->state;



			if ((*item).second == ReplicationAction::Create)
			{
				//Behaviour
				if (gameObject->behaviour != nullptr)
				{
					int type = (int)gameObject->behaviour->type();

					switch (type)
					{
					case 0: {
						//There's no behaviour xD
						break; }
					case 1: { //Spaceship
						packet << type;
						gameObject->behaviour->write(packet);
						break; }
					case 2: { //Laser
						packet << type;
						break; }
					}

				}
				else
					packet << 0;


				//Textures
				if (gameObject->sprite != nullptr && gameObject->sprite->texture != nullptr)
				{
					packet << gameObject->sprite->texture->id;
					packet.Write(gameObject->sprite->color);
					packet << gameObject->sprite->order;
					packet.Write(gameObject->sprite->pivot);
				}
				else
					packet << -1;
			}


			(*item).second = ReplicationAction::None;
		}
		++item;
	}
}
