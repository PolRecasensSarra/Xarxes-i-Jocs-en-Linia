#include "ModuleNetworkingServer.h"
#include <cmath>


//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingServer::setListenPort(int port)
{
	listenPort = port;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingServer::onStart()
{
	if (!createSocket()) return;

	// Reuse address
	int enable = 1;
	int res = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingServer::start() - setsockopt");
		disconnect();
		return;
	}

	// Create and bind to local address
	if (!bindSocketToPort(listenPort)) {
		return;
	}

	state = ServerState::Listening;
}

void ModuleNetworkingServer::onGui()
{
	if (ImGui::CollapsingHeader("ModuleNetworkingServer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Connection checking info:");
		ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
		ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

		ImGui::Separator();

		if (state == ServerState::Listening)
		{
			int count = 0;

			for (int i = 0; i < MAX_CLIENTS; ++i)
			{
				if (clientProxies[i].connected)
				{
					ImGui::Text("CLIENT %d", count++);
					ImGui::Text(" - address: %d.%d.%d.%d",
						clientProxies[i].address.sin_addr.S_un.S_un_b.s_b1,
						clientProxies[i].address.sin_addr.S_un.S_un_b.s_b2,
						clientProxies[i].address.sin_addr.S_un.S_un_b.s_b3,
						clientProxies[i].address.sin_addr.S_un.S_un_b.s_b4);
					ImGui::Text(" - port: %d", ntohs(clientProxies[i].address.sin_port));
					ImGui::Text(" - name: %s", clientProxies[i].name.c_str());
					ImGui::Text(" - id: %d", clientProxies[i].clientId);
					if (clientProxies[i].gameObject != nullptr)
					{
						ImGui::Text(" - gameObject net id: %d", clientProxies[i].gameObject->networkId);
					}
					else
					{
						ImGui::Text(" - gameObject net id: (null)");
					}
					
					ImGui::Separator();
				}
			}

			ImGui::Checkbox("Render colliders", &App->modRender->mustRenderColliders);
		}
	}
}

void ModuleNetworkingServer::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	if (state == ServerState::Listening)
	{
		uint32 protoId;
		packet >> protoId;
		if (protoId != PROTOCOL_ID) return;

		ClientMessage message;
		packet >> message;

		ClientProxy *proxy = getClientProxy(fromAddress);

		if (message == ClientMessage::Hello)
		{
			if (proxy == nullptr)
			{
				proxy = createClientProxy();

				if (proxy != nullptr)
				{
					std::string playerName;
					uint8 spaceshipType;
					packet >> playerName;
					packet >> spaceshipType;

					proxy->address.sin_family = fromAddress.sin_family;
					proxy->address.sin_addr.S_un.S_addr = fromAddress.sin_addr.S_un.S_addr;
					proxy->address.sin_port = fromAddress.sin_port;
					proxy->connected = true;
					proxy->name = playerName;
					proxy->clientId = nextClientId++;

					// Create new network object
					vec2 initialPosition = 500.0f * vec2{ Random.next() - 0.5f, Random.next() - 0.5f};
					float initialAngle = 360.0f * Random.next();
					proxy->gameObject = spawnPlayer(spaceshipType, initialPosition, initialAngle);
				}
				else
				{
					// NOTE(jesus): Server is full...
				}
			}

			if (proxy != nullptr)
			{
				// Send welcome to the new player
				OutputMemoryStream welcomePacket;
				welcomePacket << PROTOCOL_ID;
				welcomePacket << ServerMessage::Welcome;
				welcomePacket << proxy->clientId;
				welcomePacket << proxy->gameObject->networkId;
				sendPacket(welcomePacket, fromAddress);

				// Send all network objects to the new player
				uint16 networkGameObjectsCount;
				GameObject *networkGameObjects[MAX_NETWORK_OBJECTS];
				App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
				GameObject* go_aux = nullptr;
				for (uint16 i = 0; i < networkGameObjectsCount; ++i)
				{
					GameObject *gameObject = networkGameObjects[i];
					// TODO(you): World state replication lab session
					proxy->replication_manager_server.create(gameObject->networkId);  // En teoria això hauria d'instanciar els objectes antics al nou proxy

					go_aux = gameObject;
				}

				OutputMemoryStream packet;
				proxy->replication_manager_server.write(packet);
				sendPacket(packet, fromAddress);



		


				LOG("Message received: hello - from player %s", proxy->name.c_str());
			}
			else
			{
				OutputMemoryStream unwelcomePacket;
				unwelcomePacket << PROTOCOL_ID;
				unwelcomePacket << ServerMessage::Unwelcome;
				sendPacket(unwelcomePacket, fromAddress);

				WLOG("Message received: UNWELCOMED hello - server is full");
			}
		}
		else if (message == ClientMessage::Input)
		{
			// Process the input packet and update the corresponding game object
			if (proxy != nullptr && IsValid(proxy->gameObject))
			{
				
				uint32 lastSequenceNumber = 0;
				// Read input data
				while (packet.RemainingByteCount() > 0)
				{
					InputPacketData inputData;
					packet >> inputData.sequenceNumber;
					packet >> inputData.horizontalAxis;
					packet >> inputData.verticalAxis;
					packet >> inputData.buttonBits;

					if (inputData.sequenceNumber >= proxy->nextExpectedInputSequenceNumber)
					{
						proxy->gamepad.horizontalAxis = inputData.horizontalAxis;
						proxy->gamepad.verticalAxis = inputData.verticalAxis;
						unpackInputControllerButtons(inputData.buttonBits, proxy->gamepad);
						proxy->gameObject->behaviour->onInput(proxy->gamepad);
						proxy->nextExpectedInputSequenceNumber = inputData.sequenceNumber + 1;
					}

					lastSequenceNumber = inputData.sequenceNumber;
				}
				OutputMemoryStream packet;
				packet << PROTOCOL_ID;
				packet << ServerMessage::Reliability;
				packet << lastSequenceNumber;
				sendPacket(packet, fromAddress);
				

			}
		}
		else if (message == ClientMessage::Ping)
		{
			proxy->secondsSinceLastPacketReceived = 0.0f;
			LOG("Ping Received");
		}
	}
}

