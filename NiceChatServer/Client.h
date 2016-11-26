#pragma once


class Client
{
private:
	char name[STR_BUFF_SIZE];
	char last_name[STR_BUFF_SIZE];
	char login[STR_BUFF_SIZE];
	char pass[STR_BUFF_SIZE];
public:
	Client(char* name, char* last_name, char *login, char *pass);
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
};