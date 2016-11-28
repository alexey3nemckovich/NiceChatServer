#pragma once
#include <WinSock2.h>


class Client
{
private:
	char name[STR_BUFF_SIZE];
	char last_name[STR_BUFF_SIZE];
	char login[STR_BUFF_SIZE];
	char pass[STR_BUFF_SIZE];
	sockaddr_in udp_serv_list_addr;
	sockaddr_in udp_video_list_addr;
	bool online = false;
public:
	Client(
		char* name,
		char* last_name,
		char *login,
		char *pass,
		sockaddr_in udp_serv_list_addr,
		sockaddr_in udp_video_list_addr
	);
	~Client();
	char *const Name()
	{
		return name;
	}
	char *const LastName()
	{
		return last_name;
	}
	char *const Login()
	{
		return login;
	}
	char *const Pass()
	{
		return pass;
	}
	bool IsOnline()
	{
		return online;
	}
};