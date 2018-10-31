// NetHandler.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "cServer.h"


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