void ModuleNetworkingServer::onUpdate()
{
	if (state == ServerState::Listening)
	{
		asteroidTime.currentRandomTime += Time.deltaTime;
		if (asteroidTime.currentRandomTime >= asteroidTime.randomTime)
		{
			asteroidTime.randomTime = asteroidTime.min_time_between_spawns + (rand() % asteroidTime.max_difference_time);
			asteroidTime.currentRandomTime = 0;
			spawnGameElement(BehaviourType::Asteroid);
		}

		powerUpTime.currentRandomTime += Time.deltaTime;
		if (powerUpTime.currentRandomTime >= powerUpTime.randomTime)
		{
			powerUpTime.randomTime = powerUpTime.min_time_between_spawns + (rand() % powerUpTime.max_difference_time);
			powerUpTime.currentRandomTime = 0;

			int randomPowerUp = rand() % 3;

			switch (randomPowerUp)
			{
			case 0:
				spawnGameElement(BehaviourType::Battery);
				break;
			case 1:
				spawnGameElement(BehaviourType::Shield);
				break;
			case 2:
				spawnGameElement(BehaviourType::DoubleBullet);
				break;
			}
		}

		// Handle networked game object destructions
		for (DelayedDestroyEntry &destroyEntry : netGameObjectsToDestroyWithDelay)
		{
			if (destroyEntry.object != nullptr)
			{
				destroyEntry.delaySeconds -= Time.deltaTime;
				if (destroyEntry.delaySeconds <= 0.0f)
				{
					destroyNetworkObject(destroyEntry.object);
					destroyEntry.object = nullptr;
				}
			}
		}

		for (ClientProxy &clientProxy : clientProxies)
		{
			if (clientProxy.connected)
			{
				clientProxy.secondsSinceLastPacketReceived += Time.deltaTime;
				clientProxy.pingTimer += Time.deltaTime;
				
				

				if(clientProxy.secondsSinceLastPacketReceived > DISCONNECT_TIMEOUT_SECONDS)
				{
					onConnectionReset(clientProxy.address);
				}

				if (clientProxy.pingTimer > PING_INTERVAL_SECONDS)
				{
					OutputMemoryStream packet;
					packet << PROTOCOL_ID;
					packet << ServerMessage::Ping;

					sendPacket(packet, clientProxy.address);

					clientProxy.pingTimer = 0.0f;
				}

				// Don't let the client proxy point to a destroyed game object
				if (!IsValid(clientProxy.gameObject))
				{
					clientProxy.gameObject = nullptr;
				}

				OutputMemoryStream packet_up;
				clientProxy.replication_manager_server.write(packet_up);
				sendPacket(packet_up, clientProxy.address);
				

				// TODO(you): Reliability on top of UDP lab session
			}
		}
	}
}

void ModuleNetworkingServer::onConnectionReset(const sockaddr_in & fromAddress)
{
	// Find the client proxy
	ClientProxy *proxy = getClientProxy(fromAddress);

	if (proxy)
	{
		// Clear the client proxy
		destroyClientProxy(proxy);
	}
}

