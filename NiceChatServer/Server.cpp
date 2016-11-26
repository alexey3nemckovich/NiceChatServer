#include "stdafx.h"
#include "Server.h"


DWORD WINAPI ClientProc(LPVOID client_socket);
void Registrate(SOCKET client);
void Login(SOCKET client);
void GetOtherClientAddr(SOCKET client);
void ClientLeaveChat(SOCKET client);


Server::Server()
{
	Init();
}


void Server::Init()
{
	buff = (char*)malloc(BUFF_LEN);
	WSADATA wsa;
	//Initialise winsock lib
	if (WSAStartup(MAKEWORD(2, 2), (WSADATA*)buff))
	{
		printf("Error WSAStartup %d\nPress 'Enter' to exit.", WSAGetLastError());
		getchar();
		ExitProcess(0);
	}
	//Create a socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Error creating socket %d\nPress 'Enter' to exit.", WSAGetLastError());
		WSACleanup();
		ExitProcess(0);
	}
	sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(SERVER_PORT);
	sock_addr.sin_addr.s_addr = 0;
	if (bind(sock, (sockaddr*)&sock_addr, sizeof(sock_addr)))
	{
		printf("Error bind %d\nPress 'Enter' to exit.", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		ExitProcess(0);
	}
}


Server::~Server()
{
	free(buff);
	closesocket(sock);
	WSACleanup();
}


/*
@Server listening operation
@Is receives data, where first byte from client
@says, what type of operation is requested:
@	0 - registration
@	1 - logination
@	2 - desire to connect to another client
@	3 - desite to leave chat
*/
void Server::Listen()
{
	printf("NiceChat Server\n");
	printf("Waiting for connections...\n");
	listen(sock, 0x100);
	SOCKET client_socket;
	struct sockaddr_in client_addr;
	int client_addr_size = sizeof(client_addr);
	while ((client_socket = accept(sock, (sockaddr *)
		&client_addr, &client_addr_size)))
	{
		DWORD thID;
		CreateThread(NULL, NULL, ClientProc,
			&client_socket, NULL, &thID);
	}
}


DWORD WINAPI ClientProc(LPVOID client_socket)
{
	char operation_num;
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	recv(my_sock, &operation_num, sizeof(char), 0);
	switch (operation_num)
	{
	case 0:
		Registrate(my_sock);
		break;
	case 1:
		Login(my_sock);
		break;
	case 2:
		GetOtherClientAddr(my_sock);
		break;
	case 3:
		ClientLeaveChat(my_sock);
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


void Registrate(SOCKET client)
{
	char name[Server::str_buff_size];
	char last_name[Server::str_buff_size];
	char login[Server::str_buff_size];
	char pass[Server::str_buff_size];
	ZeroMemory(name, Server::str_buff_size);
	ZeroMemory(last_name, Server::str_buff_size);
	ZeroMemory(login, Server::str_buff_size);
	ZeroMemory(pass, Server::str_buff_size);
	recv(client, name, Server::str_buff_size, 0);
	recv(client, last_name, Server::str_buff_size, 0);
	recv(client, login, Server::str_buff_size, 0);
	recv(client, pass, Server::str_buff_size, 0);
	printf("Registrated client:\nName - %s\nLast name - %s\nLogin - %s\nPassword - %s\n", name, last_name, login, pass);
}


void Login(SOCKET client)
{

}


void GetOtherClientAddr(SOCKET client)
{

}


void ClientLeaveChat(SOCKET client)
{

}