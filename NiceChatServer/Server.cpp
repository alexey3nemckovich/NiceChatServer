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
	char *SERVERADDR = "192.168.100.3";
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
		ClientConnectInfo clConnect;
		clConnect.socket = client_socket;
		clConnect.sock_addr = client_addr;
		CreateThread(NULL, NULL, ClientProc,
			&clConnect, NULL, &thID);
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
@   5 - end connection with client
*/
DWORD WINAPI ClientProc(LPVOID client_socket)
{
	char operation_num;
	ClientConnectInfo connectedClient;
	connectedClient = ((ClientConnectInfo *)client_socket)[0];
	recv(connectedClient.socket, &operation_num, sizeof(char), 0);
	switch (operation_num)
	{
	case 0:
		Server::GetInstance()->Registrate(connectedClient);
		break;
	case 1:
		Server::GetInstance()->Login(connectedClient);
		break;
	case 2:
		Server::GetInstance()->Connect(connectedClient.socket);
		break;
	case 3:
		Server::GetInstance()->ClientLeaveChat(connectedClient.socket);
		break;
	case 4:
		Server::GetInstance()->GiveOnlineClientsList(connectedClient.socket);
		break;
	case 5:
		Server::GetInstance()->Disconnect(connectedClient.socket);
		break;
	default:
		break;
	}
	if (closesocket(connectedClient.socket))
	{
		printf("Error closing socket %d", connectedClient.socket);
	}
	return 0;
}


void Server::Registrate(ClientConnectInfo clConnectInf)
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
	recv(clConnectInf.socket, name, STR_BUFF_SIZE, 0);
	recv(clConnectInf.socket, last_name, STR_BUFF_SIZE, 0);
	recv(clConnectInf.socket, login, STR_BUFF_SIZE, 0);
	recv(clConnectInf.socket, pass, STR_BUFF_SIZE, 0);
	char buff[Server::BUFF_LEN];
	ZeroMemory(buff, Server::BUFF_LEN);
	//Get client udp listen ports
	USHORT udp_client_serv_list_port;
	recv(clConnectInf.socket, (char*)&udp_client_serv_list_port, sizeof(USHORT), 0);
	//
	sockaddr_in udp_client_serv_list_addr, udp_client_video_list_addr;
	if (FreeLogin(login))
	{
		udp_client_serv_list_addr.sin_family = udp_client_video_list_addr.sin_family = clConnectInf.sock_addr.sin_family;
		udp_client_serv_list_addr.sin_addr = udp_client_video_list_addr.sin_addr = clConnectInf.sock_addr.sin_addr;
		udp_client_serv_list_addr.sin_port = udp_client_serv_list_port;
		Client* client = new Client(name, last_name, login, pass, udp_client_serv_list_addr);
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
		send(clConnectInf.socket, err_msg, strlen(err_msg) + 1, 0);
	}
}


void Server::Login(ClientConnectInfo clConnectInf)
{
	//Get login info
	char login[STR_BUFF_SIZE];
	char pass[STR_BUFF_SIZE];
	ZeroMemory(login, STR_BUFF_SIZE);
	ZeroMemory(pass, STR_BUFF_SIZE);
	USHORT udp_client_serv_list_port;
	recv(clConnectInf.socket, login, STR_BUFF_SIZE, 0);
	recv(clConnectInf.socket, pass, STR_BUFF_SIZE, 0);
	recv(clConnectInf.socket, (char*)&udp_client_serv_list_port, sizeof(USHORT), 0);
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
				client->udp_serv_list_addr.sin_addr = clConnectInf.sock_addr.sin_addr;
				client->udp_serv_list_addr.sin_port = udp_client_serv_list_port;
				send(clConnectInf.socket, loginOk, strlen(loginOk) + 1, 0);
				Sleep(50);
				send(clConnectInf.socket, name, strlen(name) + 1, 0);
				Sleep(50);
				send(clConnectInf.socket, lastName, strlen(lastName) + 1, 0);
				onlineClients.push_back(client);
				printf("Logined client '%s'.\n", login);
				NotifyClientsAboutEvent(2, login);
			}
			else
			{
				char *err_msg = "Incorrect password.";
				send(clConnectInf.socket, err_msg, strlen(err_msg) + 1, 0);
			}
		}
		else
		{
			char *err_msg = "Client with such login is already online.";
			send(clConnectInf.socket, err_msg, strlen(err_msg) + 1, 0);
		}
	}
	else
	{
		char *err_msg = "Client with such login is not registered.";
		send(clConnectInf.socket, err_msg, strlen(err_msg) + 1, 0);
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
		//Get src client
		Client* srcClient;
		ClientRegistered(login, srcClient);
		//mark clients status as 'OnCall'
		destClient->SetOnCallWith(srcClient);
		srcClient->SetOnCallWith(destClient);
		printf("'%s' called to '%s'.\n", login, destClientLogin);
		//Get clients addrs
		sockaddr_in destServListAddr = destClient->udp_serv_list_addr;
		//
		int destServListAddrSize = sizeof(destServListAddr);
		char buff[BUFF_LEN];
		char *srcClientLogin = srcClient->Login();
		sendto(udp_sock, &callEventNumber, sizeof(callEventNumber), 0, (sockaddr*)&(destServListAddr), destServListAddrSize);
		Sleep(50);
		sendto(udp_sock, srcClientLogin, strlen(srcClientLogin) + 1, 0, (sockaddr*)&(destServListAddr), destServListAddrSize);
		recvfrom(udp_sock, buff, BUFF_LEN, 0, (sockaddr*)&(destServListAddr), &destServListAddrSize);
		if (strcmp(buff, CALL_ACCEPT_STR) == 0)
		{
			//get from dest client his video list addr
			callEventNumber = 4;
			sendto(udp_sock, (char*)&callEventNumber, sizeof(char), 0, (sockaddr*)&destServListAddr, destServListAddrSize);
			Sleep(50);
			sockaddr_in destVideoListAddr;
			recvfrom(udp_sock, (char*)&destVideoListAddr, sizeof(destVideoListAddr), 0, (sockaddr*)&destServListAddr, &destServListAddrSize);
			destVideoListAddr.sin_addr = destServListAddr.sin_addr;
			//send to src client accept str and dest client addr
			send(clientSock, CALL_ACCEPT_STR, strlen(CALL_ACCEPT_STR) + 1, 0);
			Sleep(50);
			send(clientSock, (char*)&destVideoListAddr, sizeof(destVideoListAddr), 0);
		}
		else
		{
			send(clientSock, CALL_CANCEL_STR, strlen(CALL_CANCEL_STR) + 1, 0);
			srcClient->SetFree();
			destClient->SetFree();
		}
	}
	else
	{
		send(clientSock, CLIENT_ON_CALL_STR, strlen(CLIENT_ON_CALL_STR) + 1, 0);
	}
}


