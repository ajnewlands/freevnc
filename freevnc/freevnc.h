#pragma once
#include<condition_variable>
#include<iostream>
#include<atomic>

#include<WinSock2.h>
#include<bcrypt.h>
#include<ntstatus.h>

class __declspec(dllexport) vncscreen
{
public:
	vncscreen(std::condition_variable* has_clients, std::atomic<int>* clients);
	void vncscreen::start();

private:
	std::condition_variable* has_clients;
	std::atomic<int>* clients;
};

class __declspec(dllexport) server {
public:
	server(int port);
private:
	~server();
	int server::handshake(SOCKET);
	void security_handshake(const SOCKET& s, char* pszBuffer);
	int server_init(const SOCKET& s);
	void HandleSetPixelFormat(const SOCKET& s);
	void HandleSetEncodings(const SOCKET& s);
	void HandleFrameBufferUpdateRequest(const SOCKET& s);
	void HandlePointerEvent(const SOCKET& s);
	void HandleKeyEvent(const SOCKET& s);
	void HandleClientCutEvent(const SOCKET& s);
};
