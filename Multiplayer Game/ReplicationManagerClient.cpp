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

			// TODO: (POL) DELETE THIS SHIT
		}
		else if (action != ReplicationAction::None)
		{
			GameObject* gameObject = nullptr;

			if (action == ReplicationAction::Update)
			{
				gameObject = App->modLinkingContext->getNetworkGameObject(id);
				
			}

			else
			{
				gameObject = App->modGameObject->Instantiate();
				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, id);

			}

			packet >> gameObject->position.x;
			packet >> gameObject->position.y;
			packet >> gameObject->size.x;
			packet >> gameObject->size.y;
			packet >> gameObject->angle;
			packet >> gameObject->tag;
			packet >> gameObject->networkInterpolationEnabled;
			packet >> gameObject->state;

			LOG("POSITION X: %f", gameObject->position.x);
			LOG("POSITION Y: %f", gameObject->position.y);

		}
	}
}