void Server::ClientLeaveChat(SOCKET clientSock)
{
	char clientLogin[BUFF_LEN];
	recv(clientSock, clientLogin, BUFF_LEN, 0);
	ClientSearch onlineClientSearchInf = GetClientByLogin(clientLogin, SEARCH_FROM::ONLINE_CLIENTS);
	Client *leavingClient = onlineClientSearchInf.client;
	if (leavingClient->IsOnCall())
	{
		NotifyClientAboutConnectionEnd(leavingClient->Interlocutor());
		leavingClient->SetFree();
	}
	leavingClient->SetOffline();
	NotifyClientsAboutEvent(1, leavingClient->Login());
	onlineClients.erase(onlineClients.begin() + onlineClientSearchInf.searchedIndex);
	printf("Client '%s' leaved chat.\n", leavingClient->Login());
}


void Server::Disconnect(SOCKET clientSock)
{
	char clientLogin[BUFF_LEN];
	recv(clientSock, clientLogin, BUFF_LEN, 0);
	ClientSearch onlineClientSearchInf = GetClientByLogin(clientLogin, SEARCH_FROM::ONLINE_CLIENTS);
	Client *disonnectingClient = onlineClientSearchInf.client;
	disonnectingClient->SetFree();
	//disonnectingClient->Interlocutor()->SetFree();
	NotifyClientAboutConnectionEnd(disonnectingClient->Interlocutor());
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


void Server::NotifyClientAboutConnectionEnd(Client *client)
{
	char eventNumber = 3;
	sendto(udp_sock, &eventNumber, sizeof(char), 0, (sockaddr*)&(client->udp_serv_list_addr), sizeof(sockaddr_in));
	client->SetFree();
}


bool Server::FreeLogin(char *login)
{
	ClientSearch clSearch = GetClientByLogin(login, SEARCH_FROM::ALL_CLIENTS);
	if (clSearch.searchedIndex != -1)
	{
		return false;
	}
	return true;
}


bool Server::ClientRegistered(char *login, Client* &client)
{
	ClientSearch clSearch = GetClientByLogin(login, SEARCH_FROM::ALL_CLIENTS);
	if (clSearch.searchedIndex != -1)
	{
		client = clSearch.client;
		return true;
	}
	else
	{
		return false;
	}
}


ClientSearch Server::GetClientByLogin(char *login, SEARCH_FROM searchFrom)
{
	ClientSearch clientSearch{ NULL, -1 };
	std::vector<Client*> searchList;
	switch (searchFrom)
	{
	case SEARCH_FROM::ALL_CLIENTS:
		searchList = clients;
		break;
	case SEARCH_FROM::ONLINE_CLIENTS:
		searchList = onlineClients;
		break;
	default:
		break;
	}
	int listSize = searchList.size();
	for (int i = 0; i < listSize; i++)
	{
		if (strcmp(searchList[i]->Login(), login) == 0)
		{
			clientSearch.client = searchList[i];
			clientSearch.searchedIndex = i;
			return clientSearch;
		}
	}
	return clientSearch;
}