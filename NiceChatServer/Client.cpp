#include "stdafx.h"
#include "Client.h"
#include <string>


Client::Client(char *name, char *last_name, char *login, char *pass)
{
	strcpy(this->name, name);
	strcpy(this->last_name, last_name);
	strcpy(this->login, login);
	strcpy(this->pass, pass);
}


Client::~Client()
{

}