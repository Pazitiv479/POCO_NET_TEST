#include "Server.h"

Server::Server(unsigned short port)
    : _port(port), _running(false), _server(nullptr)
{
    // ������������� ������� ���������� (�������� ��� Windows)
    Poco::Net::initializeNetwork();
}

void Server::start() {
    if (_running) return;

    try {
        // 1. ������ ����� ��� ������������� �����
        Poco::Net::ServerSocket serverSocket(_port);

        // 2. ������ ������� ����������
        ConnectionFactory* factory = new ConnectionFactory();

        // 3. �������������� TCP-������
        _server = new Poco::Net::TCPServer(factory, serverSocket);

        // 4. ��������� ������ (�������� ��������� ����������)
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
        _server->stop(); // ������������� ������
        delete _server;  // ����������� �������
        _server = nullptr;
        _running = false;

        std::cout << "Server stopped" << std::endl;
    }
    catch (...) {
        std::cerr << "Error stopping server" << std::endl;
    }

    // ����������� ����
    Poco::Net::uninitializeNetwork();
}

bool Server::isRunning() const {
    return _running;
}

// ���������� ConnectionHandler
Server::ConnectionHandler::ConnectionHandler(const Poco::Net::StreamSocket& socket, int id)
    : TCPServerConnection(socket), _clientId(id)
{
}

void Server::ConnectionHandler::run() {
    try {
        Poco::Net::StreamSocket& socket = this->socket();

        // ���������� � �������
        std::cout << "Client " << _clientId << " connected from: "
            << socket.peerAddress().toString() << std::endl;

        // ���������� �����������
        std::string welcomeMsg = "Welcome to POCO server! Your ID: " + std::to_string(_clientId);
        socket.sendBytes(welcomeMsg.data(), welcomeMsg.size());

        // ������� ���� ��������� ���������
        char buffer[256];
        while (true) {
            int bytesReceived = socket.receiveBytes(buffer, sizeof(buffer));
            if (bytesReceived <= 0) break; // ������ ����������

            buffer[bytesReceived] = '\0';
            std::cout << "Client " << _clientId << " says: " << buffer << std::endl;

            // ���-�����
            std::string echoMsg = "Echo: " + std::string(buffer);
            socket.sendBytes(echoMsg.data(), echoMsg.size());
        }

        std::cout << "Client " << _clientId << " disconnected" << std::endl;
    }
    catch (const Poco::Exception& e) {
        std::cerr << "Client handler error: " << e.displayText() << std::endl;
    }
}

// ���������� ConnectionFactory
Server::ConnectionFactory::ConnectionFactory() : _nextClientId(1) {}

Poco::Net::TCPServerConnection* Server::ConnectionFactory::createConnection(
    const Poco::Net::StreamSocket& socket)
{
    // ������ ����� ���������� � ���������� ID
    return new ConnectionHandler(socket, _nextClientId++);
}