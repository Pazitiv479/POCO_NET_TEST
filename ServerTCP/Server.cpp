// Server.cpp
#include <iostream>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/TCPServerParams.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <Poco/AtomicCounter.h>
#include <Poco/Mutex.h>
#include <Poco/Format.h>
#include <Poco/Timestamp.h>
#include <memory>
#include <atomic>

using namespace Poco;
using namespace Poco::Net;

// Класс для обработки отдельного TCP-соединения с клиентом
// Наследуется от TCPServerConnection - базового класса POCO для обработки соединений
class EchoConnection : public TCPServerConnection {
public:
    // Конструктор принимает сокет и ID клиента
    EchoConnection(const StreamSocket& socket, int clientId)
        : TCPServerConnection(socket), _clientId(clientId) {
    }

    // Основной метод, который выполняется при запуске соединения в отдельном потоке
    void run() override {
        // Получаем ссылку на сокет для общения с клиентом
        StreamSocket& socket = this->socket();
        try {
            // Логируем подключение клиента с его IP-адресом
            std::cout << format("Client %d connected from %s",
                _clientId,
                socket.peerAddress().toString()) << std::endl;

            // Отправляем приветственное сообщение клиенту
            std::string welcome = format("Welcome! Your client ID is: %d\n", _clientId);
            socket.sendBytes(welcome.data(), welcome.size());

            // Буфер для приема данных от клиента
            char buffer[1024];

            // Основной цикл обработки сообщений от клиента
            while (true) {
                // Читаем данные от клиента (блокирующая операция)
                // receiveBytes вернет количество прочитанных байт или 0/отрицательное значение при отключении
                int n = socket.receiveBytes(buffer, sizeof(buffer));

                // Проверяем, отключился ли клиент
                if (n <= 0) {
                    std::cout << format("Client %d disconnected", _clientId) << std::endl;
                    break; // Выходим из цикла и завершаем обработку
                }

                // Преобразуем полученные данные в строку
                std::string message(buffer, n);

                // Очищаем строку от служебных символов (CR и LF)
                message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
                message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

                // Обрабатываем только непустые сообщения
                if (!message.empty()) {
                    // Логируем полученное сообщение
                    std::cout << format("Message from client %d: %s", _clientId, message) << std::endl;

                    // Формируем и отправляем эхо-ответ клиенту
                    std::string response = "Server received: " + message + "\n";
                    socket.sendBytes(response.data(), response.size());
                }
            }
        }
        // Обработка исключений POCO
        catch (Exception& exc) {
            std::cerr << format("Error with client %d: %s", _clientId, exc.displayText()) << std::endl;
        }
        // Обработка стандартных исключений C++
        catch (std::exception& exc) {
            std::cerr << format("Error with client %d: %s", _clientId, std::string(exc.what())) << std::endl;
        }
        // При выходе из метода сокет автоматически закрывается благодаря RAII
    }

private:
    int _clientId; // Уникальный идентификатор клиента
};

// Фабрика для создания соединений - шаблон "Фабричный метод"
// POCO использует эту фабрику для создания экземпляров EchoConnection
class EchoConnectionFactory : public TCPServerConnectionFactory {
public:
    EchoConnectionFactory() : _clientCounter(0) {}

    // POCO вызывает этот метод при каждом новом подключении
    TCPServerConnection* createConnection(const StreamSocket& socket) override {
        // Атомарно увеличиваем счетчик и создаем новое соединение
        int clientId = ++_clientCounter;
        return new EchoConnection(socket, clientId);
    }

private:
    std::atomic<int> _clientCounter; // Атомарный счетчик для генерации ID клиентов
};

int main() {
    try {
        // ========== НАСТРОЙКА ПАРАМЕТРОВ СЕРВЕРА ==========

        // Создаем параметры сервера для тонкой настройки
        TCPServerParams* params = new TCPServerParams();
        params->setMaxThreads(100);           // Максимальное количество потоков в пуле
        params->setMaxQueued(1000);           // Максимальная очередь ожидающих соединений
        params->setThreadIdleTime(10000);     // Время простоя потока перед уничтожением (мс)

        // ========== СОЗДАНИЕ И ЗАПУСК СЕРВЕРА ==========

        // Создаем endpoint сервера: слушаем на всех интерфейсах (0.0.0.0) порт 12345
        SocketAddress address("0.0.0.0", 12345);

        // ServerSocket - сокет, который принимает входящие соединения
        ServerSocket serverSocket(address);

        // Создаем фабрику соединений
        EchoConnectionFactory factory;

        // Создаем TCP-сервер с указанными параметрами
        TCPServer server(&factory, serverSocket, params);

        // Запускаем сервер - он начинает принимать соединения
        server.start();
        std::cout << "POCO TCP Server started on port 12345" << std::endl;
        std::cout << "Max threads: " << params->getMaxThreads() << std::endl;

        // ========== ДОПОЛНИТЕЛЬНЫЕ ПОТОКИ ==========

        // Создаем и запускаем поток для имитации другой работы сервера
        // Это демонстрирует, что сервер не блокируется на обработке подключений
        Thread workThread;
        workThread.startFunc([]() {
            while (true) {
                std::cout << "Main server work in progress..." << std::endl;
                Thread::sleep(5000); // Пауза 5 секунд
            }
            });

        // ========== ОЖИДАНИЕ КОМАНДЫ ОСТАНОВКИ ==========

        std::cout << "Press Enter to stop server..." << std::endl;
        std::cin.get(); // Блокируем основной поток до нажатия Enter

        // ========== КОРРЕКТНАЯ ОСТАНОВКА СЕРВЕРА ==========

        // Останавливаем сервер - перестаем принимать новые соединения
        server.stop();

        // Дожидаемся завершения рабочего потока
        workThread.join();

        std::cout << "Server stopped" << std::endl;
    }
    // Обработка исключений POCO
    catch (Exception& exc) {
        std::cerr << "Server error: " << exc.displayText() << std::endl;
        return 1;
    }
    // Обработка стандартных исключений C++
    catch (std::exception& exc) {
        std::cerr << "Server error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}