//
// Created by Maria.Filipanova on 5/5/21.
//

#include "Server.h"

Server::Server() {
    struct mq_attr attr;

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    internal_mq = mq_open(INTERNAL_QUEUE_NAME.c_str(), O_CREAT | O_RDWR);
    CHECK((mqd_t)-1 != mq);
    CHECK((mqd_t)-1 != internal_mq);
}

void Server::greetClient(unsigned int sessionId) {
    constexpr unsigned int size = 1 + sizeof(unsigned int);
    char buffer[size];
    buffer[0] = SERVER_HELLO;
    memcpy(buffer + 1, &sessionId, sizeof(sessionId));
    CHECK(0 <= mq_send(mq, buffer, size, 0));
}

void Server::sendSTDOUT(const std::string &msg) {
    sendString(STDOUT, msg);
}

void Server::sendSTDERR(const std::string &msg) {
    sendString(STDERR, msg);
}

void Server::listSessions(const std::vector<unsigned int> &ids) {
    unsigned int size = 1 + ids.size() * sizeof (unsigned int);
    char *buffer = new char[size];
    buffer[0] = SESSIONS;
    auto my_buff = buffer + 1;
    for (const auto& id : ids)  {
        memcpy(my_buff, &id, sizeof(id));
        my_buff += sizeof (id);
    }
    CHECK(0 <= mq_send(mq, buffer, size, 0));
    delete[] buffer;
}

void Server::acceptMessage() {
    char buffer[MAX_SIZE + 1];

    while(true) {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        CHECK(bytes_read >= 0);

        buffer[bytes_read] = '\0';
        switch(buffer[0]) {
            case NEW_SESSION:
                //TODO
                break;
            case ATTACH:
                //TODO
                break;
            case STDIN:
                //TODO
                break;
            case DETACH:
                //TODO
                break;
            case KILL:
                //TODO
                break;
            case LIST:
                //TODO
                break;
            case INTERNAL_STDOUT:
                //TODO
                break;
            case INTERNAL_STDERR:
                //TODO
                break;
        }
    }
}

Server::~Server() {
    CHECK((mqd_t)-1 != mq_close(mq));
    CHECK((mqd_t)-1 != mq_close(internal_mq));
}
