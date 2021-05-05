#include "Client.h"
#include <unistd.h>
#include <thread>

void nkek(Client kek) {
    kek.acceptMessages();
}

int main(int argc, char **argv) {
    //TODO
    Client kek;
    kek.newSession("kek");
    kek.acceptMessages();
    //std::thread t1(nkek, kek);
    usleep(10000);
    std::string line;
    std::getline(std::cin, line);
    kek.sendSTDIN(line);
    usleep(10000);
    kek.detach();
    return 0;
}
