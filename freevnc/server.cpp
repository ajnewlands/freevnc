#include "freevnc.h"

enum security_types {
	INVALID=0,
	NONE=1,
	VNC=2,
};

typedef struct pixel_fmt {
	char bpp;
	char depth;
	char big_endian;
	char true_color;
	UINT16 red_max;
	UINT16 green_max;
	UINT16 blue_max;
	char red_shift;
	char green_shift;
	char blue_shift;
	char padding[3];
} pixel_fmt;

int server::server_init(const SOCKET& s)
{

	auto fb_width = htons(short(800));
	auto fb_height = htons(short(600));
	send(s, reinterpret_cast<char*>(&fb_width), 2, 0);
	send(s, reinterpret_cast<char*>(&fb_height), 2, 0);


	pixel_fmt fmt;
	fmt.bpp = 32;
	fmt.depth = 24;
	fmt.big_endian = 0;
	fmt.true_color = 0;
	fmt.blue_shift = 0;
	fmt.red_shift = 0;
	fmt.green_shift = 0;
	fmt.red_max = htons(static_cast<short>(pow(2, 8)) - 1);
	fmt.blue_max = htons(static_cast<short>(pow(2, 8)) - 1);
	fmt.green_max = htons(static_cast<short>(pow(2, 8)) - 1);
	send(s, reinterpret_cast<char*>(&fmt), 16, 0);

	uint32_t namelen = htonl(3);
	const char* name = "vnc";
	send(s, reinterpret_cast<char*>(&namelen), 4, 0);
	send(s, name, 3, 0);
	return 0;
}

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
		auto read = recv(s, pszBuffer, strlen(pszVersion) - len, MSG_WAITALL);
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
	auto security_type_vnc = (char)security_types::NONE;
	send(s, &security_type_vnc, 1, 0);

	// Client selects a type and returns it;
	auto security_type_negotiated = (char)0;
	recv(s, &security_type_negotiated, 1, MSG_WAITALL);
	
	if (security_type_vnc == security_type_negotiated)
	{
		auto ok_response = htonl(0);
		send(s, reinterpret_cast<char*>( &ok_response ), 4, 0);
		std::cout << "Negotiated security type " << (int)security_type_negotiated << std::endl;
	}
	else
	{
		// TODO: send a reason string.
		auto nok_response = htonl(1);
		send(s, reinterpret_cast<char*>( &nok_response ), 4, 0);
		throw "Failed to negotiate a valid security type";
	}

	//security_handshake(s, pszBuffer);

	// Client init; 0=exclusive, non-zero = shared
	char exclusive;
	recv(s, &exclusive, 1, MSG_WAITALL);
	if ((int)exclusive == 0)
		std::cout << "Client requests exclusive connection" << std::endl;
	else
		std::cout << "Client requests shared connection" << std::endl;

	server_init(s);

	char c;
	while (recv(s, &c, 1, 0) > 0) // receive a stream of client messages.
		printf("%hhx ", c);

	return 0;
}

void server::security_handshake(const SOCKET& s, char* pszBuffer)
{
	// Security challenge - using a dummy value (all 0) for now.
	auto chal = new char[16];
	memset(chal, 0, 16); // set the challenge to 16 known bytes.
	send(s, chal, 16, 0);


	char* pszCiphertext = new char[16];
	for (auto len = 0; len < 16; )
	{
		auto read = recv(s, pszCiphertext, 16 - len, 0);
		(read == SOCKET_ERROR) ? throw WSAGetLastError() : len += read;
	}
	//for (int i = 0; i < 16; i++)
	//	std::cout << (int)pszCiphertext[i];

	// decrypt the password
	BCRYPT_ALG_HANDLE hDES;
	BCRYPT_KEY_HANDLE hKey;
	// get the windows crypto provider
	if (BCryptOpenAlgorithmProvider(&hDES, BCRYPT_DES_ALGORITHM, NULL, 0) != STATUS_SUCCESS)
		throw "Failed to initialize the DES provider";
	char Cleartext[16];
	char* Password = "hello123";
	char* pszPassword = new char[16];
	std::cout << "Password will be " << pszPassword << std::endl;
	// VNC requires we reverse the bits in each byte of the password
	for (int i = 0; i < 8; i++) {
		pszPassword[i] = (char)(255 - Password[i]);
	}

	if (auto res = BCryptGenerateSymmetricKey(hDES, &hKey, nullptr, 0, (PUCHAR)pszPassword, 8, 0); res != STATUS_SUCCESS)
	{
		std::cout << "Key generation failed, rc was :" << std::hex << res << std::endl;;
	}


	ULONG OutputLength = 0;
	if (auto res = BCryptDecrypt(hKey, (PUCHAR)pszCiphertext, 16, NULL, NULL, 0, (PUCHAR)& Cleartext, 16, &OutputLength, 0); res != STATUS_SUCCESS)
	{
		std::cout << "Decrypt failed " << std::hex << res << std::endl;
	}
	std::cout << OutputLength << std::endl;

	std::cout << "\nChallenge:\n";
	for (int i = 0; i < 16; i++)
		std::cout << std::hex << (int)chal[i] << ",";
	std::cout << "\nCleartext:\n";
	for (int i = 0; i < 16; i++)
		std::cout << std::hex << Cleartext[i] << ",";

	if (memcmp(Cleartext, chal, 16) == 0)
		std::cout << " By some miracle, authentication worked" << std::endl;
	auto c = getchar();

	free(pszBuffer);
}

server::~server()
{
	WSACleanup();
}