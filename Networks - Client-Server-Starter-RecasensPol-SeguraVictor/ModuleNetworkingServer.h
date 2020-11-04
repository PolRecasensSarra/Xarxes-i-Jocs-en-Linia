#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingServer : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingServer public methods
	//////////////////////////////////////////////////////////////////////

	bool start(int port);

	bool isRunning() const;



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	bool isListenSocket(SOCKET socket) const override;

	void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) override;

	void onSocketReceivedData(SOCKET s, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;

	std::string GetWordInString(const std::string& string, int word_position, bool want_to_take_all_remaining_words = false);

	bool IsNameAvailable(std::string name);

	//////////////////////////////////////////////////////////////////////
	// State
	//////////////////////////////////////////////////////////////////////

	enum class ServerState
	{
		Stopped,
		Listening
	};

	ServerState state = ServerState::Stopped;

	SOCKET listenSocket;

	enum RockPaperScissorsElection
	{
		Rock,
		Paper,
		Scissors,
		NONE = -1,
	};

	struct ConnectedSocket
	{
		sockaddr_in address;
		SOCKET socket;
		std::string playerName;
		std::vector<std::pair<std::string, RockPaperScissorsElection>> players_sent;
	};

	std::vector<ConnectedSocket> connectedSockets;
};

