// freevnc.cpp : Defines the entry point for the application.
//

#include "freevncsrv.h"

using namespace std;

int main()
{
	std::mutex mut;
	std::condition_variable has_clients;
	std::atomic<int> clients = 0;

	auto screen = new vncscreen(&has_clients, &clients);
	std::thread screen_thread (&vncscreen::start, screen);

	// bind port and wait for connections
	auto srv = new server(5900);

	// simulate clients connecting and disconnecting
	//while (auto c = getchar())
	//{
	//	cout << "Signalling has_clients" << endl;
	//	if (clients == 0)
	//	{
	//		clients++;
	//		has_clients.notify_one();
	//		cout << "set clients to 1" << endl;
	//	}

	//	else
	//		clients--;

	//}

	return 0;
}
