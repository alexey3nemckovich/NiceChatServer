#pragma once


#define CALL_ACCEPT_STR "accept"
#define CALL_CANCEL_STR "cancel"


#include <winsock2.h>
#include <vector>
#include "Client.h"
#include "ClientNotFoundException.h"


class Server
{
private:
	//Fields
	static const int TCP_PORT = 666;
	static const int UDP_PORT = 777;
	static const int BUFF_LEN = 1000;
	SOCKET udp_sock;
	SOCKET tcp_sock;
	std::vector<Client*> clients;
	std::vector<Client*> onlineClients;
	//Methods
	Server();
	~Server();
	void Init();
	friend DWORD WINAPI ClientProc(LPVOID client_socket);
	void Registrate(SOCKET clientSock);
	void Login(SOCKET clientSock);
	void Connect(SOCKET clientSock);
	void GiveOnlineClientsList(SOCKET clientSock);
	void ClientLeaveChat(SOCKET clientSock);
	bool FreeLogin(char *login);
	bool ClientRegistered(char *login, Client* &client);
	void NotifyClientsAboutEvent(char eventNumber, char *eventSrcClientLogin);
public:
	static Server* GetInstance();
	static const int str_buff_size = 50;
	void Listen();
};