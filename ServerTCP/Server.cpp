#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Mutex.h>
#include <Poco/ThreadPool.h>
#include <iostream>
#include <map>
#include <memory>

using namespace Poco::Net;
using namespace Poco;
using namespace std;

// Глобальный реестр для хранения активных клиентов (id --> StreamSocket)
static Mutex registryMutex;
static map<int, StreamSocket> clients;
static int clientIDCount = 0;

// Класс для обработки соединения с клиентом
class ClientConnection : public TCPServerConnection 
{
public:
	ClientConnection(const StreamSocket& sock) : TCPServerConnection(sock)
	{
		Poco::ScopedLock<Poco::Mutex> lock(registryMutex);
		//std::_Atomic_lock_guard<Mutex> lock(registryMutex);
		int clientID = clientIDCount++;
		clients[clientID] = sock;

		cout << "[+] Client # " << clientID << " connected: " << sock.peerAddress().toString() << endl;
	}

	void run() override
	{
		StreamSocket sock = socket();

		try
		{
			char buffer[1024];
			int n = sock.receiveBytes(buffer, sizeof(buffer));

			while (n > 0)
			{
				string msg(buffer, n);
				cout << "[#" << clientID << "]: " << msg << endl;

				// Ответ клиенту (echo)
				string reply = "The server responds: " + msg;
				sock.sendBytes(reply.data(), (int)reply.size());

				n = sock.receiveBytes(buffer, sizeof(buffer));
			}
		}
		catch (Exception& ex)
		{
			cerr << "[!] Client error #" << clientID << ": "
				<< ex.displayText() << endl;
		}

		// Удаление клиента из реестра при отключении
		{
			Poco::ScopedLock<Poco::Mutex> lock(registryMutex);
			//std::_Atomic_lock_guard<Mutex> lock(registryMutex);
			clients.erase(clientID);
		}
		cout << "[-] Client #" << clientID << " disconnected" << endl;
	}

private:
	int clientID;
};

int main()
{
	UINT16 port = 12345;
	ServerSocket serverSocket(port);
	TCPServerParams::Ptr params = new TCPServerParams;
	params->setMaxQueued(100);
	params->setMaxThreads(16);

	TCPServer server(new TCPServerConnectionFactoryImpl<ClientConnection>(), 
		serverSocket, 
		params);

	server.start();
	cout << "The server is running on the port: " << port << endl;

	// Команда "exit" остановит сервер
	cout << "Enter \"exit\" to stop the server.\n";
	
	string cmd;
	while (getline(cin, cmd))
	{
		if (cmd == "exit")
		{
			break;
		}

		// Пример отправки сообщения всем подключенным клиентам
		if (cmd.rfind("broadcast ", 0) == 0)
		{
			string msg = cmd.substr(10);
			Poco::ScopedLock<Poco::Mutex> lock(registryMutex);
			//std::lock_guard<Mutex> lock(registryMutex);
			for (auto& kv : clients)
			{
				kv.second.sendBytes(msg.data(), (int)msg.size());
			}

			cout << "Sent to all" << msg << endl;
		}
	}

	server.stop();
	cout << "The server is stopped" << endl;
	return 0;
}

// stackOwerflow

//#include <iostream>
//#include "Poco/Net/TCPServer.h"
//#include "Poco/Net/TCPServerParams.h"
//#include "Poco/Net/TCPServerConnectionFactory.h"
//#include "Poco/Net/TCPServerConnection.h"
//#include "Poco/Net/Socket.h"
//using namespace std;
//
//class newConnection : public Poco::Net::TCPServerConnection 
//{
//public:
//    newConnection(const Poco::Net::StreamSocket& s) :
//        Poco::Net::TCPServerConnection(s) 
//    {}
//
//    void run() 
//    {
//        cout << "New connection from: " << socket().peerAddress().host().toString() << endl << flush;
//        bool isOpen = true;
//        Poco::Timespan timeOut(10, 0);
//        unsigned char incommingBuffer[1000];
//        while (isOpen) 
//        {
//            if (socket().poll(timeOut, Poco::Net::Socket::SELECT_READ) == false) 
//            {
//                cout << "TIMEOUT!" << endl << flush;
//            }
//            else 
//            {
//                cout << "RX EVENT!!! ---> " << flush;
//                int nBytes = -1;
//
//                try 
//                {
//                    nBytes = socket().receiveBytes(incommingBuffer, sizeof(incommingBuffer));
//                }
//                catch (Poco::Exception& exc) 
//                {
//                    //Handle your network errors.
//                    cerr << "Network error: " << exc.displayText() << endl;
//                    isOpen = false;
//                }
//
//
//                if (nBytes == 0) 
//                {
//                    cout << "Client closes connection!" << endl << flush;
//                    isOpen = false;
//                }
//                else 
//                {
//                    cout << "Receiving nBytes: " << nBytes << endl << flush;
//                }
//            }
//        }
//        cout << "Connection finished!" << endl << flush;
//    }
//};
//
//int main() 
//{
//
//    //Create a server socket to listen.
//    Poco::Net::ServerSocket svs(1234);
//
//    //Configure some server params.
//    Poco::Net::TCPServerParams* pParams = new Poco::Net::TCPServerParams();
//    pParams->setMaxThreads(4);
//    pParams->setMaxQueued(4);
//    pParams->setThreadIdleTime(100);
//
//    //Create your server
//    Poco::Net::TCPServer myServer(new Poco::Net::TCPServerConnectionFactoryImpl<newConnection>(), svs, pParams);
//    myServer.start();
//
//    while (1);
//
//    return 0;
//}