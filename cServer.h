#pragma once


#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class cServer;


class cServer
{
private:
	WSADATA wData;
	SOCKET socket_id;
	sockaddr_in local_adr;
	sockaddr_in server_adr;

	short local_port;
	short global_port;
public:
	cServer(short port);
	int init(DWORD timeout);

	int update(char* buff, int size);
	int send(char* buff, int size);
	int send_ex(const char * adress, int port, char* buff, int size);

	~cServer();
};