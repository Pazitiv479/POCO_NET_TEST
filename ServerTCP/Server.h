#pragma once
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/StreamSocket.h>
#include <iostream>
#include <atomic>

class Server {
public:
    Server(unsigned short port);
    void start();
    void stop();
    bool isRunning() const;

private:
    // �����-���������� ��� ������� ����������� �����������
    class ConnectionHandler : public Poco::Net::TCPServerConnection {
    public:
        ConnectionHandler(const Poco::Net::StreamSocket& socket, int id);
        void run() override;

    private:
        int _clientId; // ���������� ID �������
    };

    // ������� ��� �������� ������������
    class ConnectionFactory : public Poco::Net::TCPServerConnectionFactory {
    public:
        ConnectionFactory();
        Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) override;

    private:
        std::atomic<int> _nextClientId; // ��������� ������� ID
    };

    Poco::Net::TCPServer* _server; // ��������� ������� POCO
    unsigned short _port;          // ���� �������������
    std::atomic<bool> _running;    // ���� ��������� �������
};
