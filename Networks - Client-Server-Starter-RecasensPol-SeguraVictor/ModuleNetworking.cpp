#include "Networks.h"
#include "ModuleNetworking.h"


//POL RECASENS & VÍCTOR SEGURA

static uint8 NumModulesUsingWinsock = 0;


bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	if (send(socket, packet.GetBufferPtr(), packet.GetSize(), 0) == SOCKET_ERROR) {
		reportError("Error sending a packet");
		return false;

	}
	return true;
}


void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	InputMemoryStream packet;



	fd_set readfds;
	FD_ZERO(&readfds);

	for (auto s : sockets) {
		FD_SET(s, &readfds);
	}

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// TODO(jesus): select those sockets that have a read operation available
	int res = select(0, &readfds, nullptr, nullptr, &timeout);
	if (res == SOCKET_ERROR) {
		LOG("ERROR ON SELECT FUNCTION");
	}



	std::vector<SOCKET> disconnectedSockets;
	// Read selected sockets
	for (auto s : sockets)
	{
		if (FD_ISSET(s, &readfds)) {
			if (isListenSocket(s)) { // Is the server socket
			// Accept stuff
				struct sockaddr_in client;
				int addr_len = sizeof(client);
				SOCKET socket_aux = accept(s, (sockaddr*)&client, &addr_len);

				if (socket_aux != SOCKET_ERROR) {

					onSocketConnected(socket_aux, client);
					addSocket(socket_aux);
				}
				else {
					LOG("ERROR ON SOCKET ACCEPT");
				}
			}
			else { // Is a client socket
				int result_receive = recv(s, packet.GetBufferPtr(), packet.GetCapacity(), 0);
				
			
				if (result_receive > 0) {
					packet.SetSize((uint32)result_receive);
					onSocketReceivedData(s, packet);
				}
			
				else if (result_receive == 0 || result_receive == ECONNRESET) {  //CHECK IF THE SOCKET IS DISCONNECTED
					onSocketDisconnected(s);
					disconnectedSockets.push_back(s);
				}
				else {
					printf("ERROR ON RECIEVING RESULT SOCKET");
				}

				

			}
		}
	}


	//ERASE THE DISCONNECTED SOCKETS FROM THE LIST
	for (auto item = disconnectedSockets.begin(); item != disconnectedSockets.end(); ++item) {
		auto item_s = sockets.begin();
		for (item_s; item_s != sockets.end(); ++item_s) {
			if ((*item_s) == (*item)) {
				sockets.erase(item_s);
				break;
			}
		}
	}
	

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}
