#pragma once
#include <winsock2.h>


class Server
{
private:
	void Init();
	void TryRegistrate(char *name, char *last_name, char *login, char *pass);
	bool ClientExists();
	//Fields
	char* buff;
	SOCKET sock;
	const int SERVER_PORT = 666;
	const int BUFF_LEN = 1000000;
public:
	static const int str_buff_size = 50;
	Server();
	~Server();
	void Listen();
};