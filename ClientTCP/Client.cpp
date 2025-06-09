//GPT_1

//#include "StreamSocket.h"      // Для работы с потоковыми сокетами
//#include "SocketAddress.h"     // Для указания адреса сервера
//#include <iostream>
//#include <string>
//#include <thread>
//
//using namespace Poco::Net;
//using namespace std;
//
//// Функция для приема сообщений от сервера
//void receiveMessages(StreamSocket& socket) 
//{
//    try {
//        char buffer[1024];
//        int bytesReceived;
//        while ((bytesReceived = socket.receiveBytes(buffer, sizeof(buffer))) > 0) {
//            string message(buffer, bytesReceived);
//            cout << "Сообщение от сервера: " << message << endl;
//        }
//    }
//    catch (Poco::Exception& ex) {
//        cerr << "Ошибка при приеме данных: " << ex.displayText() << endl;
//    }
//}
//
//int main() 
//{
//    try 
//    {
//        // Устанавливаем соединение с сервером на localhost:12345
//        SocketAddress serverAddress("127.0.0.1", 12345);
//        StreamSocket socket(serverAddress);
//        cout << "Подключено к серверу." << endl;
//
//        // Запускаем поток для приема сообщений от сервера
//        thread receiver(receiveMessages, ref(socket));
//
//        // Основной поток отправляет сообщения, введенные пользователем
//        string userInput;
//        while (getline(cin, userInput)) 
//        {
//            if (userInput.empty()) continue;
//            socket.sendBytes(userInput.data(), userInput.size());
//        }
//
//        receiver.join(); // Ожидаем завершения потока приема сообщений
//    }
//    catch (Poco::Exception& ex) 
//    {
//        cerr << "Ошибка клиента: " << ex.displayText() << endl;
//    }
//
//    return 0;
//}

//GPT_2

//#include <iostream>
//#include "Poco/Net/StreamSocket.h"
//#include "Poco/Net/SocketAddress.h"
//
//using namespace Poco::Net;
//using namespace std;
//
//int main()
//{
//    try
//    {
//        SocketAddress sa("127.0.0.1", 12345);
//        StreamSocket socket(sa);
//        cout << "Подключено к серверу!" << endl;
//
//        string message;
//        while (true)
//        {
//            cout << "Введите сообщение (или 'exit' для выхода): ";
//            getline(cin, message);
//            if (message == "exit") break;
//            socket.sendBytes(message.data(), message.size());
//
//            char buffer[1024];
//            int n = socket.receiveBytes(buffer, sizeof(buffer));
//            if (n > 0)
//            {
//                string response(buffer, n);
//                cout << "Ответ сервера: " << response << endl;
//            }
//        }
//
//        socket.close();
//    }
//    catch (Exception& e)
//    {
//        cerr << "Ошибка клиента: " << e.displayText() << endl;
//    }
//
//    return 0;
//}

//Google

#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include <iostream>


int main() {
	try {
		Poco::Net::SocketAddress addr("127.0.0.1", 9980);
		Poco::Net::StreamSocket socket;
		socket.connect(addr);


		socket.sendBytes("Hello from client", 17);
		char buffer[256];
		int bytesReceived = socket.receiveBytes(buffer, sizeof(buffer) - 1);
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			std::cout << "Received: " << buffer << std::endl;
		}
		socket.close();
	}
	catch (Poco::Exception& e) {
		std::cerr << "Exception: " << e.displayText() << std::endl;
	}
	return 0;
}