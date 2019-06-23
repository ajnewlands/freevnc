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
	const char* pszVersion = "RFB 003.008\n";
	char* pszBuffer = reinterpret_cast<char*>(malloc(sizeof(char) * 128));
	if (pszBuffer == NULL)
		throw "malloc() failed to allocated recv buffer in server::handshake()";

	// version handshake
	// state our protocol version
	send(s, pszVersion, strlen(pszVersion), 0);

	// client responds with its protocol version
	for (auto len = 0; len < strlen(pszVersion); )
	{
		auto read = recv(s, pszBuffer, strlen(pszVersion) - len, 0);
		(read == SOCKET_ERROR) ? throw WSAGetLastError() : len += read;
	}
	if (strncmp(pszVersion, pszBuffer, strlen(pszVersion)) == 0)
		std::cout << "Successfully negotiated version RFB Proto v3.8" << std::endl;
	else
		throw "Invalid protocol version requested";
	
	// next, security handshake
	// first we state the number of security types we support and then send each security type.
	auto security_type_c = (char)1;
	send(s, &security_type_c, 1, 0);
	auto security_type_vnc = (char)2;
	send(s, &security_type_vnc, 1, 0);

	// Client selects a type and returns it;
	auto security_type_negotiated = (char)0;
	recv(s, &security_type_negotiated, 1, 0);
	
	if (security_type_vnc == security_type_negotiated)
	{
		auto ok_response = reinterpret_cast<char*>(htonl(0));
		send(s, ok_response, 4, 0);
		std::cout << "Negotiated the VNC security type" << std::endl;
	}
	else
	{
		// TODO: send a reason string.
		auto nok_response = reinterpret_cast<char*>(htonl(1));
		send(s, nok_response, 4, 0);
		throw "Failed to negotiate a valid security type";
	}

	// Security challenge - using a dummy value (all 0) for now.
	auto chal = reinterpret_cast<char*>(calloc(1, 16));
	if (chal != 0)
		send(s, chal, 16, 0);
	else
		throw "calloc() failed to allocate buffer space";

	char* pszCiphertext = reinterpret_cast<char*>( alloca(16) );
	for (auto len = 0; len < 16; )
	{
		auto read = recv(s, pszCiphertext, 16 - len, 0);
		(read == SOCKET_ERROR) ? throw WSAGetLastError() : len += read;
	}
	for (int i = 0; i < 16; i++)
		std::cout << (int)pszCiphertext[i];

	auto c = getchar();

	free(pszBuffer);
	return 0;
}

server::~server()
{
	WSACleanup();
}