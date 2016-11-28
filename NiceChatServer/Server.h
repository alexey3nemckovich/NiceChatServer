#pragma once
#include <winsock2.h>
#include <vector>
#include "Client.h"


class Server
{
private:
	//Fields
	SOCKET udp_sock;
	SOCKET tcp_sock;
	std::vector<Client> clients;
	static const int PORT = 666;
	static const int BUFF_LEN = 1000000;
	//Methods
	Server();
	~Server();
	void Init();
	friend DWORD WINAPI ClientProc(LPVOID client_socket);
	void Registrate(SOCKET client);
	void Login(SOCKET client);
	void GetOtherClientAddr(SOCKET client);
	void ClientLeaveChat(SOCKET client);
	bool FreeLogin(char *login);
public:
	static Server* GetInstance();
	static const int str_buff_size = 50;
	void Listen();
};