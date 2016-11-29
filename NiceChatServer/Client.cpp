#include "stdafx.h"
#include "Client.h"
#include <string>


Client::Client(
	char *name,
	char *last_name,
	char *login,
	char *pass,
	sockaddr_in udp_serv_list_addr,
	sockaddr_in udp_video_list_addr
)
{
	strcpy(this->name, name);
	strcpy(this->last_name, last_name);
	strcpy(this->login, login);
	strcpy(this->pass, pass);
	this->udp_serv_list_addr = udp_serv_list_addr;
	this->udp_video_list_addr = udp_video_list_addr;
	online = false;
}


Client::~Client()
{

}