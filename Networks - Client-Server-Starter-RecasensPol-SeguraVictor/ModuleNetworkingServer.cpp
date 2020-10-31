#include "ModuleNetworkingServer.h"


//POL RECASENS & VÍCTOR SEGURA

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{

	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		// Log and handle error
		return false;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, 0);;

	struct sockaddr_in bindAddr, client;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(port); // Port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY;  // Not so remote… :-P

	int addr_len = sizeof(client);

	int enable = 1;

	int res = bind(listenSocket, (const sockaddr*)&bindAddr, sizeof(bindAddr));

	int listen_check = listen(listenSocket, 2);
	if (listen_check) {
		printf("not listenned");
	}


	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);

}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET s, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	if (clientMessage == ClientMessage::Hello)
	{
		std::string playerName;
		packet >> playerName;


		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				OutputMemoryStream packet;
				std::string msg = "This username already exists";

				packet << ServerMessage::NoWelcome;
				packet << msg;

				sendPacket(packet, s);

				onSocketDisconnected(s);

				break;
			}


			if (connectedSocket.socket == s)
			{
				connectedSocket.playerName = playerName;

				OutputMemoryStream packet;
				std::string msg = std::string("**************************************************\n") 
					+ std::string("               ")
					+ std::string("WELCOME TO THE CHAT\n")
					+ std::string("Please type /help to see the available commands\n")
					+ std::string("**************************************************\n");
				packet << ServerMessage::Welcome;
				packet << msg;
				
				if (sendPacket(packet, connectedSocket.socket)) 
				{
					OutputMemoryStream packetJoin;
					std::string msgjoin = "***** " + playerName + " joined" + " *****";

					packetJoin << ServerMessage::Join;
					packetJoin << msgjoin;

					for (auto& connectedSocket2 : connectedSockets)
					{
						sendPacket(packetJoin, connectedSocket2.socket);
					}
				}

				

			}
		}
	}

	else if (clientMessage == ClientMessage::Message)
	{
		std::string playerName;
		std::string msg;
		packet >> playerName;
		packet >> msg;

		OutputMemoryStream packetMessage;
		std::string message = playerName + " : " + msg;
		packetMessage << ServerMessage::Chat;
		packetMessage << message;

		for (auto& connectedSocket : connectedSockets)
		{
			sendPacket(packetMessage, connectedSocket.socket);
		}
	}
	
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	std::string playerNameLeft;


	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			playerNameLeft = connectedSocket.playerName;
			connectedSockets.erase(it);
			break;
		}
	}

	//---------------------------------------

	OutputMemoryStream packet;
	std::string msg = "***** " + playerNameLeft + " left" + " *****";

	packet << ServerMessage::Left;
	packet << msg;
	for (auto& connectedSocket : connectedSockets)
	{
		sendPacket(packet, connectedSocket.socket);
	}
}

