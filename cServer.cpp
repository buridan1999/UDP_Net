#include "pch.h"
#include "cServer.h"




cServer::cServer(short port)
{
	this->local_port = port;
}

int cServer::init(DWORD timeout)
{
	char StunHostname[] = "stun.sipgate.net";
	int StunPort = 3478;
	//int local_port = 27015;


	//struct sockaddr_in servaddr;
	//struct sockaddr_in localaddr;
	const int MAXLINE = 512;
	unsigned char buf[MAXLINE];
	int i;
	//SOCKET sockfd, i;

	short attr_type;
	short attr_length;
	short port;
	short n;

	// server
	memset(&server_adr, 0, sizeof(server_adr));


	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error %d\n", iResult);
		return 1;
	}

	
	char cStunPort[8];
	_itoa_s(StunPort, cStunPort, 10);
	//_itoa(StunPort, cStunPort, 10);
	int dwRetval = getaddrinfo(StunHostname, cStunPort, &hints, &result);
	if (dwRetval != 0) {
		printf("getaddrinfo failed with error: %d\n", dwRetval);
	}

	sockaddr_in* temp = (sockaddr_in*)result->ai_addr; // ->ai_addr;
	server_adr.sin_family = AF_INET;
	server_adr.sin_addr.s_addr = temp->sin_addr.s_addr; // = inet_addr(StunHostname);
	server_adr.sin_port = htons(StunPort);
	//printf("\n addr = %d", servaddr.sin_addr.s_addr);


	socket_id = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof DWORD); //без этого вызова висим вечно
	if (socket_id == INVALID_SOCKET)
	{
		printf("Couldn't socket\n");
		return -2;
	}

	// local
	memset(&local_adr, 0, sizeof(local_adr));
	local_adr.sin_family = AF_INET;
	local_adr.sin_port = htons(local_port);

	n = bind(socket_id, (struct sockaddr *)&local_adr, sizeof(local_adr));

	//	recvfrom(sd, buff, 100, 0, (sockaddr*)&addr, &notUsed);
	if (SOCKET_ERROR == n)
	{
		printf("Couldn't bind socket\n");
		return -3;
	}


	/// Step 1

	//## first bind
	unsigned char bindingReq[64];
	*(short *)(&bindingReq[0]) = htons(0x0001);      // stun_method
	*(short *)(&bindingReq[2]) = htons(0x0000);      // msg_length
	*(int *)(&bindingReq[4]) = htonl(0x2112A442);  // magic cookie
	*(int *)(&bindingReq[8]) = htonl(0x63c7117e);  // transacation ID
	*(int *)(&bindingReq[12]) = htonl(0x0714278f);
	*(int *)(&bindingReq[16]) = htonl(0x5ded3221);

	//printf("%s", servaddr.sin_addr);

	n = sendto(socket_id, (const char*)bindingReq, 20, 0, (struct sockaddr*)&server_adr, sizeof(server_adr));
	if (n == -1)
	{
		printf("Couldn't sendto\n");
		return -4;
	}
	else
	{
		printf("\nWaiting for answear!");
	}

	//recv()
	//n = recvfrom(socket_id, (char*)buf, MAXLINE, 0, NULL, 0);
	n = recv(socket_id, (char*)buf, MAXLINE, 0);
	if (n != -1)
	{
		if (*(short *)(&buf[0]) == htons(0x0101))
		{
			// parse XOR
			i = 20;
			while (i < sizeof(buf))
			{
				attr_type = htons(*(short *)(&buf[i]));
				attr_length = htons(*(short *)(&buf[i + 2]));
				if (attr_type == 0x0001)
				{
					// parse : port, IP 

					global_port = ntohs(*(short *)(&buf[i + 6]));
					//g_port = port;

					printf("%d.%d.%d.%d:%d", buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11], global_port);
					//sprintf_s(return_ip_port, size_of_port_string, "%d.%d.%d.%d:%d", buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11], port);

					//return 0;
					break;
				}
				i += (4 + attr_length);
			}

		}

		
	}
	else
	{
		printf("\n [ STEP 1 ] Couldn't recvfrom! Timeout!");
		//return -5;
	}

	

	/// Step 2

	*(short *)(&bindingReq[0]) = htons(0x0001);      // stun_method
	*(short *)(&bindingReq[2]) = htons(0x0000);      // msg_length
	*(int *)(&bindingReq[4]) = htonl(0x2112A442);  // magic cookie
	*(int *)(&bindingReq[8]) = htonl(0x63c7117e);  // transacation ID
	*(int *)(&bindingReq[12]) = htonl(0x0714278f);
	*(int *)(&bindingReq[16]) = htonl(0x5ded3221);
	*(short *)(&bindingReq[20]) = htonl(0x0003); // CHANGE_REQUEST

	//printf("%s", servaddr.sin_addr);

	n = sendto(socket_id, (const char*)bindingReq, 22, 0, (struct sockaddr*)&server_adr, sizeof(server_adr));
	if (n == -1)
	{
		printf("Couldn't sendto\n");
		return -4;
	}
	else
	{
		printf("\nWaiting for answear!");
	}

	//recv()
	//n = recvfrom(socket_id, (char*)buf, MAXLINE, 0, NULL, 0);
	n = recv(socket_id, (char*)buf, MAXLINE, 0);
	if (n != -1)
	{
		
	}
	else
	{
		printf("\n [ STEP 2 ] Couldn't recvfrom step: CHANGE_REQUEST! Timeout!");
		//return -5;
	}

	/// Step 3

	*(short *)(&bindingReq[0]) = htons(0x0001);      // stun_method
	*(short *)(&bindingReq[2]) = htons(0x0000);      // msg_length
	*(int *)(&bindingReq[4]) = htonl(0x2112A442);  // magic cookie
	*(int *)(&bindingReq[8]) = htonl(0x63c7117e);  // transacation ID
	*(int *)(&bindingReq[12]) = htonl(0x0714278f);
	*(int *)(&bindingReq[16]) = htonl(0x5ded3221);
	*(short *)(&bindingReq[20]) = htonl(0x0005); //: CHANGE_ADDRESS
	//printf("%s", servaddr.sin_addr);

	n = sendto(socket_id, (const char*)bindingReq, 22, 0, (struct sockaddr*)&server_adr, sizeof(server_adr));
	if (n == -1)
	{
		printf("Couldn't sendto\n");
		return -4;
	}
	else
	{
		printf("\nWaiting for answear!");
	}

	//recv()
	//n = recvfrom(socket_id, (char*)buf, MAXLINE, 0, NULL, 0);
	n = recv(socket_id, (char*)buf, MAXLINE, 0);
	if (n != -1)
	{

	}
	else
	{
		printf("\n [ STEP 3 ] Couldn't recvfrom step: CHANGE_ADDRESS! Timeout!");
		//return -5;
	}


	return 0;
}
/*
int cServer::init(DWORD timeout)
{
	int iResult = 0;
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error %d\n", iResult);
		return 1;
	}

	

	// Create a receiver socket to receive datagrams
	socket_id = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_id == INVALID_SOCKET) {
		wprintf(L"socket in failed with error %d\n", WSAGetLastError());
		return 1;
	}
	setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof DWORD); //без этого вызова висим вечно

	// Receiver
	

	//-----------------------------------------------
	// Bind the socket to any address and the specified port.
	local_adr.sin_family = AF_INET;
	local_adr.sin_port = htons(this->port);
	local_adr.sin_addr.s_addr = htonl(INADDR_ANY);

	iResult = bind(socket_id, (SOCKADDR *)& local_adr, sizeof(local_adr));
	if (iResult != 0) {
		wprintf(L"bind failed with error %d\n", WSAGetLastError());
		return 1;
	}

	server_adr.sin_family = AF_INET;
	server_adr.sin_port = htons(NULL);
	server_adr.sin_addr.s_addr = NULL;//inet_addr("127.0.0.1");


	
	//-----------------------------------------------
	// Call the recvfrom function to receive datagrams
	// on the bound socket.


	//update();
	//



	return 0;
}
*/

