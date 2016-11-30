#include "stdafx.h"
#include "Server.h"
#include <PlusVectorAddition.h>


DWORD WINAPI ClientProc(LPVOID client_socket);


Server::Server()
{
	Init();
}


Server::~Server()
{
	int countRegClients = clients.size();
	for (int i = 0; i < countRegClients; i++)
	{
		delete(clients[i]);
	}
	closesocket(udp_sock);
	closesocket(tcp_sock);
	WSACleanup();
}


Server* Server::GetInstance()
{
	static Server server = Server();
	return &server;
}


void Server::Init()
{
	char *SERVERADDR = "127.0.0.1";
	//Initialise winsock lib
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		printf("Error WSAStartup %d\nPress 'Enter' to exit.", WSAGetLastError());
		getchar();
		ExitProcess(0);
	}
	//Create tcp socket
	if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Error creating socket %d\nPress 'Enter' to exit.", WSAGetLastError());
		WSACleanup();
		ExitProcess(0);
	}
	sockaddr_in tcp_sock_addr;
	tcp_sock_addr.sin_family = AF_INET;
	tcp_sock_addr.sin_port = htons(Server::TCP_PORT);
	tcp_sock_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	if (bind(tcp_sock, (sockaddr*)&tcp_sock_addr, sizeof(tcp_sock_addr)))
	{
		printf("Error bind %d\nPress 'Enter' to exit.", WSAGetLastError());
		closesocket(tcp_sock);
		WSACleanup();
		ExitProcess(0);
	}
	//Create udp socket
	if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Error creating socket %d\nPress 'Enter' to exit.", WSAGetLastError());
		WSACleanup();
		ExitProcess(0);
	}
	sockaddr_in udp_sock_addr;
	udp_sock_addr.sin_family = AF_INET;
	udp_sock_addr.sin_port = htons(Server::UDP_PORT);
	udp_sock_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	if (bind(udp_sock, (sockaddr*)&udp_sock_addr, sizeof(udp_sock_addr)))
	{
		printf("Error bind %d\nPress 'Enter' to exit.", WSAGetLastError());
		closesocket(udp_sock);
		WSACleanup();
		ExitProcess(0);
	}
}


/*
@Server listening operation
@Creates new thread for every new client connection
@to process his requests.
*/
void Server::Listen()
{
	printf("NiceChat Server\n");
	printf("Waiting for connections...\n");
	listen(tcp_sock, 0x100);
	SOCKET client_socket;
	struct sockaddr_in client_addr;
	int client_addr_size = sizeof(client_addr);
	while ((client_socket = accept(tcp_sock, (sockaddr *)
		&client_addr, &client_addr_size)))
	{
		DWORD thID;
		CreateThread(NULL, NULL, ClientProc,
			&client_socket, NULL, &thID);
	}
}


/*
@Client processing procedure
@Is receives data, where first byte from client
@says, what type of operation is requested:
@	0 - registration
@	1 - logination
@	2 - connect to another client
@	3 - leave chat
@   4 - get online clients list
*/
DWORD WINAPI ClientProc(LPVOID client_socket)
{
	char operation_num;
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	recv(my_sock, &operation_num, sizeof(char), 0);
	switch (operation_num)
	{
	case 0:
		Server::GetInstance()->Registrate(my_sock);
		break;
	case 1:
		Server::GetInstance()->Login(my_sock);
		break;
	case 2:
		Server::GetInstance()->Connect(my_sock);
		break;
	case 3:
		Server::GetInstance()->ClientLeaveChat(my_sock);
		break;
	case 4:
		Server::GetInstance()->GiveOnlineClientsList(my_sock);
		break;
	default:
		break;
	}
	if (closesocket(my_sock))
	{
		printf("Error closing socket %d", my_sock);
	}
	return 0;
}


