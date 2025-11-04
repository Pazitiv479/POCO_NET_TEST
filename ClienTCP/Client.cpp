// Client.cpp
#include <iostream>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <string>

using namespace Poco;
using namespace Poco::Net;

int main() {
    try {
        // ========== ПОДКЛЮЧЕНИЕ К СЕРВЕРУ ==========

        // Создаем адрес сервера: localhost (127.0.0.1) порт 12345
        SocketAddress address("127.0.0.1", 12345);

        // Создаем сокет для общения с сервером
        StreamSocket socket;

        std::cout << "Connecting to server..." << std::endl;

        // Устанавливаем соединение с сервером (блокирующая операция)
        socket.connect(address);
        std::cout << "Connected to server!" << std::endl;

        // ========== ЧТЕНИЕ ПРИВЕТСТВЕННОГО СООБЩЕНИЯ ==========

        char buffer[1024]; // Буфер для приема данных
        int n = socket.receiveBytes(buffer, sizeof(buffer));
        if (n > 0) {
            std::cout << "Server: " << std::string(buffer, n);
        }

        // ========== ОСНОВНОЙ ЦИКЛ ОБМЕНА СООБЩЕНИЯМИ ==========

        std::string message;
        while (true) {
            // Запрашиваем у пользователя сообщение для отправки
            std::cout << "Enter message (or 'exit' to quit): ";
            std::getline(std::cin, message);

            // Проверяем команду выхода
            if (message == "exit") {
                break;
            }

            // Не отправляем пустые сообщения
            if (message.empty()) {
                std::cout << "Cannot send empty message" << std::endl;
                continue;
            }

            // Отправляем сообщение серверу
            socket.sendBytes(message.data(), message.size());

            // Получаем ответ от сервера (эхо)
            n = socket.receiveBytes(buffer, sizeof(buffer));
            if (n > 0) {
                std::cout << "Server: " << std::string(buffer, n);
            }
            else {
                // Сервер отключился
                std::cout << "Server disconnected" << std::endl;
                break;
            }
        }

        // ========== КОРРЕКТНОЕ ЗАВЕРШЕНИЕ РАБОТЫ ==========

        // Закрываем соединение с сервером
        socket.close();
        std::cout << "Disconnected from server" << std::endl;
    }
    // Обработка исключений POCO
    catch (Exception& exc) {
        std::cerr << "Client error: " << exc.displayText() << std::endl;
        return 1;
    }
    // Обработка стандартных исключений C++
    catch (std::exception& exc) {
        std::cerr << "Client error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}