void ModuleNetworkingServer::onDisconnect()
{
	uint16 netGameObjectsCount;
	GameObject *netGameObjects[MAX_NETWORK_OBJECTS];
	App->modLinkingContext->getNetworkGameObjects(netGameObjects, &netGameObjectsCount);
	for (uint32 i = 0; i < netGameObjectsCount; ++i)
	{
		NetworkDestroy(netGameObjects[i]);
	}

	for (ClientProxy &clientProxy : clientProxies)
	{
		destroyClientProxy(&clientProxy);
	}
	
	nextClientId = 0;

	state = ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Client proxies
//////////////////////////////////////////////////////////////////////

ModuleNetworkingServer::ClientProxy * ModuleNetworkingServer::createClientProxy()
{
	// If it does not exist, pick an empty entry
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!clientProxies[i].connected)
		{
			return &clientProxies[i];
		}
	}

	return nullptr;
}

ModuleNetworkingServer::ClientProxy * ModuleNetworkingServer::getClientProxy(const sockaddr_in &clientAddress)
{
	// Try to find the client
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].address.sin_addr.S_un.S_addr == clientAddress.sin_addr.S_un.S_addr &&
			clientProxies[i].address.sin_port == clientAddress.sin_port)
		{
			return &clientProxies[i];
		}
	}

	return nullptr;
}

void ModuleNetworkingServer::destroyClientProxy(ClientProxy *clientProxy)
{
	// Destroy the object from all clients
	if (IsValid(clientProxy->gameObject))
	{
		destroyNetworkObject(clientProxy->gameObject);
	}

    *clientProxy = {};
}


//////////////////////////////////////////////////////////////////////
// Spawning
//////////////////////////////////////////////////////////////////////

GameObject * ModuleNetworkingServer::spawnPlayer(uint8 spaceshipType, vec2 initialPosition, float initialAngle)
{
	// Create a new game object with the player properties
	GameObject *gameObject = NetworkInstantiate();
	gameObject->position = initialPosition;
	gameObject->size = { 100, 100 };
	gameObject->angle = initialAngle;

	// Create behaviour
	Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);
	gameObject->behaviour = spaceshipBehaviour;
	gameObject->behaviour->isServer = true;
	

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->order = 5;

	switch (spaceshipType)
	{
	case 0:
		gameObject->sprite->texture = App->modResources->spacecraft1;
		spaceshipBehaviour->original_texture = gameObject->sprite->texture;
		spaceshipBehaviour->shielded_texture = App->modResources->spacecraft1Shield;
		break;
	case 1:
		gameObject->sprite->texture = App->modResources->spacecraft2;
		spaceshipBehaviour->original_texture = gameObject->sprite->texture;
		spaceshipBehaviour->shielded_texture = App->modResources->spacecraft2Shield;
		break;
	case 2:
		gameObject->sprite->texture = App->modResources->spacecraft3;
		spaceshipBehaviour->original_texture = gameObject->sprite->texture;
		spaceshipBehaviour->shielded_texture = App->modResources->spacecraft3Shield;
		break;
	case 3:
		gameObject->sprite->texture = App->modResources->spacecraft4;
		spaceshipBehaviour->original_texture = gameObject->sprite->texture;
		spaceshipBehaviour->shielded_texture = App->modResources->spacecraft4Shield;
		break;
	case 4:
		gameObject->sprite->texture = App->modResources->spacecraft5;
		spaceshipBehaviour->original_texture = gameObject->sprite->texture;
		spaceshipBehaviour->shielded_texture = App->modResources->spacecraft5Shield;
		break;
	case 5:
		gameObject->sprite->texture = App->modResources->spacecraft6;
		spaceshipBehaviour->original_texture = gameObject->sprite->texture;
		spaceshipBehaviour->shielded_texture = App->modResources->spacecraft6Shield;
		break;
	}

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
	gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events



	return gameObject;
}

