#pragma once


#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class cServer;
struct cAddress;

inline long ip_bytes_to_long(char ip[4])
{
	long ip_l = (ip[3] & 255 << 24) | (ip[2] & 255 << 16) | (ip[1] & 255 << 8) | (ip[0] & 255);
	return ip_l;
}

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
	int launch_stuff(DWORD timeout);
	int init(cAddress& adress);
	int establish_connection(cAddress adress, DWORD timeout);

	int update(char* buff, int size);
	int send(char* buff, int size);
	int send_ex(const char * adress, int port, char* buff, int size);

	~cServer();
};

struct cAddress
{
	char ip[4];
	short port;
	enum TYPE
	{
		UDP_BLOCKED,
		SYM_UDP_FIREWALL,
		FULL_CONE,
		SYMMETRIC,
		RESTRICTED,
		PORT_RESTRICTED,
		UNKNOWN
	} type;
};