void Server::Registrate(SOCKET clientSock)
{
	//Get client info
	char name[STR_BUFF_SIZE];
	char last_name[STR_BUFF_SIZE];
	char login[STR_BUFF_SIZE];
	char pass[STR_BUFF_SIZE];
	ZeroMemory(name, STR_BUFF_SIZE);
	ZeroMemory(last_name, STR_BUFF_SIZE);
	ZeroMemory(login, STR_BUFF_SIZE);
	ZeroMemory(pass, STR_BUFF_SIZE);
	recv(clientSock, name, STR_BUFF_SIZE, 0);
	recv(clientSock, last_name, STR_BUFF_SIZE, 0);
	recv(clientSock, login, STR_BUFF_SIZE, 0);
	recv(clientSock, pass, STR_BUFF_SIZE, 0);
	char buff[Server::BUFF_LEN];
	ZeroMemory(buff, Server::BUFF_LEN);
	//Get clientSock serv list addr
	sockaddr_in udp_client_serv_list_addr;
	recv(clientSock, buff, Server::BUFF_LEN, 0);
	udp_client_serv_list_addr = ((sockaddr_in*)buff)[0];
	//Get client video list addr
	sockaddr_in udp_client_video_list_addr;
	recv(clientSock, buff, Server::BUFF_LEN, 0);
	udp_client_video_list_addr = ((sockaddr_in*)buff)[0];
	if (FreeLogin(login))
	{
		Client* client = new Client(name, last_name, login, pass, udp_client_serv_list_addr, udp_client_video_list_addr);
		client->SetOnline();
		clients.push_back(client);
		onlineClients.push_back(client);
		//sendto(udp_sock, NULL, 0, 0, (sockaddr*)&udp_client_serv_list_addr, sizeof(udp_client_serv_list_addr));
		printf("Registrated new client:\
		\n\tName - %s;\
		\n\tLast name - %s;\
		\n\tLogin - %s;\
		\n\tPassword - %s.\n",
			name, last_name, login, pass);
		NotifyClientsAboutEvent(2, client->Login());
	}
	else
	{
		char *err_msg = "Login is not free.";
		send(clientSock, err_msg, strlen(err_msg) + 1, 0);
	}
}


void Server::Login(SOCKET clientSock)
{
	//Get login info
	char login[STR_BUFF_SIZE];
	char pass[STR_BUFF_SIZE];
	ZeroMemory(login, STR_BUFF_SIZE);
	ZeroMemory(pass, STR_BUFF_SIZE);
	recv(clientSock, login, STR_BUFF_SIZE, 0);
	recv(clientSock, pass, STR_BUFF_SIZE, 0);
	//get addresses
	Client* client = nullptr;
	if (ClientRegistered(login, client))
	{
		if (!client->IsOnline())
		{
			if (strcmp(client->Pass(), pass) == 0)
			{
				client->SetOnline();
				char *loginOk = "ok";
				char *name = client->Name();
				char *lastName = client->LastName();
				send(clientSock, loginOk, strlen(loginOk) + 1, 0);
				Sleep(50);
				send(clientSock, name, strlen(name) + 1, 0);
				Sleep(50);
				send(clientSock, lastName, strlen(lastName) + 1, 0);
				onlineClients.push_back(client);
				printf("Logined client '%s'.\n", login);
				NotifyClientsAboutEvent(2, login);
			}
			else
			{
				char *err_msg = "Incorrect password.";
				send(clientSock, err_msg, strlen(err_msg) + 1, 0);
			}
		}
		else
		{
			char *err_msg = "Client with such login is already online.";
			send(clientSock, err_msg, strlen(err_msg) + 1, 0);
		}
	}
	else
	{
		char *err_msg = "Client with such login is not registered.";
		send(clientSock, err_msg, strlen(err_msg) + 1, 0);
	}
}


