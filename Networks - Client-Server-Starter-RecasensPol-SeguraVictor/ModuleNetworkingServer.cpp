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

	switch (clientMessage)
	{
	case ClientMessage::Hello:
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
				shutdown(s, 2);
				closesocket(s);

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
		break;
	}
	case ClientMessage::Message:
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

		break;
	}
	case ClientMessage::Command:
	{
		std::string playerName;
		std::string command;
		packet >> playerName;
		packet >> command;

		OutputMemoryStream packetMessage;
		std::string message;
		packetMessage << ServerMessage::Command;
		

		if (command.find("/help") == 0)
		{
			message = "---------------------------------------------\n" +
				(std::string)"Here is the command list:\n" +
				"/help -> To get help!\n" +
				"/userlist -> View all users in the server\n" +
				"/kick -> Expel a user. Please use gently :c\n"+
				"/changename -> Change your nickname\n"+
				"---------------------------------------------";
		}
		else if (command.find("/userlist") == 0)
		{
			message = "---------------------------------------------\nUsers in the server:\n";

			for (auto iter = connectedSockets.begin(); iter != connectedSockets.end(); ++iter)
			{
				if ((*iter).socket == s)
				{
					message += "*" + (*iter).playerName + "* -> This is you :D\n";
				}
				else
				{
					message += (*iter).playerName + "\n";
				}
			}

			message += "---------------------------------------------";
		}
		else if (command.find("/kick") == 0)
		{
			std::string user = GetWordInString(command, 1);

			for (auto user_to_kick = connectedSockets.begin(); user_to_kick != connectedSockets.end(); ++user_to_kick)
			{
				if ((*user_to_kick).playerName.compare(user) == 0)
				{
					OutputMemoryStream kick_pack;
					kick_pack << ServerMessage::Kick;
					std::string msg = "";
					kick_pack << msg;

					sendPacket(kick_pack, (*user_to_kick).socket);
					break;
				}
			}
		}
		else if (command.find("/changename") == 0)
		{
			std::string old_name = playerName;
			std::string new_name = GetWordInString(command, 1);
			
			OutputMemoryStream pack;
			std::string msg;
			msg = "'" + old_name + "'" + " has changed their name to " + "'" + new_name + "'";
			pack << ServerMessage::ChangeName;
			pack << msg;

			OutputMemoryStream newname_packet;
			newname_packet << ServerMessage::NewName;
			newname_packet << msg;
			newname_packet << new_name;

			for (auto& connectedsocket : connectedSockets)
			{
				if (connectedsocket.socket == s)
				{
					connectedsocket.playerName = new_name;
					sendPacket(newname_packet, s);
					
					break;
				}
				else
				{
					sendPacket(pack, connectedsocket.socket);
				}
			}
		}
		else
		{
			message = "---------------------------------------------\nERROR: No such command exists\n---------------------------------------------";
		}

		packetMessage << message;

		sendPacket(packetMessage, s);

		break;
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

std::string ModuleNetworkingServer::GetWordInString(const std::string& string, int word_position, bool want_to_take_all_remaining_words)
{
	std::string argument;

	int current_word_position = 0;
	bool add_letter = false;

	if (word_position == 0)
	{
		for (auto character = string.begin(); character != string.end(); ++character)
		{
			if (*character == ' ')
			{
				return argument;
			}
			else
			argument += *character;
		}
	}

	for (auto character = string.begin(); character != string.end(); ++character)
	{
		if ((*character) == ' ')
		{
			if ((*character - 1) == ' ')
			{
				continue;
			}
			if (add_letter)
			{
				break;
			}

			++current_word_position;

			if (current_word_position == word_position)
			{
				add_letter = true;
			}
			else if (current_word_position > word_position)
			{
				return "There aren't that many words";
			}
		}
		else if (add_letter)
		{
			if (want_to_take_all_remaining_words)
			{
				for (auto curr_char = character; curr_char != string.end(); ++curr_char)
				{
					argument += *curr_char;
				}
				break;
			}
			else
			{
				argument += (*character);
			}
		}
	}

	return argument;
}

