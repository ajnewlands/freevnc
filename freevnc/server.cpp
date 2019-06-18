#include "freevnc.h"


server::server(int port)
{
	WSADATA wsaData;
	struct sockaddr_in server, client;
	int client_len = sizeof(client);
	
	// initialize winsock version 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	auto sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		throw WSAGetLastError();
	}
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	
	if (bind(sock, reinterpret_cast<sockaddr*>( &server ), sizeof(server)) == SOCKET_ERROR)
	{
		throw WSAGetLastError();
	}
	std::cout << "Socket bound.." << std::endl;

	if (listen(sock, 4) == SOCKET_ERROR)
	{
		throw WSAGetLastError();
	}
	std::cout << "Listening" << std::endl;

	while (auto cl = accept(sock, reinterpret_cast<sockaddr*>(&client), &client_len))
	{
		if (cl == SOCKET_ERROR) throw WSAGetLastError();
		std::cout << "got connection from " << inet_ntoa(client.sin_addr) << std::endl;
		handshake(cl);
	}
}

int server::handshake(SOCKET s)
{
	char c;
	// state our protocol version
	send(s, "RFB 003.008\n", strlen("RFB 003.008\n"), 0);
	// client responds with its protocol version
	while (recv(s, &c, 1, 0) != SOCKET_ERROR)
	{
		std::cout << c;
	}
	return 0;
}

server::~server()
{
	WSACleanup();
}