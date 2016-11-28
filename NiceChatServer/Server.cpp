#include "stdafx.h"
#include "Server.h"


DWORD WINAPI ClientProc(LPVOID client_socket);


Server::Server()
{
	Init();
}


Server::~Server()
{
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
	sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(Server::PORT);
	sock_addr.sin_addr.s_addr = 0;
	if (bind(tcp_sock, (sockaddr*)&sock_addr, sizeof(sock_addr)))
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
		Server::GetInstance()->GiveOtherClientAddr(my_sock);
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


void Server::Registrate(SOCKET client)
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
	recv(client, name, STR_BUFF_SIZE, 0);
	recv(client, last_name, STR_BUFF_SIZE, 0);
	recv(client, login, STR_BUFF_SIZE, 0);
	recv(client, pass, STR_BUFF_SIZE, 0);
	char buff[Server::BUFF_LEN];
	ZeroMemory(buff, Server::BUFF_LEN);
	//Get client serv list addr
	sockaddr_in udp_client_serv_list_addr;
	recv(client, buff, Server::BUFF_LEN, 0);
	udp_client_serv_list_addr = ((sockaddr_in*)buff)[0];
	//Get client video list addr
	sockaddr_in udp_client_video_list_addr;
	recv(client, buff, Server::BUFF_LEN, 0);
	udp_client_video_list_addr = ((sockaddr_in*)buff)[0];
	if (FreeLogin(login))
	{
		Client client = Client(name, last_name, login, pass, udp_client_serv_list_addr, udp_client_video_list_addr);
		clients.push_back(client);
		onlineClients.push_back(client);
		//sendto(udp_sock, NULL, 0, 0, (sockaddr*)&udp_client_serv_list_addr, sizeof(udp_client_serv_list_addr));
		printf("Registrated new client:\
		\nName - %s\
		\nLast name - %s\
		\nLogin - %s\
		\nPassword - %s\n",
			name, last_name, login, pass);
		NotifyClientsAboutNewJoin(client);
	}
	else
	{
		char *err_msg = "Login is not free.";
		send(client, err_msg, strlen(err_msg), 0);
	}
}


bool Server::FreeLogin(char *login)
{
	for each (Client client in clients)
	{
		if (strcmp(login, client.Login()) == 0)
		{
			return false;
		}
	}
	return true;
}


void Server::Login(SOCKET client)
{

}


void Server::GiveOtherClientAddr(SOCKET client)
{

}


void Server::ClientLeaveChat(SOCKET client)
{

}


void Server::GiveOnlineClientsList(SOCKET client)
{
	int countOnlineClients = onlineClients.size();
	send(client, (char*)&countOnlineClients, sizeof(countOnlineClients), 0);
	char *clientLogin;
	for (int i = 0; i < countOnlineClients; i++)
	{
		Sleep(5);
		clientLogin = onlineClients[i].Login();
		send(client, clientLogin, strlen(clientLogin), 0);
	}
}


void Server::NotifyClientsAboutNewJoin(Client joinedClient)
{
	char buff[BUFF_LEN];
	int countOnlineClients = onlineClients.size();
	for (int i = 0; i < countOnlineClients; i++)
	{
		if (strcmp(onlineClients[i].Login(), joinedClient.Login()) != 0)
		{
			buff[0] = 2;
			sendto(
				udp_sock,
				buff,
				BUFF_LEN,
				0,
				(sockaddr*)&onlineClients[i].udp_serv_list_addr,
				sizeof(onlineClients[i].udp_serv_list_addr)
			);
		}
	}
}