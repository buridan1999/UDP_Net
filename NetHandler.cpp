// NetHandler.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "cServer.h"

#include <string>
#include <map>

#define STUN_HOST_PORT ("3478")

int stun_xor_addr(char* StunHostname, short StunPort, short local_port, DWORD timeout, char* return_ip_port, int size_of_port_string)
{
	struct sockaddr_in servaddr;
	struct sockaddr_in localaddr;
	const int MAXLINE = 512;
	unsigned char buf[MAXLINE];
	SOCKET sockfd;
	
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
	if (iResult != NO_ERROR)
	{
		wprintf(L"WSAStartup failed with error %d\n", iResult);
		return 1;
	}


	int dwRetval = getaddrinfo(StunHostname, STUN_HOST_PORT, &hints, &result);
	if (dwRetval != 0)
		printf("getaddrinfo failed with error: %d\n", dwRetval);


	sockaddr_in* temp = (sockaddr_in*)result->ai_addr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = temp->sin_addr.s_addr;
	servaddr.sin_port = htons(StunPort);

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
		const int ATR_END = 0x0001;
		int i = 20;
		while (i < sizeof(buf))
		{
			attr_type = htons(*(short *)(&buf[i]));
			attr_length = htons(*(short *)(&buf[i + 2]));
			if (attr_type == ATR_END)
			{
				// parse : port, IP 
				port = ntohs(*(short *)(&buf[i + 6]));

				sprintf_s(return_ip_port, size_of_port_string, "%d.%d.%d.%d:%d", buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11], port);
				return port;
			}
			i += (4 + attr_length);
		}
	}

	return 0;
}










int main()
{
	char return_ip_port[24];
	char URL[] = "stun.gmx.net";//"stun.sipgate.net"; 
	


	auto server = new cServer(27015);
	cAddress adress;
	memset(&adress, NULL, sizeof(cAddress));

	if (int error = server->launch_stuff(5000))
	{
		std::cout << "\n Server starting failed! Error code: " << error << std::endl;
	}
	if (int error = server->init(adress))
	{
		std::cout << "\n Server starting failed! Error code: " << error << std::endl;
	}
	std::cout << "\n\n\n\n";

	std::map<cAddress::TYPE, std::string> NatType
	{
		{ cAddress::TYPE::UDP_BLOCKED, "UDP_BLOCKED" },
		{ cAddress::TYPE::SYM_UDP_FIREWALL, "SYM_UDP_FIREWALL" },
		{ cAddress::TYPE::FULL_CONE, "FULL_CONE" },
		{ cAddress::TYPE::SYMMETRIC, "SYMMETRIC" },
		{ cAddress::TYPE::RESTRICTED, "RESTRICTED" },
		{ cAddress::TYPE::PORT_RESTRICTED, "PORT_RESTRICTED" },
		{ cAddress::TYPE::UNKNOWN, "UNKNOWN" }
	};
	
	auto type = NatType.find(adress.type);
	if (type != NatType.end())
	{
		std::cout << "Yout NAT is " << type->second << std::endl;
	}
	else
	{
		std::cout << "Err type" << std::endl;
	}


	

	int pause;
	std::cout << "\nEnd of programm!\n";
	std::cin >> pause;
}
