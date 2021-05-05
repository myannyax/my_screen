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

void Server::greetClient(std::string sessionId) {
    //TODO assert id size
    sendString(SERVER_HELLO, sessionId, mq);
}

void Server::sendSTDOUT(const std::string &msg) {
    sendString(STDOUT, msg, mq);
}

void Server::sendSTDERR(const std::string &msg) {
    sendString(STDERR, msg, mq);
}

void Server::listSessions(const std::vector<std::string> &ids) {
    unsigned int size = 1 + ids.size();
    char *buffer = new char[size];
    buffer[0] = SESSIONS;
    auto my_buff = buffer + 1;
    for (const auto& id : ids)  {
        memcpy(my_buff, id.c_str(), id.size());
        my_buff += id.size();
    }
    CHECK(0 <= mq_send(mq, buffer, size, 0));
    delete[] buffer;
}

void Server::acceptMessage() {
    char buffer[MAX_SIZE + 1];

    //TODO listen to internal queue

    while(true) {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        CHECK(bytes_read >= 0);

        buffer[bytes_read] = '\0';
        std::string id;
        switch(buffer[0]) {
            case NEW_SESSION:
                //todo
                id = logic.createNewSession(std::string(buffer + 1));
                greetClient(id);
                break;
            case ATTACH:
                logic.attachClientToSession(std::string(buffer + 1));
                greetClient(id);
                for (auto& entry: logic.getSessionBuffer(id)) {
                    if (entry.isErr) {
                        sendSTDERR(entry.data);
                    } else {
                        sendSTDOUT(entry.data);
                    }
                }
                break;
            case STDIN:
                logic.receiveSTDIN(std::string(buffer + 1));
                break;
            case DETACH:
                logic.detachClient();
                break;
            case KILL:
                logic.killSession(std::string(buffer + 1));
                break;
            case LIST:
                listSessions(logic.getSessionIds());
                break;
        }
    }
}

Server::~Server() {
    CHECK((mqd_t)-1 != mq_close(mq));
    CHECK((mqd_t)-1 != mq_close(internal_mq));
}
