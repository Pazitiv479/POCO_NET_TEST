//GPT_1

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include "Poco/Net/ServerSocket.h"              // Для создания серверного сокета
#include "Poco/Net/StreamSocket.h"              // Для работы с потоковыми сокетами
#include "Poco/Net/TCPServer.h"                 // Для создания многопоточного TCP-сервера
#include "Poco/Net/TCPServerConnection.h"       // Базовый класс для обработки соединений
#include "Poco/Net/TCPServerConnectionFactory.h"// Фабрика для создания соединений
#include "Poco/Net/TCPServerParams.h"           // Параметры сервера

using namespace Poco::Net;
using namespace std;

// Глобальный список активных клиентских сокетов и мьютекс для синхронизации доступа
vector<StreamSocket> clients;
mutex clientsMutex;

// Класс для обработки каждого клиентского соединения
class ClientConnection : public TCPServerConnection {
public:
    ClientConnection(const StreamSocket& s) : TCPServerConnection(s) {}

    void run() override {
        StreamSocket& clientSocket = socket();
        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(clientSocket); // Добавляем нового клиента в список
        }

        try {
            char buffer[1024];
            int bytesReceived;
            while ((bytesReceived = clientSocket.receiveBytes(buffer, sizeof(buffer))) > 0) {
                string message(buffer, bytesReceived);
                cout << "Получено сообщение от клиента: " << message << endl;

                // Отправляем сообщение обратно всем клиентам
                lock_guard<mutex> lock(clientsMutex);
                for (auto& sock : clients) {
                    if (sock != clientSocket) { // Не отправляем обратно отправителю
                        sock.sendBytes(message.data(), message.size());
                    }
                }
            }
        }
        catch (Poco::Exception& ex) {
            cerr << "Ошибка: " << ex.displayText() << endl;
        }

        // Удаляем клиента из списка при отключении
        {
            lock_guard<mutex> lock(clientsMutex);
            clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
        }
    }
};

int main() {
    try {
        Poco::Net::initializeNetwork();
        // Создаем серверный сокет, прослушивающий порт 12345
        ServerSocket serverSocket(12345);
        // Устанавливаем параметры сервера
        auto params = new TCPServerParams();
        params->setMaxThreads(100); // Максимальное количество потоков
        params->setMaxQueued(100);  // Максимальное количество ожидающих соединений

        // Создаем TCP-сервер с использованием фабрики соединений
        TCPServer server(new TCPServerConnectionFactoryImpl<ClientConnection>(), serverSocket, params);
        server.start(); // Запускаем сервер

        cout << "Сервер запущен на порту 12345. Ожидание подключений..." << endl;

        // Основной поток остается активным
        while (true) {
            this_thread::sleep_for(chrono::seconds(1));
        }

        server.stop(); // Останавливаем сервер при завершении (никогда не достигается в этом примере)
    }
    catch (Poco::Exception& ex) {
        cerr << "Ошибка сервера: " << ex.displayText() << endl;
    }

    return 0;
}

//GPT_2

//#include <iostream>
//#include <Poco/Net/TCPServer.h>
//#include <Poco/Net/TCPServerConnection.h>
//#include <Poco/Net/TCPServerConnectionFactory.h>
//#include <Poco/Net/StreamSocket.h>
//#include <Poco/Net/ServerSocket.h>
//#include <Poco/Net/SocketStream.h>
////#include <Poco/StreamCopier.h>
////#include <Poco/ThreadPool.h>
//
//using namespace Poco::Net;
//using namespace Poco;
//using namespace std;
//
//class EchoConnection : public TCPServerConnection
//{
//public:
//    EchoConnection(const StreamSocket& s) : TCPServerConnection(s) {}
//
//    void run() override
//    {
//        try
//        {
//            StreamSocket& ss = socket();
//            char buffer[1024];
//            int n;
//            while ((n = ss.receiveBytes(buffer, sizeof(buffer))) > 0)
//            {
//                ss.sendBytes(buffer, n);
//            }
//        }
//        catch (Exception& e)
//        {
//            cerr << "EchoConnection error: " << e.displayText() << endl;
//        }
//    }
//};
//
//int main()
//{
//    try
//    {
//        ServerSocket svs(12345); // Создание сервера на порту 12345
//        TCPServer server(new TCPServerConnectionFactoryImpl<EchoConnection>(), svs);
//        server.start();
//        cout << "Сервер запущен. Нажмите Enter для завершения." << endl;
//        cin.get();
//        server.stop();
//    }
//    catch (Exception& e)
//    {
//        cerr << "Ошибка сервера: " << e.displayText() << endl;
//    }
//
//    return 0;
//}

//Google

//#include "Poco/Net/TCPServer.h"
//#include "Poco/Net/TCPServerConnection.h"
//#include "Poco/Net/TCPServerConnectionFactory.h"
//#include "Poco/Net/StreamSocket.h"
//#include <iostream>
//
//
//class MyConnection : public Poco::Net::TCPServerConnection {
//public:
//	MyConnection(const Poco::Net::StreamSocket& s) : Poco::Net::TCPServerConnection(s) {}
//
//
//	void run() {
//		Poco::Net::StreamSocket& ss = socket();
//		char buffer[256];
//		int bytesReceived = ss.receiveBytes(buffer, sizeof(buffer) - 1);
//		if (bytesReceived > 0) {
//			buffer[bytesReceived] = '\0';
//			std::cout << "Received: " << buffer << std::endl;
//			ss.sendBytes("Hello from server", 17);
//		}
//	}
//};
//
//
//class MyConnectionFactory : public Poco::Net::TCPServerConnectionFactory {
//public:
//	Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& s) {
//		return new MyConnection(s);
//	}
//};
//
//
//int main() {
//	Poco::Net::ServerSocket serverSocket(9980);
//	MyConnectionFactory factory;
//	Poco::Net::TCPServer server(factory, serverSocket);
//	server.start();
//	std::cout << "Server started on port 9980" << std::endl;
//	getchar();
//	server.stop();
//	return 0;
//}