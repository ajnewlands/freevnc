#pragma once
#include<condition_variable>
#include<iostream>
#include<atomic>

#include<WinSock2.h>


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
};
