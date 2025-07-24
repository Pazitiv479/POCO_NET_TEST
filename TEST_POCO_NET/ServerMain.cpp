#include "..\ServerTCP\Server.h"
#include <thread>
#include <iostream>

int main() {
    const unsigned short PORT = 12345;

    // —оздаЄм и запускаем сервер
    Server server(PORT);
    server.start();

    // ∆дЄм команды на остановку
    std::cout << "Press Enter to stop server..." << std::endl;
    std::cin.get();

    server.stop();
    return 0;
}