// NetHandler.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "cServer.h"

#define STUN_HOST_PORT ("3478")
int stun_xor_addr(char* StunHostname, short StunPort, short local_port, DWORD timeout, char* return_ip_port, int size_of_port_string)
{
	//const DWORD timeout = 1000;//6000;
	struct sockaddr_in servaddr;
	struct sockaddr_in localaddr;
	const int MAXLINE = 512;
	unsigned char buf[MAXLINE];
	SOCKET sockfd, i;
	
	short attr_type;
	short attr_length;
	short port;
	short n;

	// server
	memset(&servaddr, 0, sizeof(servaddr));


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


	int dwRetval = getaddrinfo(StunHostname, STUN_HOST_PORT, &hints, &result);
	if (dwRetval != 0) {
		printf("getaddrinfo failed with error: %d\n", dwRetval);
	}

	sockaddr_in* temp = (sockaddr_in*)result->ai_addr; // ->ai_addr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = temp->sin_addr.s_addr; // = inet_addr(StunHostname);
	servaddr.sin_port = htons(StunPort);
	//printf("\n addr = %d", servaddr.sin_addr.s_addr);


	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof DWORD); //без этого вызова висим вечно
	if (sockfd == INVALID_SOCKET)
	{
		printf("Couldn't socket\n");
		return -2;
	}

	// local
	memset(&localaddr, 0, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(local_port);

	n = bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));

	//	recvfrom(sd, buff, 100, 0, (sockaddr*)&addr, &notUsed);
	if (SOCKET_ERROR == n)
	{
		printf("Couldn't bind socket\n");
		return -3;
	}

	//## first bind
	unsigned char bindingReq[20];
	*(short *)(&bindingReq[0]) = htons(0x0001);      // stun_method
	*(short *)(&bindingReq[2]) = htons(0x0000);      // msg_length
	*(int *)(&bindingReq[4]) = htonl(0x2112A442);  // magic cookie
	*(int *)(&bindingReq[8]) = htonl(0x63c7117e);  // transacation ID
	*(int *)(&bindingReq[12]) = htonl(0x0714278f);
	*(int *)(&bindingReq[16]) = htonl(0x5ded3221);

	//printf("%s", servaddr.sin_addr);

	n = sendto(sockfd, (const char*)bindingReq, sizeof(bindingReq), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if (n == -1)
	{
		printf("Couldn't sendto\n");
		return -4;
	}
	else
	{
		printf("\nWaiting for answear!");
	}

	n = recvfrom(sockfd, (char*)buf, MAXLINE, 0, NULL, 0);
	if (n == -1)
	{
		printf("\nCouldn't recvfrom! Timeout!");
		return -5;
	}

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

				port = ntohs(*(short *)(&buf[i + 6]));
				//g_port = port;

				//sprintf(return_ip_port, "%d.%d.%d.%d:%d", buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11], port);
				sprintf_s(return_ip_port, size_of_port_string, "%d.%d.%d.%d:%d", buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11], port);

				return port;
				break;
			}
			i += (4 + attr_length);
		}

	}

	return 0;
}










int main()
{
	std::cout << "NEW v 1" << std::endl;
	char return_ip_port[24];
	char URL[] = "stun.gmx.net";//"stun.sipgate.net"; 
	//int opened_port = stun_xor_addr(URL, 3478, 27015, 1000, return_ip_port, sizeof(return_ip_port));
	//printf("\n%s \n Port: %d", return_ip_port, opened_port);


	auto server = new cServer(27015);
	if (int error = server->init(10000))
	{
		std::cout << "\n Server starting failed! Error code: " << error << std::endl;
	}

	

	char buff[1024];
	char send_buff[1024] = "MESSAGE!";

	
	
	int send_port;
	char send_ip[32];
	std::cout << "\n Enter ip: ";
	std::cin >> send_ip;
	std::cout << " Enter port: ";
	std::cin >> send_port;
	std::cout << " Enter message: ";
	std::cin >> send_buff;


	unsigned char bindingReq[20];
	*(short *)(&bindingReq[0]) = htons(0x0001);      // stun_method
	*(short *)(&bindingReq[2]) = htons(0x0000);      // msg_length
	*(int *)(&bindingReq[4]) = htonl(0x2112A442);  // magic cookie
	*(int *)(&bindingReq[8]) = htonl(0x63c7117e);  // transacation ID
	*(int *)(&bindingReq[12]) = htonl(0x0714278f);
	*(int *)(&bindingReq[16]) = htonl(0x5ded3221);
	char StunHostname[] = "stun.sipgate.net";
	int StunPort = 3478;

	// Отправка UDP пакеетов
	for (int i = 0; i < 1000; i++)
	{
		int ret_val = server->send_ex(send_ip, send_port, send_buff, strlen(send_buff) + 1);
		std::cout << "\nSend() return " << ret_val << std::endl;
	}
	
	
	// Получение пакетов
	while (true)
	{
		int getted_size = server->update(buff, sizeof(buff));
		if (getted_size)
		{
			buff[1023] = 0;
			std::cout << "\nReveived data" << std::endl;
			std::cout << "Size: " << getted_size << std::endl;
			std::cout << "Data: " << buff << std::endl;
		}
	}

	

	int pause;
	std::cout << "\nEnd of programm!\n";
	std::cin >> pause;
}
