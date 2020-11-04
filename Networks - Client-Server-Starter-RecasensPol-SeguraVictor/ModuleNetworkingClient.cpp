#include "ModuleNetworkingClient.h"


//POL RECASENS & VÍCTOR SEGURA


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;


	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		// Log and handle error
		return false;
	}


	socket_client = socket(AF_INET, SOCK_STREAM, 0);
	
	
	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_port = htons(serverPort); // Port
	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);

	int check_connection = connect(socket_client, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	if (check_connection == SOCKET_ERROR)
	{
		LOG("SOCKET NOT CONNECTED TO REMOTE ADRESS");
	}

	addSocket(socket_client);
	

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (sendPacket(packet, socket_client))
		{
			state = ClientState::Logging;
		}
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}

	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("Hello %s, welcome to the chat", playerName.c_str());
		ImGui::SameLine();
		if (ImGui::Button("Logout"))
		{
			LogOut();
		}

		if (ImGui::BeginChild("##chat", ImVec2(ImGui::GetWindowWidth() * 0.95f, ImGui::GetWindowHeight() * 0.65f), true))
		{
			for (auto item = messages.begin(); item != messages.end(); ++item)
			{
				ImGui::TextColored((*item).second, (*item).first.c_str());
			}
			ImGui::EndChild();
		}

		static char str0[128] = "";
		if (ImGui::InputText("Line", str0, IM_ARRAYSIZE(str0), ImGuiInputTextFlags_EnterReturnsTrue)) 
		{
			if (str0[0] != '\0')
			{
				OutputMemoryStream packet;

				if (str0[0] != '/')
				{
					packet << ClientMessage::Message;
				}
				else
				{
					packet << ClientMessage::Command;
				}

				packet << playerName;
				packet << std::string(str0);
				sendPacket(packet, socket_client);
				memset(str0, 0, 128);
			}
		}

		// Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		
		

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET s, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;
	std::string msg;
	packet >> msg;
	ImVec4 color;

	switch (serverMessage)
	{
	case ServerMessage::Welcome:
		color = { 1.0f, 1.0f, 0.0f, 1.0f };
		break;
	case ServerMessage::NoWelcome:
		LOG("This username already exists! >:(");
		LogOut();
		return;
		break;
	case ServerMessage::Join:
		color = { 0.0f, 1.0f, 0.0f, 1.0f };
		break;
	case ServerMessage::Left:
		color = { 0.8f, 0.8f, 0.8f, 1.0f };
		break;
	case ServerMessage::Chat:
		color = { 1.0f, 1.0f, 1.0f, 1.0f };
		break;
	case ServerMessage::Command:
		color = { 0.0f, 1.0f, 1.0f, 1.0f };
		break;
	case ServerMessage::ChangeName:
		color = { 0.0f, 1.0f, 1.0f, 1.0f };
		break;
	case ServerMessage::Kick:
		LogOut();
		return;
		break;
	case ServerMessage::NewName:
	{
		std::string new_name;
		packet >> new_name;
		playerName = new_name;
		color = { 0.0f, 1.0f, 1.0f, 1.0f };
		break;
	}
	case ServerMessage::Error:
		color = { 1.0f, 0.0f, 0.0f, 1.0f };
		break;
	case ServerMessage::Whisper:
		color = { 1.0f,0.0f,1.0f,1.0f };
		break;
	case ServerMessage::RockPaperScissors:
		color = { 1.0f,0.0f,1.0f,1.0f };
		break;
	case ServerMessage::Clear:
		ClearChat();
		return;
		break;
	}

	messages.push_back({ msg, color });
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	LOG("SERVER DISCONNECTED");

	while (!messages.empty()) {
		messages.pop_back();
	}
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::LogOut()
{
	onSocketDisconnected(socket_client);
	disconnect();
}

void ModuleNetworkingClient::ClearChat()
{
	while (!messages.empty()) {
		messages.pop_back();
	}
}

