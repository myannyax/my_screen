#include <unistd.h>

#include "Client.h"
#include "Common.h"

int main(int argc, char **argv) {
    Client client;

    if (argc < 2) {
        std::cout << "Usage:" << std::endl;
        std::cout << "    " << argv[0] << " list" << std::endl;
        std::cout << "    " << argv[0] << " new [sessionId]" << std::endl;
        std::cout << "    " << argv[0] << " attach <sessionId>" << std::endl;
        std::cout << "    " << argv[0] << " kill <sessionId>" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    if (command == "list") {
        client.list();
    } else if (command == "new") {
        std::string id;
        if (argc > 2) {
            id = argv[2];
        }
        std::string sessionId = client.newSession(id);
        if (!sessionId.empty() && client.attach(sessionId)) {
            client.start();
        }
    } else if (command == "attach") {
        std::string sessionId = argv[2];
        if (client.attach(sessionId)) {
            client.start();
        }
    } else if (command == "kill") {
        std::string sessionId = argv[2];
        client.kill(sessionId);
    } else if (command == "shutdown") {
        client.shutdown();
    }
}
