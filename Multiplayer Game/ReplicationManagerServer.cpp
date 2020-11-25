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
		else if ((*item).second!= ReplicationAction::None)
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
				if (gameObject->sprite && gameObject->sprite->texture)
					packet << gameObject->sprite->texture->id;
				else
					packet << -1;
			}


			(*item).second = ReplicationAction::None;
		}
		++item;
	}
}
