#include "Server.h"

Server::Server(unsigned short port)
    : _port(port), _running(false), _server(nullptr)
{
    // Инициализация сетевой подсистемы (критично для Windows)
    Poco::Net::initializeNetwork();
}

void Server::start() {
    if (_running) return;

    try {
        // 1. Создаём сокет для прослушивания порта
        Poco::Net::ServerSocket serverSocket(_port);

        // 2. Создаём фабрику соединений
        ConnectionFactory* factory = new ConnectionFactory();

        // 3. Инициализируем TCP-сервер
        _server = new Poco::Net::TCPServer(factory, serverSocket);

        // 4. Запускаем сервер (начинает принимать соединения)
        _server->start();
        _running = true;

        std::cout << "Server started on port " << _port << std::endl;
    }
    catch (const Poco::Exception& e) {
        std::cerr << "Server start error: " << e.displayText() << std::endl;
        _running = false;
    }
}

void Server::stop() {
    if (!_running || !_server) return;

    try {
        _server->stop(); // Останавливаем сервер
        delete _server;  // Освобождаем ресурсы
        _server = nullptr;
        _running = false;

        std::cout << "Server stopped" << std::endl;
    }
    catch (...) {
        std::cerr << "Error stopping server" << std::endl;
    }

    // Финализация сети
    Poco::Net::uninitializeNetwork();
}

bool Server::isRunning() const {
    return _running;
}

// Реализация ConnectionHandler
Server::ConnectionHandler::ConnectionHandler(const Poco::Net::StreamSocket& socket, int id)
    : TCPServerConnection(socket), _clientId(id)
{
}

void Server::ConnectionHandler::run() {
    try {
        Poco::Net::StreamSocket& socket = this->socket();

        // Информация о клиенте
        std::cout << "Client " << _clientId << " connected from: "
            << socket.peerAddress().toString() << std::endl;

        // Отправляем приветствие
        std::string welcomeMsg = "Welcome to POCO server! Your ID: " + std::to_string(_clientId);
        socket.sendBytes(welcomeMsg.data(), welcomeMsg.size());

        // Простой цикл обработки сообщений
        char buffer[256];
        while (true) {
            int bytesReceived = socket.receiveBytes(buffer, sizeof(buffer));
            if (bytesReceived <= 0) break; // Клиент отключился

            buffer[bytesReceived] = '\0';
            std::cout << "Client " << _clientId << " says: " << buffer << std::endl;

            // Эхо-ответ
            std::string echoMsg = "Echo: " + std::string(buffer);
            socket.sendBytes(echoMsg.data(), echoMsg.size());
        }

        std::cout << "Client " << _clientId << " disconnected" << std::endl;
    }
    catch (const Poco::Exception& e) {
        std::cerr << "Client handler error: " << e.displayText() << std::endl;
    }
}

// Реализация ConnectionFactory
Server::ConnectionFactory::ConnectionFactory() : _nextClientId(1) {}

Poco::Net::TCPServerConnection* Server::ConnectionFactory::createConnection(
    const Poco::Net::StreamSocket& socket)
{
    // Создаём новый обработчик с уникальным ID
    return new ConnectionHandler(socket, _nextClientId++);
}