//
// Created by Maria.Filipanova on 5/5/21.
//

#include "Client.h"

Client::Client() {
    mq = mq_open(QUEUE_NAME, O_WRONLY);
    CHECK((mqd_t)-1 != mq);
}

void Client::newSession(std::string sessionId = "") {
    //TODO assert id size
    sendString(NEW_SESSION, sessionId, mq);
}

void Client::attach(std::string sessionId) {
    //TODO assert id size
    sendString(ATTACH, sessionId, mq);
}

void Client::sendSTDIN(const std::string &msg) {
    sendString(STDIN, msg, mq);
}

void Client::detach() {
    constexpr unsigned int size = 1;
    char buffer[size];
    buffer[0] = DETACH;
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Client::kill(std::string sessionId) {
    //TODO assert id size
    sendString(KILL, sessionId, mq);
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

Client::~Client() {
    CHECK((mqd_t)-1 != mq_close(mq));
}
