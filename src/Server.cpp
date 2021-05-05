//
// Created by Maria.Filipanova on 5/5/21.
//

#include "Server.h"
#include "vector"

void Server::greetClient(unsigned int sessionId) {
    unsigned int size = 1 + sizeof(unsigned int);
    char buffer[size];
    buffer[0] = SERVER_HELLO;
    memcpy(buffer + 1, &sessionId, sizeof(sessionId));
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Server::sendSTDOUT(const std::string &msg) {
    unsigned int size = 1 + msg.size();
    char *buffer = new char[size];
    buffer[0] = STDOUT;
    memcpy(buffer + 1, msg.c_str(), sizeof(msg.size()));
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Server::sendSTDERR(const std::string &msg) {
    unsigned int size = 1 + msg.size();
    char *buffer = new char[size];
    buffer[0] = STDERR;
    memcpy(buffer + 1, msg.c_str(), sizeof(msg.size()));
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Server::listSessions(const std::vector<unsigned int> &ids) {
    unsigned int size = 1 + ids.size() * sizeof (unsigned int);
    char *buffer = new char[size];
    buffer[0] = SESSIONS;
    auto my_buff = buffer + 1;
    for (const auto& id : ids)  {
        memcpy(buffer + 1, &id, sizeof(id));
        my_buff += sizeof (id);
    }
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Server::acceptMessage() {
    //TODO
}
