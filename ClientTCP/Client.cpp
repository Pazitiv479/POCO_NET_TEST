#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <iostream>
#include <string>

using namespace Poco::Net;
using namespace Poco;
using namespace std;

int main()
{
	try
	{
		/*SocketAddress addr("127.0.0.1", 12345);
		StreamSocket socket(addr);
		cout << "Connected to the server: " << addr.toString() << endl;*/

		SocketAddress addr("127.0.0.1", 12345);
		StreamSocket socket;
		socket.connect(addr);
		cout << "Connected to the server: " << addr.toString() << endl;

		Thread receiveThread;
		receiveThread.startFunc([&]
		{
			try
			{
				char buffer[1024];
				while (true)
				{
					int n = socket.receiveBytes(buffer, sizeof(buffer));
					if (n <= 0)
					{
						break;
					}
					string msg(buffer, n);
					cout << "[Server]: " << msg << endl;
				}
			}
			catch (Exception& ex)
			{
				cerr << "[!] Reception error: " << ex.displayText() << endl;
			}
		});

		cout << "Enter the text to send (\"exit\" — exit):\n";
		string line;
		while (std::getline(cin, line))
		{
			if (line == "exit")
			{
				break;
			}
			socket.sendBytes(line.data(), (int)line.size());
		}

		socket.shutdown();
		receiveThread.join();
		cout << "The client is closing." << endl;
	}
	catch (Exception& ex)
	{
		cerr << "[!] Error: " << ex.displayText() << endl;
	}

	return 0;
}