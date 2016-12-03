#pragma once


#define CALL_ACCEPT_STR "accept"
#define CALL_CANCEL_STR "Call was not accepted"
#define CLIENT_ON_CALL_STR "Client is already on call"


#include <winsock2.h>
#include <vector>
#include "Client.h"
#include "ClientNotFoundException.h"


struct ClientSearch
{
	Client *client;
	int searchedIndex;
};


struct ClientConnectInfo
{
	SOCKET socket;
	sockaddr_in sock_addr;
};


enum class SEARCH_FROM {ONLINE_CLIENTS, ALL_CLIENTS};


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
	void Registrate(ClientConnectInfo clConnectInfo);
	void Login(ClientConnectInfo clConnectInfo);
	void Connect(SOCKET clientSock);
	void ClientLeaveChat(SOCKET clientSock);
	void GiveOnlineClientsList(SOCKET clientSock);
	void Disconnect(SOCKET clientSock);
	bool FreeLogin(char *login);
	bool ClientRegistered(char *login, Client* &client);
	void NotifyClientsAboutEvent(char eventNumber, char *eventSrcClientLogin);
	void NotifyClientAboutConnectionEnd(Client *client);
	ClientSearch GetClientByLogin(char *login, SEARCH_FROM searchFrom);
public:
	static Server* GetInstance();
	static const int str_buff_size = 50;
	void Listen();
};