int cServer::update(char* buff, int size)
{
	int iResult = 0;
	//char RecvBuf[1024];
	//int BufLen = 1024;


	
	//iResult = recvfrom(socket_id,
	//	buff, size, 0, (SOCKADDR *)& server_adr, NULL);
	//iResult = recvfrom(socket_id,
	//	buff, size, 0, NULL, NULL);
	iResult = recv(socket_id, buff, size, NULL);

//	std::cout << std::endl << "RecvBuf is: " << buff << std::endl << std::endl;

	if (iResult == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == 10060)
		{
			return 0;
		}
		else
		{
			wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
		}
	}
	return iResult;
	//-----------------------------------------------

}

int cServer::send(char* buff, int size)
{
	char qbuff[] = "uiqweyiuqwyeiuqwiueyMESSAGE!!!";
	SOCKET_ERROR;
	int iResult = sendto(socket_id, (const char*)qbuff, sizeof(qbuff), 0, (struct sockaddr*)&server_adr, sizeof(server_adr));

	/*
	int iResult = sendto(this->socket_id,
		buff, size, 0, (SOCKADDR *)& server_adr, sizeof(server_adr));
		*/
	return iResult;
}

int cServer::send_ex(const char * adress, int port, char* buff, int size)
{
	server_adr.sin_family = AF_INET;
	server_adr.sin_port = htons(port);
	server_adr.sin_addr.s_addr = inet_addr(adress);

	
	int iResult = sendto(this->socket_id,
		(const char*)buff, sizeof(size), 0, (SOCKADDR *)& server_adr, sizeof(server_adr));
	return iResult;
}

cServer::~cServer()
{
	int iResult;
	iResult = closesocket(socket_id);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"closesocket in failed with error %d\n", WSAGetLastError());
	}
	

	WSACleanup();
}