GameObject* ModuleNetworkingServer::spawnGameElement(BehaviourType type)
{
	GameObject* gameElement = NetworkInstantiate();

	gameElement->position.x = (rand() % (max_bounds_spawn * 2)) - max_bounds_spawn;
	gameElement->position.y = (rand() % (max_bounds_spawn * 2)) - max_bounds_spawn;

	if (type == BehaviourType::Asteroid)
	{
		int i = 0;
		while (App->modBehaviour->spaceships[i].gameObject != nullptr)
		{
			while ((abs(App->modBehaviour->spaceships[i].gameObject->position.x - gameElement->position.x) < 100)
				&& (abs(App->modBehaviour->spaceships[i].gameObject->position.y - gameElement->position.y) < 100))
			{
				gameElement->position.x = (rand() % (max_bounds_spawn * 2)) - max_bounds_spawn;
				gameElement->position.y = (rand() % (max_bounds_spawn * 2)) - max_bounds_spawn;
			}
			++i;
		}
	}

	gameElement->angle = 0;
	gameElement->size = vec2{ 0,0 };

	gameElement->sprite = App->modRender->addSprite(gameElement);
	gameElement->sprite->order = 3;
	int randSprite = (rand() % 2);

	switch (type)
	{
	case BehaviourType::Asteroid:
	{
		gameElement->size = vec2{ 120.0f,120.0f };

		if (randSprite == 1)
			gameElement->sprite->texture = App->modResources->asteroid2;
		else
			gameElement->sprite->texture = App->modResources->asteroid1;

		gameElement->behaviour = App->modBehaviour->addAsteroid(gameElement);
		break;
	}
	case BehaviourType::Shield:
	{
		gameElement->sprite->texture = App->modResources->shield;
		gameElement->behaviour = App->modBehaviour->addShield(gameElement);
		break;
	}
	case BehaviourType::Battery:
	{
		gameElement->sprite->texture = App->modResources->battery;
		gameElement->behaviour = App->modBehaviour->addBattery(gameElement);
		break;
	}
	case BehaviourType::DoubleBullet:
	{
		gameElement->sprite->texture = App->modResources->doubleBullet;
		gameElement->behaviour = App->modBehaviour->addDoubleBullet(gameElement);
		break;
	}
	}
	
	gameElement->behaviour->isServer = true;
	return gameElement;
}

//////////////////////////////////////////////////////////////////////
// Update / destruction
//////////////////////////////////////////////////////////////////////

GameObject * ModuleNetworkingServer::instantiateNetworkObject()
{
	// Create an object into the server
	GameObject * gameObject = Instantiate();

	// Register the object into the linking context
	App->modLinkingContext->registerNetworkGameObject(gameObject);

	// Notify all client proxies' replication manager to create the object remotely
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].connected)
		{
			// TODO(you): World state replication lab session
			clientProxies[i].replication_manager_server.create(gameObject->networkId);
		}
	}

	return gameObject;
}

void ModuleNetworkingServer::updateNetworkObject(GameObject * gameObject)
{
	// Notify all client proxies' replication manager to destroy the object remotely
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].connected)
		{
			clientProxies[i].replication_manager_server.update(gameObject->networkId);
			
			// TODO(you): World state replication lab session
		}
	}
}

void ModuleNetworkingServer::destroyNetworkObject(GameObject * gameObject)
{
	// Notify all client proxies' replication manager to destroy the object remotely
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].connected)
		{
			// TODO(you): World state replication lab session
			clientProxies[i].replication_manager_server.destroy(gameObject->networkId);

		}
	}

	// Assuming the message was received, unregister the network identity
	App->modLinkingContext->unregisterNetworkGameObject(gameObject);

	// Finally, destroy the object from the server
	Destroy(gameObject);
}

void ModuleNetworkingServer::destroyNetworkObject(GameObject * gameObject, float delaySeconds)
{
	uint32 emptyIndex = MAX_GAME_OBJECTS;
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (netGameObjectsToDestroyWithDelay[i].object == gameObject)
		{
			float currentDelaySeconds = netGameObjectsToDestroyWithDelay[i].delaySeconds;
			netGameObjectsToDestroyWithDelay[i].delaySeconds = min(currentDelaySeconds, delaySeconds);
			return;
		}
		else if (netGameObjectsToDestroyWithDelay[i].object == nullptr)
		{
			if (emptyIndex == MAX_GAME_OBJECTS)
			{
				emptyIndex = i;
			}
		}
	}

	ASSERT(emptyIndex < MAX_GAME_OBJECTS);

	netGameObjectsToDestroyWithDelay[emptyIndex].object = gameObject;
	netGameObjectsToDestroyWithDelay[emptyIndex].delaySeconds = delaySeconds;
}


//////////////////////////////////////////////////////////////////////
// Global create / update / destruction of network game objects
//////////////////////////////////////////////////////////////////////

GameObject * NetworkInstantiate()
{
	ASSERT(App->modNetServer->isConnected());

	return App->modNetServer->instantiateNetworkObject();
}

void NetworkUpdate(GameObject * gameObject)
{
	ASSERT(App->modNetServer->isConnected());
	ASSERT(gameObject->networkId != 0);

	App->modNetServer->updateNetworkObject(gameObject);
}

void NetworkDestroy(GameObject * gameObject)
{
	NetworkDestroy(gameObject, 0.0f);
}

void NetworkDestroy(GameObject * gameObject, float delaySeconds)
{
	ASSERT(App->modNetServer->isConnected());
	ASSERT(gameObject->networkId != 0);

	App->modNetServer->destroyNetworkObject(gameObject, delaySeconds);
}