void Server::Connect(SOCKET clientSock)
{
	//Get data
	char callEventNumber = 0;
	char login[STR_BUFF_SIZE];
	char destClientLogin[STR_BUFF_SIZE];
	ZeroMemory(login, STR_BUFF_SIZE);
	ZeroMemory(destClientLogin, STR_BUFF_SIZE);
	recv(clientSock, login, STR_BUFF_SIZE, 0);
	recv(clientSock, destClientLogin, STR_BUFF_SIZE, 0);
	//Get dest client
	Client* destClient;
	ClientRegistered(destClientLogin, destClient);
	if (!destClient->IsOnCall())
	{
		printf("'%s' called to '%s'.\n", login);
		//Get src client
		Client* srcClient;
		ClientRegistered(login, srcClient);
		//Get clients addrs
		sockaddr_in srcVideoListAddr = srcClient->udp_video_list_addr;
		sockaddr_in destServListAddr = destClient->udp_serv_list_addr;
		sockaddr_in destVideoListAddr = destClient->udp_video_list_addr;
		//
		int destServListAddrSize = sizeof(destServListAddr);
		char buff[BUFF_LEN];
		sendto(udp_sock, &callEventNumber, sizeof(callEventNumber), 0, (sockaddr*)&(destServListAddr), destServListAddrSize);
		recvfrom(udp_sock, buff, BUFF_LEN, 0, (sockaddr*)&(destServListAddr), &destServListAddrSize);
		if (strcmp(buff, CALL_ACCEPT_STR) == 0)
		{
			destClient->SetOnCall();
			srcClient->SetOnCall();
			sendto(udp_sock, (char*)&srcVideoListAddr, sizeof(srcVideoListAddr), 0, (sockaddr*)&destServListAddr, destServListAddrSize);
			send(clientSock, CALL_ACCEPT_STR, strlen(CALL_ACCEPT_STR) + 1, 0);
			Sleep(50);
			send(clientSock, (char*)&destVideoListAddr, sizeof(destVideoListAddr), 0);
		}
		else
		{
			send(clientSock, CALL_CANCEL_STR, strlen(CALL_CANCEL_STR) + 1, 0);
		}
	}
}


void Server::ClientLeaveChat(SOCKET clientSock)
{
	char clientLogin[BUFF_LEN];
	recv(clientSock, clientLogin, BUFF_LEN, 0);
	int countOnlineClients = onlineClients.size();
	int clientIndex = 0;
	for (int i = 0; i < countOnlineClients; i++)
	{
		if (strcmp(onlineClients[i]->Login(), clientLogin) == 0)
		{
			clientIndex = i;
			break;
		}
	}
	onlineClients[clientIndex]->SetOffline();
	onlineClients[clientIndex]->SetFree();
	char leavedClientLogin[STR_BUFF_SIZE];
	strcpy(leavedClientLogin, onlineClients[clientIndex]->Login());
	NotifyClientsAboutEvent(1, leavedClientLogin);
	onlineClients.erase(onlineClients.begin() + clientIndex);
	printf("Client '%s' leaved chat.\n", leavedClientLogin);
}


void Server::GiveOnlineClientsList(SOCKET clientSock)
{
	int countOnlineClients = onlineClients.size();
	send(clientSock, (char*)&countOnlineClients, sizeof(countOnlineClients), 0);
	char *clientLogin;
	for (int i = 0; i < countOnlineClients; i++)
	{
		Sleep(50);
		clientLogin = onlineClients[i]->Login();
		send(clientSock, clientLogin, strlen(clientLogin) + 1, 0);
	}
}


void Server::NotifyClientsAboutEvent(char eventNumber, char *eventSrcClientLogin)
{
	int msgLen = strlen(eventSrcClientLogin) + 1;
	int countOnlineClients = onlineClients.size();
	for (int i = 0; i < countOnlineClients; i++)
	{
		if (strcmp(onlineClients[i]->Login(), eventSrcClientLogin) != 0)
		{
			sendto(
				udp_sock,
				&eventNumber,
				sizeof(eventNumber),
				0,
				(sockaddr*)&onlineClients[i]->udp_serv_list_addr,
				sizeof(onlineClients[i]->udp_serv_list_addr)
			);
			Sleep(50);
			sendto(
				udp_sock,
				eventSrcClientLogin,
				msgLen,
				0,
				(sockaddr*)&onlineClients[i]->udp_serv_list_addr,
				sizeof(onlineClients[i]->udp_serv_list_addr)
			);
		}
	}
}


bool Server::FreeLogin(char *login)
{
	for each (Client* client in clients)
	{
		if (strcmp(login, client->Login()) == 0)
		{
			return false;
		}
	}
	return true;
}


bool Server::ClientRegistered(char *login, Client* &client)
{
	int countRegisteredClients = clients.size();
	for (int i = 0; i < countRegisteredClients; i++)
	{
		if (strcmp(login, clients[i]->Login()) == 0)
		{
			client = clients[i];
			return true;
		}
	}
	return false;
}