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

    mq_to = mq_open(QUEUE_NAME_CS, O_CREAT | O_RDONLY, 0644, &attr);
    mq_from = mq_open(QUEUE_NAME_SC, O_CREAT | O_WRONLY, 0644, &attr);
    internal_mq = mq_open(INTERNAL_QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    CHECK((mqd_t) - 1 != mq_from);
    CHECK((mqd_t) - 1 != mq_to);
    CHECK((mqd_t) - 1 != internal_mq);
}

void Server::greetClient(std::string sessionId) {
    //TODO assert id size
    sendString(SERVER_HELLO, sessionId, mq_from);
}

void Server::sendSTDOUT(const std::string &msg) {
    sendString(STDOUT, msg, mq_from);
}

void Server::sendSTDERR(const std::string &msg) {
    sendString(STDERR, msg, mq_from);
}

void Server::listSessions(const std::vector<std::string> &ids) {
    unsigned int size = 1 + ids.size() + 1;
    for (const auto &id : ids) {
        size += (id.size() + 1);
    }
    char *buffer = new char[size];
    buffer[0] = SESSIONS;
    auto my_buff = buffer + 1;
    for (const auto &id : ids) {
        memcpy(my_buff, id.c_str(), id.size());
        my_buff += id.size();
        *my_buff = '\0';
        my_buff++;
    }
    buffer[size] = '\0';
    CHECK(0 <= mq_send(mq_from, buffer, size, 0));
    delete[] buffer;
}

void Server::acceptMessages() {
    char buffer[MAX_SIZE + 1];

    //TODO listen to internal queue

    while (true) {
        ssize_t bytes_read;
        /* receive the message */
        std::cout << "trying to receive" << std::endl;
        bytes_read = mq_receive(mq_to, buffer, MAX_SIZE, NULL);
        CHECK(bytes_read >= 0);

        buffer[bytes_read] = '\0';
        std::string id;
        switch (buffer[0]) {
            case NEW_SESSION:
                std::cout << "newSession received" << std::endl;
                //todo
                id = logic.createNewSession(std::string(buffer + 1));
                logic.attachClientToSession(id);
                greetClient(id);
                break;
            case ATTACH:
                std::cout << "attach received" << std::endl;
                logic.attachClientToSession(std::string(buffer + 1));
                greetClient(id);
                for (auto &entry: logic.getSessionBuffer(id)) {
                    if (entry.isErr) {
                        sendSTDERR(entry.data);
                    } else {
                        sendSTDOUT(entry.data);
                    }
                }
                break;
            case STDIN:
                std::cout << "stdin received" << std::endl;
                logic.receiveSTDIN(std::string(buffer + 1));
                break;
            case DETACH:
                std::cout << "detach received" << std::endl;
                logic.detachClient();
                break;
            case KILL:
                std::cout << "kill received" << std::endl;
                logic.killSession(std::string(buffer + 1));
                break;
            case LIST:
                std::cout << "list received" << std::endl;
                listSessions(logic.getSessionIds());
                break;
            default :
                std::cout << "kek received" << std::endl;
                break;
        }
    }
}

Server::~Server() {
    CHECK((mqd_t) - 1 != mq_close(mq_to));
    CHECK((mqd_t) - 1 != mq_close(mq_from));
    CHECK((mqd_t) - 1 != mq_close(internal_mq));
}
