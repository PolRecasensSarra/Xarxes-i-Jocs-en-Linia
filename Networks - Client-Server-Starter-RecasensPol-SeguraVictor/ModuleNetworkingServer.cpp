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
				std::string msg = "";
				packet << ServerMessage::NoWelcome;
				packet << msg;

				sendPacket(packet, s);
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

		if (command.find("/help") == 0)
		{
			OutputMemoryStream packetMessage;
			std::string message;
			packetMessage << ServerMessage::Command;
			message = "---------------------------------------------\n" +
				(std::string)"Here is the command list:\n\n" +
				"/help -> To get help!\n\n" +
				"/userlist -> View all users in the server\n\n" +
				"/kick -> Expel a user. Please use gently :c\n" +
				"Example: /kick playername\n\n" +
				"/changename -> Change your nickname\n" +
				"Example: /changename new_name\n\n" +
				"/whisper -> Send a private text to someone ;)\n" +
				"Example: /whisper playername\n\n" +
				"/rps -> Rock Paper Scissors Game!\n" +
				"Example: /rps playername scissors\n\n" +
				"/clear -> Clear the chat\n\n" +
				"---------------------------------------------";

			packetMessage << message;

			sendPacket(packetMessage, s);
		}
		else if (command.find("/userlist") == 0)
		{
			OutputMemoryStream packetMessage;
			std::string message;
			packetMessage << ServerMessage::Command;
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

			packetMessage << message;

			sendPacket(packetMessage, s);
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
				else if (user_to_kick == (connectedSockets.end() - 1))
				{
					// No player found with that name
					OutputMemoryStream kick_error;
					kick_error << ServerMessage::Error;
					std::string msg = "*ERROR: No player found with that name.";
					kick_error << msg;

					sendPacket(kick_error, s);
				}
			}
		}
		else if (command.find("/changename") == 0)
		{
			std::string old_name = playerName;
			std::string new_name = GetWordInString(command, 1);

			if (IsNameAvailable(new_name))
			{
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
				OutputMemoryStream pack;
				std::string msg;
				msg = "The name is already taken :(";
				pack << ServerMessage::Error;
				pack << msg;

				for (auto& connectedsocket : connectedSockets)
				{
					if (connectedsocket.socket == s)
					{
						sendPacket(pack, s);
						break;
					}
				}
			}
		}
		else if (command.find("/whisper") == 0)
		{
			std::string msg = playerName + ": " + GetWordInString(command, 2, true);
			std::string player_to_send = GetWordInString(command, 1);

			for (auto connectedsocket = connectedSockets.begin(); connectedsocket != connectedSockets.end(); ++connectedsocket)
			{
				if ((*connectedsocket).playerName == player_to_send)
				{
					OutputMemoryStream pack_receiver;
					pack_receiver << ServerMessage::Whisper;
					pack_receiver << msg;

					OutputMemoryStream pack_sender;
					msg= playerName + "(to " + player_to_send + ")" + ": " + GetWordInString(command, 2, true);
					pack_sender << ServerMessage::Whisper;
					pack_sender << msg;

					sendPacket(pack_receiver, (*connectedsocket).socket);
					sendPacket(pack_sender, s);
					break;
				}
				else if (connectedsocket == (connectedSockets.end() - 1))
				{
					// No player found with that name
					OutputMemoryStream whisper_error;
					whisper_error << ServerMessage::Error;
					std::string msg = "*ERROR: No player found with that name.";
					whisper_error << msg;

					sendPacket(whisper_error, s);
					break;
				}
			}
		}
		else if (command.find("/rps") == 0)
		{
			std::string player_to_send = GetWordInString(command, 1);
			std::string election = GetWordInString(command, 2);
			RockPaperScissorsElection election_value = RockPaperScissorsElection::NONE;
			SOCKET socket_to_send;
			
			// Save what move the player made
			if (election.compare("rock") == 0)
			{
				election_value = RockPaperScissorsElection::Rock;
			}
			else if (election.compare("paper") == 0)
			{
				election_value = RockPaperScissorsElection::Paper;
			}
			else if (election.compare("scissors") == 0)
			{
				election_value = RockPaperScissorsElection::Scissors;
			}
			else
			{
				OutputMemoryStream error_packet;
				error_packet << ServerMessage::Error;
				std::string msg = "There exists no move like that in Rock Paper Scissors!";
				error_packet << msg;

				sendPacket(error_packet, s);
				return;
			}

			bool player_found = false;

			// Checks if the player challenged already challenged the sender player
			for (auto connectedsocket = connectedSockets.begin(); connectedsocket != connectedSockets.end(); ++connectedsocket)
			{
				if ((*connectedsocket).playerName == player_to_send)
				{
					socket_to_send = (*connectedsocket).socket;
					player_found = true;

					if ((*connectedsocket).players_sent.empty())
					{
						for (auto& iter : connectedSockets)
						{
							if (iter.socket == s)
							{
								iter.players_sent.push_back({ player_to_send, election_value });
								
							}
						}
						break;
					}

					for (auto i = (*connectedsocket).players_sent.begin(); i != (*connectedsocket).players_sent.end(); ++i)
					{
						if ((*i).first == playerName)
						{
							std::string first_player_election;
							std::string who_wins;

							switch ((*i).second)
							{
							case RockPaperScissorsElection::Rock:
								first_player_election = "rock";

								switch (election_value)
								{
								case RockPaperScissorsElection::Rock:
									who_wins = "The result was a draw";
									break;
								case RockPaperScissorsElection::Paper:
									who_wins = playerName + " wins!";
									break;
								case RockPaperScissorsElection::Scissors:
									who_wins = player_to_send + " wins!";
									break;
								}

								break;
							case RockPaperScissorsElection::Paper:
								first_player_election = "paper";

								switch (election_value)
								{
								case RockPaperScissorsElection::Rock:
									who_wins = player_to_send + " wins!";
									break;
								case RockPaperScissorsElection::Paper:
									who_wins = "The result was a draw";
									break;
								case RockPaperScissorsElection::Scissors:
									who_wins = playerName + " wins!";
									break;
								}

								break;
							case RockPaperScissorsElection::Scissors:
								first_player_election = "scissors";

								switch (election_value)
								{
								case RockPaperScissorsElection::Rock:
									who_wins = playerName + " wins!";
									break;
								case RockPaperScissorsElection::Paper:
									who_wins = player_to_send + " wins!";
									break;
								case RockPaperScissorsElection::Scissors:
									who_wins = "The result was a draw";
									break;
								}

								break;
							}

							// Answer Code
							OutputMemoryStream return_packet;
							return_packet << ServerMessage::RockPaperScissors;

							std::string msg = "---------------------------------------------\n";
							msg += "The challenge was answered!\n" + (*connectedsocket).playerName + " chose: " + first_player_election + ".\n" + playerName + " chose: " + election + ".\n*";
							msg += who_wins;
							msg += "\n---------------------------------------------\n";

							return_packet << msg;

							sendPacket(return_packet, s);
							sendPacket(return_packet, socket_to_send);

							(*connectedsocket).players_sent.erase(i);

							return;
						}
						else if (i == ((*connectedsocket).players_sent.end() - 1))
						{
							for (auto& iter : connectedSockets)
							{
								if (iter.socket == s)
								{
									iter.players_sent.push_back({ player_to_send, election_value });
								}
							}
						}
					}
				}
				else if (connectedsocket == (connectedSockets.end() - 1) && !player_found)
				{
					// No player found with that name
					OutputMemoryStream whisper_error;
					whisper_error << ServerMessage::Error;
					std::string msg = "*ERROR: No player found with that name.";
					whisper_error << msg;

					sendPacket(whisper_error, s);
					return;
				}
			}


			OutputMemoryStream game_sender;
			game_sender << ServerMessage::RockPaperScissors;
			std::string _msg = "---------------------------------------------\n";
			_msg +=	"You challenged " + player_to_send + " to a Rock Paper Scissors Game!\nYou chose: " + election + "." + "\n---------------------------------------------";
			game_sender << _msg;

			sendPacket(game_sender, s);

			OutputMemoryStream game_sent;
			game_sent << ServerMessage::RockPaperScissors;
			std::string msg = "---------------------------------------------\n" + playerName + " has challenged you to a Rock Paper Scissors Game!\nAnswer with /rps <playername> <election>\nExample: /rps playername scissors\n---------------------------------------------";
			game_sent << msg;

			sendPacket(game_sent, socket_to_send);
		}
		else if (command.find("/clear") == 0)
		{
			OutputMemoryStream packet;
			std::string msg = "";
			packet << ServerMessage::Clear;
			packet << msg;

			sendPacket(packet, s);
		}
		else
		{
			OutputMemoryStream packetMessage;
			std::string message;
			packetMessage << ServerMessage::Command;
			message = "---------------------------------------------\nERROR: No such command exists.\n---------------------------------------------";
			packetMessage << message;

			sendPacket(packetMessage, s);
		}

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

bool ModuleNetworkingServer::IsNameAvailable(std::string name)
{
	for (auto& connectedsocket : connectedSockets)
	{
		if (name == connectedsocket.playerName)
		{
			return false;
		}
	}

	return true;
}
