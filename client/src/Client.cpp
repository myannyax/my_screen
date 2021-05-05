//
// Created by Maria.Filipanova on 5/5/21.
//

#include "Client.h"

void Client::newSession() {
    constexpr unsigned int size = 1;
    char buffer[size];
    buffer[0] = NEW_SESSION;
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Client::attach(unsigned int sessionId) {
    constexpr unsigned int size = 1 + sizeof(unsigned int);
    char buffer[size];
    buffer[0] = ATTACH;
    memcpy(buffer + 1, &sessionId, sizeof(sessionId));
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Client::sendSTDIN(const std::string &msg) {
    unsigned int size = 1 + msg.size();
    char *buffer = new char[size];
    buffer[0] = STDIN;
    memcpy(buffer + 1, msg.c_str(), sizeof(msg.size()));
    CHECK(0 <= mq_send(mq, buffer, size, 0));
    delete[] buffer;
}

void Client::detach() {
    constexpr unsigned int size = 1;
    char buffer[size];
    buffer[0] = DETACH;
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Client::kill(unsigned int sessionId) {
    constexpr unsigned int size = 1 + sizeof(unsigned int);
    char buffer[size];
    buffer[0] = KILL;
    memcpy(buffer + 1, &sessionId, sizeof(sessionId));
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Client::list() {
    constexpr unsigned int size = 1;
    char buffer[size];
    buffer[0] = LIST;
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Client::acceptMessage() {
    //TODO
}
