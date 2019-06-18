#include "freevnc.h"
using namespace std;

vncscreen::vncscreen(std::condition_variable* has_clients, std::atomic<int>* clients)
{
	this->has_clients = has_clients;
	this->clients = clients;
}

void vncscreen::start()
{
	std::mutex mut;
	while (true)
	{

		while (*clients > 0)
		{
			
		}
		
		cout << "Waiting on has clients" << endl;
		this->has_clients->wait(std::unique_lock<std::mutex>(mut));
		cout << "Got client, total clients " << *clients << endl;
	}
}
