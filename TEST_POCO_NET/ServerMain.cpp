#include "..\ServerTCP\Server.h"
#include <thread>
#include <iostream>

int main() {
    const unsigned short PORT = 12345;

    // ������ � ��������� ������
    Server server(PORT);
    server.start();

    // ��� ������� �� ���������
    std::cout << "Press Enter to stop server..." << std::endl;
    std::cin.get();

    server.stop();
    return 0;
}